#ifndef __RE_KERNEL_H
#define __RE_KERNEL_H

#include <uapi/linux/android/rekernel.h>

#define NETLINK_REKERNEL_MAX	26
#define NETLINK_REKERNEL_MIN	22
#define USER_PORT				100
#define PACKET_SIZE				128

extern struct net init_net;

#endif /* __RE_KERNEL_H */
