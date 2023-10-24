#ifndef __MONITOR_NETLINK__
#define __MONITOR_NETLINK__

#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/netlink.h>
#include <linux/rwlock.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <net/net_namespace.h>
#include <net/netlink.h>
// Custom driver header imports
#include <drv_types.h>

// Netlink server definitions
#define RTW_DEFAULT_NL_DEV 0
#define RTW_NETLINK_LOG_TAG "RTW Netlink: INFO "
#define RTW_NETLINK_USER_FAMILY 25       // Netlink port NETLINK_USERSOCK (2)
#define RTW_NETLINK_GROUP 1             // Group ID (must be 1 or higher for multicast)
#define RTW_NETLINK_GFP_FLAG GFP_ATOMIC // NETLINK Non block IO flag
#define RTW_MAX_DEVICES 32              // Maximum netlink devices
#define RTW_DIR_TX 0                    // Dir TX Flag
#define RTW_DIR_RX 1                    // Dir RX Flag
#define RTW_INTERCEPTION_TIMEOUT 2000UL // Wait a max of 2ms during TX interception

// Commands received from user space
#define RTW_NETLINK_CMD_SELECT_DEVICE 0
#define RTW_NETLINK_CMD_READ_ADDR 1
#define RTW_NETLINK_CMD_WRITE_ADDR 2
#define RTW_NETLINK_CMD_MULTIWRITE_ADDR 3
#define RTW_NETLINK_CMD_MULTIREAD_ADDR 4
#define RTW_NETLINK_CMD_INTERRUPT_RX_TX_ENABLE 5
#define RTW_NETLINK_CMD_INTERCEPTION_TX_ENABLE 6
#define RTW_NETLINK_CMD_FORCE_FLAGS_ENABLE 7
#define RTW_NETLINK_CMD_FORCE_FLAGS_RETRY 8
#define RTW_NETLINK_CMD_SEND_DATA 9
#define RTW_NETLINK_CMD_INTERCEPT_TX 10

// Events sent to user space
#define RTW_NETLINK_EVT_TX 0
#define RTW_NETLINK_EVT_RX 1
#define RTW_NETLINK_EVT_TX_INJECTED 2
#define RTW_NETLINK_EVT_TIMEOUT 3

// Status to user space
#define RTW_NETLINK_STATUS_OK 0
#define RTW_NETLINK_STATUS_FAIL -1

#define NLOG(...) (printk(KERN_INFO RTW_NETLINK_LOG_TAG "%s\n", __VA_ARGS__))
#define NLOGE(...) (printk(KERN_ERR RTW_NETLINK_LOG_TAG "%s\n", __VA_ARGS__))
#define NLOGX(fmt, ...) (printk(KERN_INFO RTW_NETLINK_LOG_TAG fmt "\n", __VA_ARGS__))
#define NLOGEX(fmt, ...) (printk(KERN_ERR RTW_NETLINK_LOG_TAG fmt "\n", __VA_ARGS__))

// Misc types
typedef struct mutex nl_mutex;
struct nl_timer_list {
    struct timer_list timer;
    void (*function)(void *);
    void *arg;
};
typedef struct nl_timer_list nl_timer;

struct nl_server_dev_struct {
    void *nl_dev;
    const char *nl_dev_name;
    int nl_dev_id;
    int client_pid;
    nl_timer nl_spinlock_timeout;
    spinlock_t nl_spinlock_sync_tx;
    u8 _nl_tx_interception_requested;
    u8 nl_server_interrupt_rx_enable;
    u8 nl_server_interception_tx_enable;
    u8 nl_interception_buffer[2048];
    u16 nl_interception_buffer_len;
};

// Misc auxiliary functions

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static inline void nl_timer_hdl(struct timer_list *in_timer)
#else
static inline void nl_timer_hdl(unsigned long cntx)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    nl_timer *ptimer = from_timer(ptimer, in_timer, timer);
#else
    nl_timer *ptimer = (nl_timer *)cntx;
#endif
    ptimer->function(ptimer->arg);
}

__inline static void nl_init_timer(nl_timer *ptimer, void *pfunc, void *cntx)
{
    ptimer->function = pfunc;
    ptimer->arg = cntx;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    timer_setup(&ptimer->timer, nl_timer_hdl, 0);
#else
    ptimer->timer.function = nl_timer_hdl;
    ptimer->timer.data = (unsigned long)ptimer;
    init_timer(&ptimer->timer);
#endif
}

__inline static void nl_set_timer(nl_timer *ptimer, u32 delay_time)
{
    mod_timer(&ptimer->timer, (jiffies + (delay_time * HZ / 1000)));
}

__inline static u8 nl_cancel_timer(nl_timer *ptimer)
{
    return (del_timer_sync(&ptimer->timer) == 1 ? 1 : 0);
}

// Prototypes
int nl_server_init(void);
int nl_server_deinit(void);
int nl_server_add_device(void *ptr, const char *dev_name);
int nl_server_del_device(void *ptr);
int nl_server_send_data(uint8_t dir, void *msg, u16 msg_size, uint8_t injected_packet);
static inline int nl_server_send_reply(void *msg, u8 msg_size);

#endif