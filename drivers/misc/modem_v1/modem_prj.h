/*
 * Copyright (C) 2010 Samsung Electronics.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MODEM_PRJ_H__
#define __MODEM_PRJ_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/wakelock.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>

#include "modem_debug.h"
#include "modem_v1.h"
#include "modem_pktlog.h"

#include "include/circ_queue.h"
#include "include/sipc5.h"

#undef DEBUG_MODEM_IF
#ifdef DEBUG_MODEM_IF
#if 1
#define DEBUG_MODEM_IF_LINK_TX
#endif
#if 1
#define DEBUG_MODEM_IF_LINK_RX
#endif
#if defined(DEBUG_MODEM_IF_LINK_TX) && defined(DEBUG_MODEM_IF_LINK_RX)
#define DEBUG_MODEM_IF_LINK_HEADER
#endif

#if 0
#define DEBUG_MODEM_IF_IODEV_TX
#endif
#if 0
#define DEBUG_MODEM_IF_IODEV_RX
#endif

#if 0
#define DEBUG_MODEM_IF_FLOW_CTRL
#endif

#if 0
#define DEBUG_MODEM_IF_PS_DATA
#endif
#if 0
#define DEBUG_MODEM_IF_IP_DATA
#endif
#endif

/*
IOCTL commands
*/
#define IOCTL_MODEM_ON			_IO('o', 0x19)
#define IOCTL_MODEM_OFF			_IO('o', 0x20)
#define IOCTL_MODEM_RESET		_IO('o', 0x21)
#define IOCTL_MODEM_BOOT_ON		_IO('o', 0x22)
#define IOCTL_MODEM_BOOT_OFF		_IO('o', 0x23)
#define IOCTL_MODEM_BOOT_DONE		_IO('o', 0x24)

#define IOCTL_MODEM_PROTOCOL_SUSPEND	_IO('o', 0x25)
#define IOCTL_MODEM_PROTOCOL_RESUME	_IO('o', 0x26)

#define IOCTL_MODEM_STATUS		_IO('o', 0x27)

#define IOCTL_MODEM_DL_START		_IO('o', 0x28)
#define IOCTL_MODEM_FW_UPDATE		_IO('o', 0x29)

#define IOCTL_MODEM_NET_SUSPEND		_IO('o', 0x30)
#define IOCTL_MODEM_NET_RESUME		_IO('o', 0x31)

#define IOCTL_MODEM_DUMP_START		_IO('o', 0x32)
#define IOCTL_MODEM_DUMP_UPDATE		_IO('o', 0x33)
#define IOCTL_MODEM_FORCE_CRASH_EXIT	_IO('o', 0x34)
#define IOCTL_MODEM_CP_UPLOAD		_IO('o', 0x35)

#define IOCTL_LINK_CONNECTED		_IO('o', 0x33)
#define IOCTL_MODEM_SET_TX_LINK		_IO('o', 0x37)

#define IOCTL_MODEM_RAMDUMP_START	_IO('o', 0xCE)
#define IOCTL_MODEM_RAMDUMP_STOP	_IO('o', 0xCF)

#define IOCTL_MODEM_XMIT_BOOT		_IO('o', 0x40)
#ifdef CONFIG_LINK_DEVICE_SHMEM
#define IOCTL_MODEM_GET_SHMEM_INFO	_IO('o', 0x41)
#endif
#define IOCTL_DPRAM_INIT_STATUS		_IO('o', 0x43)

#define IOCTL_LINK_DEVICE_RESET		_IO('o', 0x44)

#ifdef CONFIG_LINK_DEVICE_SHMEM
#define IOCTL_MODEM_GET_SHMEM_SRINFO	_IO('o', 0x45)
#define IOCTL_MODEM_SET_SHMEM_SRINFO	_IO('o', 0x46)
#endif
/* ioctl command for IPC Logger */
#define IOCTL_MIF_LOG_DUMP		_IO('o', 0x51)
#define IOCTL_MIF_DPRAM_DUMP		_IO('o', 0x52)

#define IOCTL_SECURITY_REQ		_IO('o', 0x53)	/* Request smc_call */
#define IOCTL_SHMEM_FULL_DUMP		_IO('o', 0x54)	/* For shmem dump */

/*
Definitions for IO devices
*/
#define MAX_IOD_RXQ_LEN		2048
#define MAX_DM_RXQ_LEN		256

#define CP_CRASH_INFO_SIZE	512
#define CP_CRASH_TAG		"CP Crash "

#define IPv6			6
#define SOURCE_MAC_ADDR		{0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}

/* Loopback */
#define CP2AP_LOOPBACK_CHANNEL	30
#define DATA_LOOPBACK_CHANNEL	31

#define DATA_DRAIN_CHANNEL	30	/* Drain channel to drop RX packets */

/* Debugging features */
#define MIF_LOG_DIR		"/sdcard/log"
#define MIF_MAX_PATH_LEN	256

/* Does modem ctl structure will use state ? or status defined below ?*/
enum modem_state {
	STATE_OFFLINE,
	STATE_CRASH_RESET,	/* silent reset */
	STATE_CRASH_EXIT,	/* cp ramdump */
	STATE_BOOTING,
	STATE_ONLINE,
	STATE_NV_REBUILDING,	/* <= rebuilding start */
	STATE_LOADER_DONE,
	STATE_SIM_ATTACH,
	STATE_SIM_DETACH,
	STATE_CRASH_WATCHDOG,	/* cp watchdog crash */
};

enum link_state {
	LINK_STATE_OFFLINE = 0,
	LINK_STATE_IPC,
	LINK_STATE_CP_CRASH
};

struct sim_state {
	bool online;	/* SIM is online? */
	bool changed;	/* online is changed? */
};

struct modem_firmware {
	unsigned long long binary;
	u32 size;
	u32 m_offset;
	u32 b_offset;
	u32 mode;
	u32 len;
} __packed;

struct modem_sec_req {
	u32 mode;
	u32 size_boot;
	u32 size_main;
	u32 dummy;
} __packed;

enum cp_boot_mode {
	CP_BOOT_MODE_NORMAL,
	CP_BOOT_MODE_DUMP,
	CP_BOOT_RE_INIT,
	MAX_CP_BOOT_MODE
};

struct sec_info {
	enum cp_boot_mode mode;
	u32 size;
};

#define SMC_ID	0x82000700

#define SIPC_MULTI_FRAME_MORE_BIT	(0b10000000)	/* 0x80 */
#define SIPC_MULTI_FRAME_ID_MASK	(0b01111111)	/* 0x7F */
#define SIPC_MULTI_FRAME_ID_BITS	7
#define NUM_SIPC_MULTI_FRAME_IDS	(2 ^ SIPC_MULTI_FRAME_ID_BITS)
#define MAX_SIPC_MULTI_FRAME_ID		(NUM_SIPC_MULTI_FRAME_IDS - 1)

struct __packed sipc_fmt_hdr {
	u16 len;
	u8  msg_seq;
	u8  ack_seq;
	u8  main_cmd;
	u8  sub_cmd;
	u8  cmd_type;
};

static inline bool sipc_ps_ch(u8 ch)
{
	return (ch >= SIPC_CH_ID_PDP_0 && ch <= SIPC_CH_ID_PDP_14) ?
		true : false;
}

/* Channel 0, 5, 6, 27, 255 are reserved in SIPC5.
 * see SIPC5 spec: 2.2.2 Channel Identification (Ch ID) Field.
 * They do not need to store in `iodevs_tree_fmt'
 */
#define sipc5_is_not_reserved_channel(ch) \
	((ch) != 0 && (ch) != 5 && (ch) != 6 && (ch) != 27 && (ch) != 255)

struct vnet {
	struct io_device *iod;
	struct link_device *ld;
};

/* for fragmented data from link devices */
struct fragmented_data {
	struct sk_buff *skb_recv;
	struct sipc5_frame_data f_data;
	/* page alloc fail retry*/
	unsigned int realloc_offset;
};
#define fragdata(iod, ld) (&(iod)->fragments[(ld)->link_type])

/** struct skbuff_priv - private data of struct sk_buff
 * this is matched to char cb[48] of struct sk_buff
 */
struct skbuff_private {
	struct io_device *iod;
	struct link_device *ld;

	/* for time-stamping */
	struct timespec ts;

	u32 sipc_ch:8,	/* SIPC Channel Number			*/
	    frm_ctrl:8,	/* Multi-framing control		*/
	    reserved:15,
	    lnk_hdr:1;	/* Existence of a link-layer header	*/
} __packed;

static inline struct skbuff_private *skbpriv(struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(struct skbuff_private) > sizeof(skb->cb));
	return (struct skbuff_private *)&skb->cb;
}

enum iod_rx_state {
	IOD_RX_ON_STANDBY = 0,
	IOD_RX_HEADER,
	IOD_RX_PAYLOAD,
	IOD_RX_PADDING,
	MAX_IOD_RX_STATE
};

static const char const *rx_state_string[] = {
	[IOD_RX_ON_STANDBY]	= "RX_ON_STANDBY",
	[IOD_RX_HEADER]		= "RX_HEADER",
	[IOD_RX_PAYLOAD]	= "RX_PAYLOAD",
	[IOD_RX_PADDING]	= "RX_PADDING",
};

static const inline char *rx_state(enum iod_rx_state state)
{
	if (unlikely(state >= MAX_IOD_RX_STATE))
		return "INVALID_STATE";
	else
		return rx_state_string[state];
}

struct io_device {
	struct list_head list;

	/* rb_tree node for an io device */
	struct rb_node node_chan;
	struct rb_node node_fmt;

	/* Name of the IO device */
	char *name;

	/* Reference count */
	atomic_t opened;

	/* Wait queue for the IO device */
	wait_queue_head_t wq;

	/* Misc and net device structures for the IO device */
	struct miscdevice  miscdev;
	struct net_device *ndev;
	struct list_head node_ndev;
	struct napi_struct napi;

	/* ID and Format for channel on the link */
	unsigned int id;
	enum modem_link link_types;
	enum dev_format format;
	enum modem_io io_typ;
	enum modem_network net_typ;

	/* Attributes of an IO device */
	u32 attrs;

	/* The name of the application that will use this IO device */
	char *app;

	/* The size of maximum Tx packet */
	unsigned int max_tx_size;

	/* Whether or not handover among 2+ link devices */
	bool use_handover;

	/* SIPC version */
	enum sipc_ver ipc_version;

	/* Whether or not IPC is over SBD-based link device */
	bool sbd_ipc;

	/* Whether or not link-layer header is required */
	bool link_header;

	/* Rx queue of sk_buff */
	struct sk_buff_head sk_rx_q;

	/* For keeping multi-frame packets temporarily */
	struct sk_buff_head sk_multi_q[NUM_SIPC_MULTI_FRAME_IDS];

	/* RX state used in RX FSM */
	enum iod_rx_state curr_rx_state;
	enum iod_rx_state next_rx_state;

	/*
	** work for each io device, when delayed work needed
	** use this for private io device rx action
	*/
	struct delayed_work rx_work;

	/* Information ID for supporting 'Multi FMT'
	 * reference SIPC Spec. 2.2.4
	 */
	u8 info_id;
	spinlock_t info_id_lock;

	struct fragmented_data fragments[LINKDEV_MAX];

	int (*recv_skb_single)(struct io_device *iod, struct link_device *ld,
			       struct sk_buff *skb);

	int (*recv_net_skb)(struct io_device *iod, struct link_device *ld,
			    struct sk_buff *skb);

	/* inform the IO device that the modem is now online or offline or
	 * crashing or whatever...
	 */
	void (*modem_state_changed)(struct io_device *iod, enum modem_state);

	/* inform the IO device that the SIM is not inserting or removing */
	void (*sim_state_changed)(struct io_device *iod, bool sim_online);

	struct modem_ctl *mc;
	struct modem_shared *msd;

	struct wake_lock wakelock;
	long waketime;

	/* DO NOT use __current_link directly
	 * you MUST use skbpriv(skb)->ld in mc, link, etc..
	 */
	struct link_device *__current_link;
};
#define to_io_device(misc) container_of(misc, struct io_device, miscdev)

/* get_current_link, set_current_link don't need to use locks.
 * In ARM, set_current_link and get_current_link are compiled to
 * each one instruction (str, ldr) as atomic_set, atomic_read.
 * And, the order of set_current_link and get_current_link is not important.
 */
#define get_current_link(iod) ((iod)->__current_link)
#define set_current_link(iod, ld) ((iod)->__current_link = (ld))

struct link_device {
	struct list_head  list;
	enum modem_link link_type;
	struct modem_ctl *mc;
	struct modem_shared *msd;

	char *name;
	bool sbd_ipc;
	bool aligned;

	/* SIPC version */
	enum sipc_ver ipc_version;

	/* Modem data */
	struct modem_data *mdm_data;

	/* TX queue of socket buffers */
	struct sk_buff_head sk_fmt_tx_q;
	struct sk_buff_head sk_raw_tx_q;
	struct sk_buff_head *skb_txq[MAX_SIPC_DEVICES];

	/* RX queue of socket buffers */
	struct sk_buff_head sk_fmt_rx_q;
	struct sk_buff_head sk_raw_rx_q;
	struct sk_buff_head *skb_rxq[MAX_SIPC_DEVICES];

	/* Stop/resume control for network ifaces */
	spinlock_t netif_lock;

	/* bit mask for stopped channel */
	unsigned long netif_stop_mask;
	unsigned long tx_flowctrl_mask;

	struct completion raw_tx_resumed;

	/* flag of stopped state for all channels */
	atomic_t netif_stopped;

	struct workqueue_struct *rx_wq;
	struct delayed_work rx_delayed_work;

	int (*init_comm)(struct link_device *ld, struct io_device *iod);
	void (*terminate_comm)(struct link_device *ld, struct io_device *iod);

	/* called by an io_device when it has a packet to send over link
	 * - the io device is passed so the link device can look at id and
	 *   format fields to determine how to route/format the packet
	 */
	int (*send)(struct link_device *ld, struct io_device *iod,
		    struct sk_buff *skb);

	/* method for CP booting */
	void (*boot_on)(struct link_device *ld, struct io_device *iod);
	int (*xmit_boot)(struct link_device *ld, struct io_device *iod,
			 unsigned long arg);
	int (*dload_start)(struct link_device *ld, struct io_device *iod);
	int (*firm_update)(struct link_device *ld, struct io_device *iod,
			   unsigned long arg);

	/* methods for CP crash dump */
	int (*shmem_dump)(struct link_device *ld, struct io_device *iod,
			unsigned long arg);
	int (*force_dump)(struct link_device *ld, struct io_device *iod);
	int (*dump_start)(struct link_device *ld, struct io_device *iod);
	int (*dump_update)(struct link_device *ld, struct io_device *iod,
			   unsigned long arg);
	int (*dump_finish)(struct link_device *ld, struct io_device *iod,
			   unsigned long arg);

	/* IOCTL extension */
	int (*ioctl)(struct link_device *ld, struct io_device *iod,
		     unsigned int cmd, unsigned long arg);

	/* Close (stop) TX with physical link (on CP crash, etc.) */
	void (*close_tx)(struct link_device *ld);

	/* Change secure mode, Call SMC API */
	int (*security_req)(struct link_device *ld, struct io_device *iod,
			unsigned long arg);
};

#define pm_to_link_device(pm)	container_of(pm, struct link_device, pm)

static inline struct sk_buff *rx_alloc_skb(unsigned int length,
		struct io_device *iod, struct link_device *ld)
{
	struct sk_buff *skb;

	skb = dev_alloc_skb(length);
	if (likely(skb)) {
		skbpriv(skb)->iod = iod;
		skbpriv(skb)->ld = ld;
	}

	return skb;
}

struct modemctl_ops {
	int (*modem_on)(struct modem_ctl *);
	int (*modem_off)(struct modem_ctl *);
	int (*modem_shutdown)(struct modem_ctl *);
	int (*modem_reset)(struct modem_ctl *);
	int (*modem_boot_on)(struct modem_ctl *);
	int (*modem_boot_off)(struct modem_ctl *);
	int (*modem_boot_done)(struct modem_ctl *);
	int (*modem_force_crash_exit)(struct modem_ctl *);
	int (*modem_dump_start)(struct modem_ctl *);
	void (*modem_boot_confirm)(struct modem_ctl *);
};

/* for IPC Logger */
struct mif_storage {
	char *addr;
	unsigned int cnt;
};

/* modem_shared - shared data for all io/link devices and a modem ctl
 * msd : mc : iod : ld = 1 : 1 : M : N
 */
struct modem_shared {
	/* list of link devices */
	struct list_head link_dev_list;

	/* list of activated ndev */
	struct list_head activated_ndev_list;
	spinlock_t active_list_lock;

	/* Array of pointers to IO devices corresponding to ch[n] */
	struct io_device *ch2iod[256];

	/* Array of active channels */
	u8 ch[256];

	/* The number of active channels in the array @ch[] */
	unsigned int num_channels;

	/* rb_tree root of io devices. */
	struct rb_root iodevs_tree_fmt; /* group by dev_format */

	/* for IPC Logger */
	struct mif_storage storage;
	spinlock_t lock;

	/* CP crash information */
	char cp_crash_info[530];

	/* loopbacked IP address
	 * default is 0.0.0.0 (disabled)
	 * after you setted this, you can use IP packet loopback using this IP.
	 * exam: echo 1.2.3.4 > /sys/devices/virtual/misc/umts_multipdp/loopback
	 */
	__be32 loopback_ipaddr;
};

struct modem_ctl {
	struct device *dev;
	char *name;
	struct modem_data *mdm_data;

	struct modem_shared *msd;

	enum modem_state phone_state;
	struct sim_state sim_state;

	/* spin lock for each modem_ctl instance */
	spinlock_t lock;

	/* list for notify to opened iod when changed modem state */
	struct list_head modem_state_notify_list;

	/* completion for waiting for CP initialization */
	struct completion init_cmpl;

	/* completion for waiting for CP power-off */
	struct completion off_cmpl;

	unsigned int gpio_cp_on;
	unsigned int gpio_cp_off;
	unsigned int gpio_reset_req_n;
	unsigned int gpio_cp_reset;

	/* for broadcasting AP's PM state (active or sleep) */
	unsigned int gpio_pda_active;
	unsigned int int_pda_active;

	/* for checking aliveness of CP */
	unsigned int gpio_phone_active;
	unsigned int irq_phone_active;
	struct modem_irq irq_cp_active;

	/* for AP-CP power management (PM) handshaking */
	unsigned int gpio_ap_wakeup;
	unsigned int irq_ap_wakeup;

	unsigned int gpio_ap_status;
	unsigned int int_ap_status;

	unsigned int gpio_cp_wakeup;
	unsigned int int_cp_wakeup;

	unsigned int gpio_cp_status;
	unsigned int irq_cp_status;

	/* for performance tuning */
	unsigned int gpio_perf_req;
	unsigned int irq_perf_req;

	/* for USB/HSIC PM */
	unsigned int gpio_host_wakeup;
	unsigned int irq_host_wakeup;
	unsigned int gpio_host_active;
	unsigned int gpio_slave_wakeup;

	unsigned int gpio_cp_dump_int;
	unsigned int gpio_ap_dump_int;
	unsigned int gpio_flm_uart_sel;
	unsigned int gpio_cp_warm_reset;

	unsigned int gpio_sim_detect;
	unsigned int irq_sim_detect;

#ifdef CONFIG_LINK_DEVICE_SHMEM
	unsigned int mbx_pda_active;
	unsigned int mbx_phone_active;
	unsigned int mbx_ap_wakeup;
	unsigned int mbx_ap_status;
	unsigned int mbx_cp_wakeup;
	unsigned int mbx_cp_status;
	unsigned int mbx_perf_req;

	/* for checking aliveness of CP */
	struct modem_irq irq_cp_wdt;		/* watchdog timer */
	struct modem_irq irq_cp_fail;
#endif

#ifdef CONFIG_EXYNOS_BUSMONITOR
	struct notifier_block busmon_nfb;
#endif

	struct work_struct pm_qos_work;

	/* Switch with 2 links in a modem */
	unsigned int gpio_link_switch;

	const struct attribute_group *group;

	struct delayed_work dwork;
	struct work_struct work;

	struct modemctl_ops ops;
	struct io_device *iod;
	struct io_device *bootd;

	void (*gpio_revers_bias_clear)(void);
	void (*gpio_revers_bias_restore)(void);
	void (*modem_complete)(struct modem_ctl *mc);
};

static inline bool cp_offline(struct modem_ctl *mc)
{
	if (!mc)
		return true;
	return (mc->phone_state == STATE_OFFLINE);
}

static inline bool cp_online(struct modem_ctl *mc)
{
	if (!mc)
		return false;
	return (mc->phone_state == STATE_ONLINE);
}

static inline bool cp_booting(struct modem_ctl *mc)
{
	if (!mc)
		return false;
	return (mc->phone_state == STATE_BOOTING);
}

static inline bool cp_crashed(struct modem_ctl *mc)
{
	if (!mc)
		return false;
	return (mc->phone_state == STATE_CRASH_EXIT
		|| mc->phone_state == STATE_CRASH_WATCHDOG);
}

static inline bool rx_possible(struct modem_ctl *mc)
{
	if (likely(cp_online(mc)))
		return true;

	if (cp_booting(mc) || cp_crashed(mc))
		return true;

	return false;
}

int sipc5_init_io_device(struct io_device *iod);
void sipc5_deinit_io_device(struct io_device *iod);

#endif
