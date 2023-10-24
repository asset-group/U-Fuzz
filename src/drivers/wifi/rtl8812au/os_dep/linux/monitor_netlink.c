#include <linux/kprobes.h>
#include <linux/monitor_netlink.h>

#define LEN_CMD_TX_RX 1

static u8 nl_server_initialized = 0;
static struct sock *nl_sk = NULL; // Netlink socket
static struct nl_server_dev_struct nl_server_dev[RTW_MAX_DEVICES] = {0};
static struct task_struct *nl_task_thread;
static wait_queue_head_t nl_task_waitqueue;
static int nl_task_waitqueue_req = 0;
static int nl_registered_devs = 0;

// Exported functions
typedef int (*nl_sched_setaffinity_t)(pid_t pid, struct cpumask *mask);
nl_sched_setaffinity_t nl_sched_setaffinity = NULL;
// typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

__inline static void nl_reset_dev_vars(int dev_id)
{
    nl_server_dev[dev_id].nl_server_interrupt_rx_enable = 0;
    nl_server_dev[dev_id].nl_server_interception_tx_enable = 0;
    nl_server_dev[dev_id]._nl_tx_interception_requested = 0;
}

static void nl_spinlock_timeout_hanler(void *p)
{
    struct nl_server_dev_struct *_nl_dev = (struct nl_server_dev_struct *)p;
    nl_reset_dev_vars(_nl_dev->nl_dev_id);
    NLOGE("fallback: nl_spinlock_timeout_handler");
    // spin_unlock_bh(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
}

// Receive command or data from user-space
static void nl_server_handle_rcv(struct sk_buff *skb)
{
    u8 netlink_command;
    u32 *addr_requested;
    u32 *addr_value;
    u8 addr_value_size;
    u32 reg_val;
    int client_pid;
    int nl_dev_id;
    int res;
    struct sk_buff *s;

    if (!nl_server_initialized) {
        NLOG("Request received while server is not ready");
        return;
    }

    netlink_command = skb->data[0];

    switch (netlink_command) {
    case RTW_NETLINK_CMD_SEND_DATA: {
        // Radiotap header is at least 8 bytes
        if (skb->len <= 8)
            return;
        uint16_t pkt_len = skb->len - 1;
        struct sk_buff *pkt_skb = alloc_skb(pkt_len, RTW_NETLINK_GFP_FLAG);
        skb_put(pkt_skb, pkt_len);
        memcpy(pkt_skb->data, skb->data + 1, pkt_len);
        if (nl_server_dev[RTW_DEFAULT_NL_DEV].nl_dev) {
            // Custom driver code
            struct net_device *pnetdevice = (struct net_device *)nl_server_dev[RTW_DEFAULT_NL_DEV].nl_dev;
            rtw_monitor_xmit_entry(pkt_skb, pnetdevice);
        }
        else
            NLOGE("SEND_DATA: No dev found");
    } break;

    case RTW_NETLINK_CMD_SELECT_DEVICE:
        if (skb->len < 8)
            return;
        client_pid = *((int *)&skb->data[1]);
        nl_dev_id = *((int *)&skb->data[5]);
        if (nl_dev_id < sizeof(nl_server_dev)) {
            NLOGX("Client %d selected nl_dev_id=%d", client_pid, nl_dev_id);
            nl_server_dev[nl_dev_id].client_pid = client_pid;
        }

        res = RTW_NETLINK_STATUS_OK;
        nl_server_send_reply(&res, sizeof(res));
        break;

    case RTW_NETLINK_CMD_INTERRUPT_RX_TX_ENABLE:
        if (skb->len < 2)
            return;
        nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interrupt_rx_enable = skb->data[1];
        res = RTW_NETLINK_STATUS_OK;
        nl_server_send_reply(&res, sizeof(res));
        NLOGX("RX interrupt set to %d", nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interrupt_rx_enable);
        break;

    case RTW_NETLINK_CMD_INTERCEPTION_TX_ENABLE:
        if (skb->len < 2)
            return;
        nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interception_tx_enable = skb->data[1];
        res = RTW_NETLINK_STATUS_OK;
        nl_server_send_reply(&res, sizeof(res));
        NLOGX("Intercept TX set to %d", nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interception_tx_enable);
        break;

    case RTW_NETLINK_CMD_INTERCEPT_TX:
        if (likely(nl_server_dev[RTW_DEFAULT_NL_DEV]._nl_tx_interception_requested)) {
            if (likely(skb->len > 1)) {
                memcpy(nl_server_dev[RTW_DEFAULT_NL_DEV].nl_interception_buffer, &skb->data[1], skb->len - 1);
                nl_server_dev[RTW_DEFAULT_NL_DEV].nl_interception_buffer_len = skb->len - 1;
            }
            else
                nl_server_dev[RTW_DEFAULT_NL_DEV].nl_interception_buffer_len = 0;

            nl_server_dev[RTW_DEFAULT_NL_DEV]._nl_tx_interception_requested = 0;
            // NLOGX("2. Got TX intercepted, len=%d", nl_server_dev[RTW_DEFAULT_NL_DEV].nl_interception_buffer_len);
        }
        break;
    }
    // NLOGX("SKB Received, len:%d", skb->len);
}

// Netlink task thread function
static int nl_task_function(void *pv)
{
    int res, i = 0;

    NLOG("---> Thread nl_task_function STARTED");

    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread %d\n", i++);
        wait_event_interruptible(nl_task_waitqueue, nl_task_waitqueue_req || kthread_should_stop());
        if (kthread_should_stop()) {
            NLOG("Thread nl_task_function killed");
            break;
        }
    }
    NLOG("<--- Thread nl_task_function EXITED");
    return 0;
}

// Send data packets to user-space using netlink
int nl_server_send_data(uint8_t dir, void *msg, u16 msg_size, uint8_t injected_packet)
{
    struct sk_buff *skb_out;
    int res, c_cpu;
    u64 c_time, n_time;
    unsigned long irq_flags;

    if (!nl_server_initialized || !nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interrupt_rx_enable)
        return -1;

    if (msg_size > 2048) {
        NLOGE("Message size over 2048 bytes");
        return 0; // prevent packages higher than 2048 bytes
    }

    skb_out = nlmsg_new(msg_size + LEN_CMD_TX_RX, RTW_NETLINK_GFP_FLAG); // Allocate skb with msg_size
    if (!skb_out)
        return 0;

    skb_put(skb_out, NLMSG_ALIGN(msg_size + LEN_CMD_TX_RX)); // Expand skb tail to fit data buffer
    // nlmsg_put(skb_out, RTW_NETLINK_USER_FAMILY, 0, NLMSG_DONE, msg_size + LEN_CMD_TX_RX, 0); // Put netlink protocol data in skb
    skb_out->data[0] = (dir == RTW_DIR_TX ? (injected_packet ? RTW_NETLINK_EVT_TX_INJECTED : RTW_NETLINK_EVT_TX) : RTW_NETLINK_EVT_RX); // Add TX/RX direction command
    memcpy(skb_out->data + LEN_CMD_TX_RX, (u8 *)msg, msg_size);                                                                         // Copy msg to skb (ignoring netlink data offset)
    skb_out->len = msg_size + LEN_CMD_TX_RX;                                                                                            // Set skb length (ignoring netlink size)
                                                                                                                                        //printk("len:%d, data:%02x\n", skb_out->len, ((u8 *)skb_out->data)[0]);

    if (dir == RTW_DIR_TX) {
        // spin_lock_irqsave(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx, irq_flags);
        // spin_lock_irq(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
        if (nl_sched_setaffinity && smp_processor_id() == 2) {
            struct cpumask c_mask;
            cpumask_setall(&c_mask);
            cpumask_clear_cpu(2, &c_mask);
            // cpumask_clear_cpu(3, &c_mask);
            // cpumask_clear(&c_mask);
            // cpumask_set_cpu(6, &c_mask);
            int ret = nl_sched_setaffinity(0, &c_mask);
            NLOGX("Avoiding core contention... -> nl_sched_setaffinity(), ret=%d", ret);
        }
        spin_lock_bh(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
        if ((!injected_packet) && nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interception_tx_enable)
            nl_server_dev[RTW_DEFAULT_NL_DEV]._nl_tx_interception_requested = 1;
    }

    res = nlmsg_multicast(nl_sk, skb_out, RTW_NETLINK_USER_FAMILY, RTW_NETLINK_GROUP, RTW_NETLINK_GFP_FLAG);
    if (unlikely(res < 0)) {
        // Reset nl_device flags if client disconnection is detected
        nl_reset_dev_vars(RTW_DEFAULT_NL_DEV);
        NLOG("Client disconnected");
        // spin_unlock_irqrestore(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx, irq_flags);
        // if (spin_is_locked(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx))
        spin_unlock_bh(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
        // spin_unlock_irq(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
        return res;
    }

    if (dir == RTW_DIR_TX) {
        // Perform TX Interception
        if ((!injected_packet) && nl_server_dev[RTW_DEFAULT_NL_DEV].nl_server_interception_tx_enable) {
            // NLOG("1. Interception Attempt");
            c_cpu = smp_processor_id();
            c_time = ktime_to_us(ktime_get());
            // NLOGX("1. %llu, cpu:%d", c_time, c_cpu);
            // spin_lock_irq
            nl_set_timer(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_timeout, (((RTW_INTERCEPTION_TIMEOUT) + 1000) / 1000));
            while (nl_server_dev[RTW_DEFAULT_NL_DEV]._nl_tx_interception_requested != 0) {
                if (unlikely(((u64)ktime_to_us(ktime_get()) - c_time) >= RTW_INTERCEPTION_TIMEOUT)) {
                    NLOGX("%llu", (u64)ktime_to_us(ktime_get()));
                    NLOG("Timeout, disabling TX interception");
                    nl_reset_dev_vars(RTW_DEFAULT_NL_DEV);
                    nl_cancel_timer(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_timeout);
                    // if (spin_is_locked(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx))
                    spin_unlock_bh(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
                    // spin_unlock_irq(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
                    // spin_unlock_irqrestore(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx, irq_flags);
                    res = RTW_NETLINK_EVT_TIMEOUT;
                    nl_server_send_reply(&res, sizeof(res));
                    return -1;
                }
            }
            nl_cancel_timer(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_timeout);
            n_time = ktime_to_us(ktime_get());
            c_cpu = smp_processor_id();
            // NLOGX("2. %llu, cpu:%d, d=%lld", n_time, c_cpu, (n_time - c_time));
            memcpy((uint8_t *)msg, nl_server_dev[RTW_DEFAULT_NL_DEV].nl_interception_buffer,
                   nl_server_dev[RTW_DEFAULT_NL_DEV].nl_interception_buffer_len);
        }
        // if (spin_is_locked(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx))
        spin_unlock_bh(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
        // spin_unlock_irq(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx);
        // NLOG("3. Spin Unlocked");
        // spin_unlock_irqrestore(&nl_server_dev[RTW_DEFAULT_NL_DEV].nl_spinlock_sync_tx, irq_flags);
    }

    return 0;
}

// Server reply
static inline int nl_server_send_reply(void *msg, u8 msg_size)
{
    struct sk_buff *skb_out;
    int res;

    if (msg_size > 176)                                  // Raw netlink messages don't work for more than 176 bytes
        return -1;                                       // prevent packages higher than 2048 bytes
    skb_out = nlmsg_new(msg_size, RTW_NETLINK_GFP_FLAG); // Allocate skb with msg_size
    if (!skb_out)
        return -1;

    skb_put(skb_out, NLMSG_ALIGN(msg_size));    // Expand skb tail to fit data buffer
    memcpy(skb_out->data, (u8 *)msg, msg_size); // Copy msg to skb data buffer
    skb_out->len = msg_size;                    // Configure skb size

    res = nlmsg_multicast(nl_sk, skb_out, RTW_NETLINK_USER_FAMILY, RTW_NETLINK_GROUP, RTW_NETLINK_GFP_FLAG);

    if (res < 0)
        return -1;

    return msg_size;
}

static inline int nl_server_send_reply_to(void *msg, u8 msg_size)
{
    struct sk_buff *skb_out;
    int res;

    if (msg_size > 176)                                  // Raw netlink messages don't work for more than 176 bytes
        return -1;                                       // prevent packages higher than 2048 bytes
    skb_out = nlmsg_new(msg_size, RTW_NETLINK_GFP_FLAG); // Allocate skb with msg_size
    if (!skb_out)
        return -1;

    skb_put(skb_out, NLMSG_ALIGN(msg_size));    // Expand skb tail to fit data buffer
    memcpy(skb_out->data, (u8 *)msg, msg_size); // Copy msg to skb data buffer
    skb_out->len = msg_size;                    // Configure skb size

    res = nlmsg_multicast(nl_sk, skb_out, RTW_NETLINK_USER_FAMILY, RTW_NETLINK_GROUP, RTW_NETLINK_GFP_FLAG);
    // res = nlmsg_unicast(nl_sk, skb_out, )

        if (res < 0) return -1;

    return msg_size;
}

int nl_server_init(void)
{
    int ret, i;
    struct netlink_kernel_cfg cfg = {
        .input = nl_server_handle_rcv,
    };

    // Get unexported kernel symbols via kprobe and kallsyms_lookup_name
    static struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name"};

    typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
    kallsyms_lookup_name_t kallsyms_lookup_name = NULL;
    register_kprobe(&kp);
    kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
    unregister_kprobe(&kp);

    if (kallsyms_lookup_name != NULL) {
        NLOG("Got kallsyms_lookup_name symbol");
        nl_sched_setaffinity = (nl_sched_setaffinity_t)kallsyms_lookup_name("sched_setaffinity");
        if (nl_sched_setaffinity != NULL)
            NLOG("Got sched_setaffinity symbol");
    }

    // Initialize netlink server
    if (!nl_sk && !nl_server_initialized) {

        NLOG("Netlink Service started");
        nl_sk = netlink_kernel_create(&init_net, RTW_NETLINK_USER_FAMILY, &cfg);

        if (nl_sk == NULL) {
            nl_server_initialized = 0;
            NLOGE("Error creating socket");
            return -1;
        }

        // Initialize server dev structure
        nl_registered_devs = 0;
        for (i = 0; i < RTW_MAX_DEVICES; i++) {
            nl_server_dev[i].client_pid = 0;
            nl_server_dev[i].nl_dev = NULL;
            nl_server_dev[i].nl_dev_id = i;
            spin_lock_init(&nl_server_dev[i].nl_spinlock_sync_tx);
            nl_init_timer(&nl_server_dev[i].nl_spinlock_timeout, nl_spinlock_timeout_hanler, &nl_server_dev[i]);
        }

        // Initialize netlink task thread
        init_waitqueue_head(&nl_task_waitqueue);
        nl_task_thread = kthread_run(nl_task_function, NULL, "nl_task_thread");
        if (!nl_task_thread) {
            NLOGE("Cannot create kthread nl_task_thread");
            return -1;
        }

        nl_server_initialized = 1;
        return 0;
    }
    else {
        NLOGEX("Netlink service already started or port %d busy", RTW_NETLINK_USER_FAMILY);
    }
    return -1;
}

int nl_server_deinit(void)
{
    int i;

    if (nl_sk) {
        nl_server_initialized = 0;
        netlink_kernel_release(nl_sk);
        nl_sk = NULL;

        // Unlock and free mutexes
        for (i = 0; i < RTW_MAX_DEVICES; i++) {
            if (spin_is_locked(&nl_server_dev[i].nl_spinlock_sync_tx))
                spin_unlock(&nl_server_dev[i].nl_spinlock_sync_tx);
        }

        if (nl_task_thread) {
            kthread_stop(nl_task_thread);
        }

        NLOG("Netlink Service stopped");

        return 0;
    }
    else {
        NLOGEX("Netlink service not started on port %d", RTW_NETLINK_USER_FAMILY);
    }

    return -1;
}

int nl_server_add_device(void *ptr, const char *dev_name)
{
    int i;
    for (i = 0; i < RTW_MAX_DEVICES; i++) {
        if (nl_server_dev[i].nl_dev == NULL) {
            nl_server_dev[i].nl_dev = ptr;
            nl_server_dev[i].nl_dev_name = dev_name;
            nl_registered_devs++;
            NLOGX("Added dev_id=%d, dev_name=%s, dev=0x%04X, ", i, dev_name, ptr);
            return 1;
        }
    }

    NLOGE("Maximum number of devices reached");

    return 0;
}

int nl_server_del_device(void *ptr)
{
    int i;
    for (i = 0; i < RTW_MAX_DEVICES; i++) {
        if (nl_server_dev[i].nl_dev == ptr) {
            nl_server_dev[i].nl_dev = NULL;
            nl_server_dev[i].nl_dev_name = NULL;
            if (nl_registered_devs)
                nl_registered_devs--;
            NLOGX("Removed dev_id=%d, dev=0x%04X", i, ptr);
            return 1;
        }
    }

    NLOGEX("Could not find device %llu", ptr);

    return 0;
}