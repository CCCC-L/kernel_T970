#ifndef __UAPI_LINUX_RE_KERNEL_H
#define __UAPI_LINUX_RE_KERNEL_H

#include <linux/types.h>
#include <linux/cgroup.h>
#include <linux/freezer.h>

#define MIN_USERAPP_UID     10000
#define MAX_SYSTEM_UID      2000
#define RESERVE_ORDER       17
#define WARN_AHEAD_SPACE    (1 << RESERVE_ORDER)

enum report_type {
    BINDER,
    SIGNAL,
};
enum binder_type {
    REPLY,
    TRANSACTION,
    OVERFLOW,
};

static inline bool frozen_task_group(struct task_struct* task) {
    return (cgroup_task_frozen(task) || cgroup_freezing(task));
}

extern void rekernel_report(int reporttype, int type, pid_t src_pid, struct task_struct* src, pid_t dst_pid, struct task_struct* dst, bool oneway);

#endif /* __UAPI_LINUX_RE_KERNEL_H */
