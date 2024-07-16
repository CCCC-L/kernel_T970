#include <linux/init.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include "rekernel.h"

static const char *binder_type[] = {
	"reply",
	"transaction",
	"free_buffer_full",
};

static int netlink_unit = NETLINK_REKERNEL_MIN;

static int rekernel_unit_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", netlink_unit);
	return 0;
}

static int rekernel_unit_open(struct inode *inode, struct file *file)
{
	return single_open(file, rekernel_unit_show, NULL);
}

static struct sock* netlink_socket = NULL;
static struct proc_dir_entry* rekernel_dir, * rekernel_unit_entry;
static const struct file_operations rekernel_unit_fops = {
	.open		= rekernel_unit_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.owner		= THIS_MODULE,
};

static int start_rekernel_server(void)
{
	struct netlink_kernel_cfg rekernel_cfg = {};

	if (netlink_socket)
		return 0;

	for (netlink_unit = NETLINK_REKERNEL_MIN; netlink_unit < NETLINK_REKERNEL_MAX; netlink_unit++) {
		netlink_socket = netlink_kernel_create(&init_net, netlink_unit, &rekernel_cfg);
		if (netlink_socket != NULL)
			break;
	}
	if (netlink_socket == NULL) {
		pr_err("Failed to create Re:Kernel server!\n");
		return -1;
	}
	pr_info("Created Re:Kernel server! NETLINK UNIT: %d\n", netlink_unit);
	rekernel_dir = proc_mkdir("rekernel", NULL);
	if (!rekernel_dir)
		pr_err("create /proc/rekernel failed!\n");
	else {
		char buff[32];
		sprintf(buff, "%d", netlink_unit);
		rekernel_unit_entry = proc_create(buff, 0644, rekernel_dir, &rekernel_unit_fops);
		if (!rekernel_unit_entry)
			pr_err("create rekernel unit failed!\n");
	}
	return 0;
}

static int send_netlink_message(char* msg, uint16_t len) {
	struct sk_buff* skbuffer;
	struct nlmsghdr* nlhdr;

	skbuffer = nlmsg_new(len, GFP_ATOMIC);
	if (!skbuffer) {
		pr_err("netlink alloc failure.\n");
		return -1;
	}

	nlhdr = nlmsg_put(skbuffer, 0, 0, netlink_unit, len, 0);
	if (!nlhdr) {
		pr_err("nlmsg_put failaure.\n");
		nlmsg_free(skbuffer);
		return -1;
	}

	memcpy(nlmsg_data(nlhdr), msg, len);
	return netlink_unicast(netlink_socket, skbuffer, USER_PORT, MSG_DONTWAIT);
}

void rekernel_report(int reporttype, int type, pid_t src_pid, struct task_struct* src, pid_t dst_pid, struct task_struct* dst, bool oneway) {
	char binder_kmsg[PACKET_SIZE];

	if (start_rekernel_server() != 0)
		return;

	if (!frozen_task_group(dst))
		return;

	// if (task_uid(src).val == task_uid(dst).val)
	// 	return;

	switch (reporttype) {
	case BINDER:
		snprintf(binder_kmsg, sizeof(binder_kmsg), "type=Binder,bindertype=%s,oneway=%d,from_pid=%d,from=%d,target_pid=%d,target=%d;", binder_type[type], oneway, src_pid, task_uid(src).val, dst_pid, task_uid(dst).val);
		break;
	case SIGNAL:
		snprintf(binder_kmsg, sizeof(binder_kmsg), "type=Signal,signal=%d,killer_pid=%d,killer=%d,dst_pid=%d,dst=%d;", type, src_pid, task_uid(src).val, dst_pid, task_uid(dst).val);
		break;
	default:
		return;
	}
	send_netlink_message(binder_kmsg, strlen(binder_kmsg));
}
