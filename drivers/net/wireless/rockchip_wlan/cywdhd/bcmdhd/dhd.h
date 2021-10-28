/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Header file describing the internal (inter-module) DHD interfaces.
 *
 * Provides type definitions and function prototypes used to link the
 * DHD OS, bus, and protocol modules.
 *
 * Copyright (C) 1999-2018, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd.h 674559 2017-10-27 03:07:31Z $
 */

/****************
 * Common types *
 */

#ifndef _dhd_h_
#define _dhd_h_

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/ethtool.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined (CONFIG_HAS_WAKELOCK) */
#include <dhd_buzzz.h>
/* The kernel threading is sdio-specific */
struct task_struct;
struct sched_param;
int setScheduler(struct task_struct *p, int policy, struct sched_param *param);
int get_scheduler_policy(struct task_struct *p);
#define MAX_EVENT	16

#define ALL_INTERFACES	0xff

#include <wlioctl.h>
#include <wlfc_proto.h>
#include <hnd_pktq.h>

#if defined(BCMWDF)
#include <wdf.h>
#include <WdfMiniport.h>
#endif /* (BCMWDF)  */

#if defined(CUSTOMER_IPCAM)
#define MAX_RESCHED_CNT 600
#endif 

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0) && LINUX_VERSION_CODE < \
	KERNEL_VERSION(3, 18, 0) || defined(CONFIG_BCMDHD_VENDOR_EXT))
#define WL_VENDOR_EXT_SUPPORT
#endif /* 3.14.0 <= LINUX_KERNEL_VERSION < 3.18.0 || CONFIG_BCMDHD_VENDOR_EXT */

#if defined(KEEP_ALIVE)
/* Default KEEP_ALIVE Period is 55 sec to prevent AP from sending Keep Alive probe frame */
#define KEEP_ALIVE_PERIOD 55000
#define NULL_PKT_STR	"null_pkt"
#endif /* KEEP_ALIVE */

/* Forward decls */
struct dhd_bus;
struct dhd_prot;
struct dhd_info;
struct dhd_ioctl;
struct dhd_dbg;

/* The level of bus communication with the dongle */
enum dhd_bus_state {
	DHD_BUS_DOWN,		/* Not ready for frame transfers */
	DHD_BUS_LOAD,		/* Download access only (CPU reset) */
	DHD_BUS_DATA,		/* Ready for frame transfers */
	DHD_BUS_SUSPEND,	/* Bus has been suspended */
	DHD_BUS_DOWN_IN_PROGRESS,	/* Bus going Down */
};

/*
 * Bit fields to Indicate clean up process that wait till they are finished.
 * Future synchronizable processes can add their bit filed below and update
 * their functionalities accordingly
 */
#define DHD_BUS_BUSY_IN_TX                   0x01
#define DHD_BUS_BUSY_IN_SEND_PKT             0x02
#define DHD_BUS_BUSY_IN_DPC                  0x04
#define DHD_BUS_BUSY_IN_IOVAR                0x08
#define DHD_BUS_BUSY_IN_WD                   0x10
#define DHD_BUS_BUSY_IN_SUSPEND              0x20
#define DHD_BUS_BUSY_IN_RESUME               0x40

/* Download Types */
typedef enum download_type {
	FW,
	NVRAM,
	CLM_BLOB
} download_type_t;


/* For supporting multiple interfaces */
#define DHD_MAX_IFS	16
#define DHD_DEL_IF	-0xE
#define DHD_BAD_IF	-0xF

enum dhd_op_flags {
/* Firmware requested operation mode */
	DHD_FLAG_STA_MODE				= (1 << (0)), /* STA only */
	DHD_FLAG_HOSTAP_MODE				= (1 << (1)), /* SOFTAP only */
	DHD_FLAG_P2P_MODE				= (1 << (2)), /* P2P Only */
	/* STA + P2P */
	DHD_FLAG_CONCURR_SINGLE_CHAN_MODE = (DHD_FLAG_STA_MODE | DHD_FLAG_P2P_MODE),
	DHD_FLAG_CONCURR_MULTI_CHAN_MODE		= (1 << (4)), /* STA + P2P */
	/* Current P2P mode for P2P connection */
	DHD_FLAG_P2P_GC_MODE				= (1 << (5)),
	DHD_FLAG_P2P_GO_MODE				= (1 << (6)),
	DHD_FLAG_MBSS_MODE				= (1 << (7)), /* MBSS in future */
	DHD_FLAG_IBSS_MODE				= (1 << (8)),
	DHD_FLAG_MFG_MODE				= (1 << (9)),
	DHD_FLAG_RSDB_MODE				= (1 << (10)),
	DHD_FLAG_MP2P_MODE				= (1 << (11))
};

#define DHD_OPMODE_SUPPORTED(dhd, opmode_flag) \
	(dhd ? ((((dhd_pub_t *)dhd)->op_mode)  &  opmode_flag) : -1)

/* Max sequential TX/RX Control timeouts to set HANG event */
#ifndef MAX_CNTL_TX_TIMEOUT
#define MAX_CNTL_TX_TIMEOUT 2
#endif /* MAX_CNTL_TX_TIMEOUT */
#ifndef MAX_CNTL_RX_TIMEOUT
#define MAX_CNTL_RX_TIMEOUT 1
#endif /* MAX_CNTL_RX_TIMEOUT */

#define DHD_SCAN_ASSOC_ACTIVE_TIME	40 /* ms: Embedded default Active setting from DHD */
#define DHD_SCAN_UNASSOC_ACTIVE_TIME 80 /* ms: Embedded def. Unassoc Active setting from DHD */
#define DHD_SCAN_PASSIVE_TIME		130 /* ms: Embedded default Passive setting from DHD */

#ifndef POWERUP_MAX_RETRY
#define POWERUP_MAX_RETRY	3 /* how many times we retry to power up the chip */
#endif
#ifndef POWERUP_WAIT_MS
#define POWERUP_WAIT_MS		2000 /* ms: time out in waiting wifi to come up */
#endif
#define MAX_NVRAMBUF_SIZE	(16 * 1024) /* max nvram buf size */
#ifdef DHD_DEBUG
#define DHD_JOIN_MAX_TIME_DEFAULT 10000 /* ms: Max time out for joining AP */
#define DHD_SCAN_DEF_TIMEOUT 10000 /* ms: Max time out for scan in progress */
#endif


enum dhd_bus_wake_state {
	WAKE_LOCK_OFF,
	WAKE_LOCK_PRIV,
	WAKE_LOCK_DPC,
	WAKE_LOCK_IOCTL,
	WAKE_LOCK_DOWNLOAD,
	WAKE_LOCK_TMOUT,
	WAKE_LOCK_WATCHDOG,
	WAKE_LOCK_LINK_DOWN_TMOUT,
	WAKE_LOCK_PNO_FIND_TMOUT,
	WAKE_LOCK_SOFTAP_SET,
	WAKE_LOCK_SOFTAP_STOP,
	WAKE_LOCK_SOFTAP_START,
	WAKE_LOCK_SOFTAP_THREAD
};

enum dhd_prealloc_index {
	DHD_PREALLOC_PROT = 0,
	DHD_PREALLOC_RXBUF,
	DHD_PREALLOC_DATABUF,
	DHD_PREALLOC_OSL_BUF,
#if defined(STATIC_WL_PRIV_STRUCT)
	DHD_PREALLOC_WIPHY_ESCAN0 = 5,
#endif /* STATIC_WL_PRIV_STRUCT */
	DHD_PREALLOC_DHD_INFO = 7,
	DHD_PREALLOC_DHD_WLFC_INFO = 8,
	DHD_PREALLOC_IF_FLOW_LKUP = 9,
	/* 10 */
	DHD_PREALLOC_MEMDUMP_RAM = 11,
	DHD_PREALLOC_DHD_WLFC_HANGER = 12,
	DHD_PREALLOC_PKTID_MAP = 13,
	DHD_PREALLOC_PKTID_MAP_IOCTL = 14
};

enum dhd_dongledump_mode {
	DUMP_DISABLED = 0,
	DUMP_MEMONLY,
	DUMP_MEMFILE,
	DUMP_MEMFILE_BUGON,
	DUMP_MEMFILE_MAX
};

enum dhd_dongledump_type {
	DUMP_TYPE_RESUMED_ON_TIMEOUT = 1,
	DUMP_TYPE_D3_ACK_TIMEOUT,
	DUMP_TYPE_DONGLE_TRAP,
	DUMP_TYPE_MEMORY_CORRUPTION,
	DUMP_TYPE_PKTID_AUDIT_FAILURE,
	DUMP_TYPE_SCAN_TIMEOUT
};

/* Packet alignment for most efficient SDIO (can change based on platform) */
#ifndef DHD_SDALIGN
#define DHD_SDALIGN	32
#endif

/*
 * MAX_NVRAMBUF_SIZE determines the size of the Buffer in the DHD that holds
 * the NVRAM data. That is the size of the buffer pointed by bus->vars
 * This also needs to be increased to 16K to support NVRAM size higher than 8K
 */
#define MAX_NVRAMBUF_SIZE	(16 * 1024) /* max nvram buf size */
#define MAX_CLM_BUF_SIZE	(48 * 1024) /* max clm blob size */

#ifndef CONFIG_BCMDHD_CLM_PATH
#define CONFIG_BCMDHD_CLM_PATH "/system/etc/wifi/bcmdhd_clm.blob"
#endif /* CONFIG_BCMDHD_CLM_PATH */
#define WL_CCODE_NULL_COUNTRY  "#n"


/**
 * DMA-able buffer parameters
 * - dmaaddr_t is 32bits on a 32bit host.
 *   dhd_dma_buf::pa may not be used as a sh_addr_t, bcm_addr64_t or uintptr
 * - dhd_dma_buf::_alloced is ONLY for freeing a DMA-able buffer.
 */
typedef struct dhd_dma_buf {
	void      *va;      /* virtual address of buffer */
	uint32    len;      /* user requested buffer length */
	dmaaddr_t pa;       /* physical address of buffer */
	void      *dmah;    /* dma mapper handle */
	void      *secdma;  /* secure dma sec_cma_info handle */
	uint32    _alloced; /* actual size of buffer allocated with align and pad */
} dhd_dma_buf_t;

/* host reordering packts logic */
/* followed the structure to hold the reorder buffers (void **p) */
typedef struct reorder_info {
	void **p;
	uint8 flow_id;
	uint8 cur_idx;
	uint8 exp_idx;
	uint8 max_idx;
	uint8 pend_pkts;
} reorder_info_t;

#ifdef DHDTCPACK_SUPPRESS

enum {
	/* TCPACK suppress off */
	TCPACK_SUP_OFF,
	/* Replace TCPACK in txq when new coming one has higher ACK number. */
	TCPACK_SUP_REPLACE,
	/* TCPACK_SUP_REPLACE + delayed TCPACK TX unless ACK to PSH DATA.
	 * This will give benefits to Half-Duplex bus interface(e.g. SDIO) that
	 * 1. we are able to read TCP DATA packets first from the bus
	 * 2. TCPACKs that don't need to hurry delivered remains longer in TXQ so can be suppressed.
	 */
	TCPACK_SUP_DELAYTX,
	TCPACK_SUP_HOLD,
	TCPACK_SUP_LAST_MODE
};
#endif /* DHDTCPACK_SUPPRESS */

#if defined(TRAFFIC_MGMT_DWM)
#define DHD_DWM_TBL_SIZE           57
/* DSCP WMM AC Mapping macros and structures */
#define DHD_TRF_MGMT_DWM_FILTER_BIT                 0x8
#define DHD_TRF_MGMT_DWM_PRIO_BITS                  0x7
#define DHD_TRF_MGMT_DWM_FAVORED_BIT                0x10
#define DHD_TRF_MGMT_DWM_PRIO(dwm_tbl_entry) ((dwm_tbl_entry) & DHD_TRF_MGMT_DWM_PRIO_BITS)
#define DHD_TRF_MGMT_DWM_IS_FAVORED_SET(dwm_tbl_entry) \
	((dwm_tbl_entry) & DHD_TRF_MGMT_DWM_FAVORED_BIT)
#define DHD_TRF_MGMT_DWM_SET_FAVORED(dwm_tbl_entry) \
	((dwm_tbl_entry) |= DHD_TRF_MGMT_DWM_FAVORED_BIT)
#define DHD_TRF_MGMT_DWM_IS_FILTER_SET(dwm_tbl_entry) \
	((dwm_tbl_entry) & DHD_TRF_MGMT_DWM_FILTER_BIT)
#define DHD_TRF_MGMT_DWM_SET_FILTER(dwm_tbl_entry) \
	((dwm_tbl_entry) |= DHD_TRF_MGMT_DWM_FILTER_BIT)

typedef struct {
	uint8 dhd_dwm_enabled;
	uint8 dhd_dwm_tbl[DHD_DWM_TBL_SIZE];
} dhd_trf_mgmt_dwm_tbl_t;
#endif 

#define DHD_NULL_CHK_AND_RET(cond) \
	if (!cond) { \
		DHD_ERROR(("%s " #cond " is NULL\n", __FUNCTION__)); \
		return; \
	}

#define DHD_NULL_CHK_AND_RET_VAL(cond, value) \
	if (!cond) { \
		DHD_ERROR(("%s " #cond " is NULL\n", __FUNCTION__)); \
		return value; \
	}

#define DHD_NULL_CHK_AND_GOTO(cond, label) \
	if (!cond) { \
		DHD_ERROR(("%s " #cond " is NULL\n", __FUNCTION__)); \
		goto label; \
	}

/*
 * Accumulating the queue lengths of all flowring queues in a parent object,
 * to assert flow control, when the cummulative queue length crosses an upper
 * threshold defined on a parent object. Upper threshold may be maintained
 * at a station level, at an interface level, or at a dhd instance.
 *
 * cumm_ctr_t abstraction:
 * cumm_ctr_t abstraction may be enhanced to use an object with a hysterisis
 * pause on/off threshold callback.
 * All macros use the address of the cummulative length in the parent objects.
 *
 * BCM_GMAC3 builds use a single perimeter lock, as opposed to a per queue lock.
 * Cummulative counters in parent objects may be updated without spinlocks.
 *
 * In non BCM_GMAC3, if a cummulative queue length is desired across all flows
 * belonging to either of (a station, or an interface or a dhd instance), then
 * an atomic operation is required using an atomic_t cummulative counters or
 * using a spinlock. BCM_ROUTER_DHD uses the Linux atomic_t construct.
 */

/* Cummulative length not supported. */
typedef uint32 cumm_ctr_t;
#define DHD_CUMM_CTR_PTR(clen)     ((cumm_ctr_t*)(clen))
#define DHD_CUMM_CTR(clen)         *(DHD_CUMM_CTR_PTR(clen)) /* accessor */
#define DHD_CUMM_CTR_READ(clen)    DHD_CUMM_CTR(clen) /* read access */
#define DHD_CUMM_CTR_INIT(clen)                                                \
	ASSERT(DHD_CUMM_CTR_PTR(clen) != DHD_CUMM_CTR_PTR(NULL));
#define DHD_CUMM_CTR_INCR(clen)                                                \
	ASSERT(DHD_CUMM_CTR_PTR(clen) != DHD_CUMM_CTR_PTR(NULL));
#define DHD_CUMM_CTR_DECR(clen)                                                \
	ASSERT(DHD_CUMM_CTR_PTR(clen) != DHD_CUMM_CTR_PTR(NULL));

/* DMA'ing r/w indices for rings supported */
#ifdef BCM_INDX_TCM /* FW gets r/w indices in TCM */
#define DMA_INDX_ENAB(dma_indxsup)	0
#elif defined BCM_INDX_DMA  /* FW gets r/w indices from Host memory */
#define DMA_INDX_ENAB(dma_indxsup)	1
#else	/* r/w indices in TCM or host memory based on FW/Host agreement */
#define DMA_INDX_ENAB(dma_indxsup)	dma_indxsup
#endif	/* BCM_INDX_TCM */

#if defined(WLTDLS) && defined(PCIE_FULL_DONGLE)
struct tdls_peer_node {
	uint8 addr[ETHER_ADDR_LEN];
	struct tdls_peer_node *next;
};
typedef struct tdls_peer_node tdls_peer_node_t;
typedef struct {
	tdls_peer_node_t *node;
	uint8 tdls_peer_count;
} tdls_peer_tbl_t;
#endif /* defined(WLTDLS) && defined(PCIE_FULL_DONGLE) */

/* Common structure for module and instance linkage */
typedef struct dhd_pub {
	/* Linkage ponters */
	osl_t *osh;		/* OSL handle */
	struct dhd_bus *bus;	/* Bus module handle */
	struct dhd_prot *prot;	/* Protocol module handle */
	struct dhd_info  *info; /* Info module handle */
	struct dhd_dbg *dbg;	/* Debugability module handle */

	/* to NDIS developer, the structure dhd_common is redundant,
	 * please do NOT merge it back from other branches !!!
	 */

#ifdef BCMDBUS
	struct dbus_pub *dbus;
#endif

	/* Internal dhd items */
	bool up;		/* Driver up/down (to OS) */
	bool txoff;		/* Transmit flow-controlled */
	bool dongle_reset;  /* TRUE = DEVRESET put dongle into reset */
	enum dhd_bus_state busstate;
	uint dhd_bus_busy_state;	/* Bus busy state */
	uint hdrlen;		/* Total DHD header length (proto + bus) */
	uint maxctl;		/* Max size rxctl request from proto to bus */
	uint rxsz;		/* Rx buffer size bus module should use */
	uint8 wme_dp;	/* wme discard priority */

	/* Dongle media info */
	bool iswl;		/* Dongle-resident driver is wl */
	ulong drv_version;	/* Version of dongle-resident driver */
	struct ether_addr mac;	/* MAC address obtained from dongle */
	dngl_stats_t dstats;	/* Stats for dongle-based data */

	/* Additional stats for the bus level */
	ulong tx_packets;	/* Data packets sent to dongle */
	ulong tx_dropped;	/* Data packets dropped in dhd */
	ulong tx_multicast;	/* Multicast data packets sent to dongle */
	ulong tx_errors;	/* Errors in sending data to dongle */
	ulong tx_ctlpkts;	/* Control packets sent to dongle */
	ulong tx_ctlerrs;	/* Errors sending control frames to dongle */
	ulong rx_packets;	/* Packets sent up the network interface */
	ulong rx_multicast;	/* Multicast packets sent up the network interface */
	ulong rx_errors;	/* Errors processing rx data packets */
	ulong rx_ctlpkts;	/* Control frames processed from dongle */
	ulong rx_ctlerrs;	/* Errors in processing rx control frames */
	ulong rx_dropped;	/* Packets dropped locally (no memory) */
	ulong rx_flushed;  /* Packets flushed due to unscheduled sendup thread */
	ulong wd_dpc_sched;   /* Number of times dhd dpc scheduled by watchdog timer */
	ulong rx_pktgetfail; /* Number of PKTGET failures in DHD on RX */
	ulong tx_pktgetfail; /* Number of PKTGET failures in DHD on TX */
	ulong rx_readahead_cnt;	/* Number of packets where header read-ahead was used. */
	ulong tx_realloc;	/* Number of tx packets we had to realloc for headroom */
	ulong fc_packets;       /* Number of flow control pkts recvd */

	/* Last error return */
	int bcmerror;
	uint tickcnt;

	/* Last error from dongle */
	int dongle_error;

	uint8 country_code[WLC_CNTRY_BUF_SZ];

	/* Suspend disable flag and "in suspend" flag */
	int suspend_disable_flag; /* "1" to disable all extra powersaving during suspend */
	int in_suspend;			/* flag set to 1 when early suspend called */
#ifdef PNO_SUPPORT
	int pno_enable;			/* pno status : "1" is pno enable */
	int pno_suspend;		/* pno suspend status : "1" is pno suspended */
#endif /* PNO_SUPPORT */
	/* DTIM skip value, default 0(or 1) means wake each DTIM
	 * 3 means skip 2 DTIMs and wake up 3rd DTIM(9th beacon when AP DTIM is 3)
	 */
	int suspend_bcn_li_dtim;         /* bcn_li_dtim value in suspend mode */
#ifdef PKT_FILTER_SUPPORT
	int early_suspended;	/* Early suspend status */
	int dhcp_in_progress;	/* DHCP period */
#endif

	/* Pkt filter defination */
	char * pktfilter[100];
	int pktfilter_count;

	wl_country_t dhd_cspec;		/* Current Locale info */
#if defined(CUSTOM_COUNTRY_CODE) || defined(CUSTOM_FORCE_NODFS_FLAG)
	u32 dhd_cflags;
#endif /* CUSTOM_COUNTRY_CODE || CUSTOM_FORCE_NODFS_FLAG */
	bool force_country_change;
	char eventmask[WL_EVENTING_MASK_LEN];
	int	op_mode;				/* STA, HostAPD, WFD, SoftAP */

/* Set this to 1 to use a seperate interface (p2p0) for p2p operations.
 *  For ICS MR1 releases it should be disable to be compatable with ICS MR1 Framework
 *  see target dhd-cdc-sdmmc-panda-cfg80211-icsmr1-gpl-debug in Makefile
 */
/* #define WL_ENABLE_P2P_IF		1 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25))
	struct mutex 	wl_start_stop_lock; /* lock/unlock for Android start/stop */
	struct mutex 	wl_softap_lock;		 /* lock/unlock for any SoftAP/STA settings */
#endif 

#ifdef PROP_TXSTATUS
	bool	wlfc_enabled;
	int	wlfc_mode;
	void*	wlfc_state;
	/*
	Mode in which the dhd flow control shall operate. Must be set before
	traffic starts to the device.
	0 - Do not do any proptxtstatus flow control
	1 - Use implied credit from a packet status
	2 - Use explicit credit
	3 - Only AMPDU hostreorder used. no wlfc.
	*/
	uint8	proptxstatus_mode;
	bool	proptxstatus_txoff;
	bool	proptxstatus_module_ignore;
	bool	proptxstatus_credit_ignore;
	bool	proptxstatus_txstatus_ignore;

	bool	wlfc_rxpkt_chk;
#ifdef LIMIT_BORROW
	bool	wlfc_borrow_allowed;
#endif /* LIMIT_BORROW */
	/*
	 * implement below functions in each platform if needed.
	 */
	/* platform specific function whether to skip flow control */
	bool (*skip_fc)(void);
	/* platform specific function for wlfc_enable and wlfc_deinit */
	void (*plat_init)(void *dhd);
	void (*plat_deinit)(void *dhd);
#ifdef DHD_WLFC_THREAD
	bool                wlfc_thread_go;
	struct task_struct* wlfc_thread;
	wait_queue_head_t   wlfc_wqhead;
#endif /* DHD_WLFC_THREAD */
#endif /* PROP_TXSTATUS */
#ifdef PNO_SUPPORT
	void *pno_state;
#endif
#ifdef RTT_SUPPORT
	void *rtt_state;
#endif
	bool	dongle_isolation;
	bool	dongle_trap_occured;	/* flag for sending HANG event to upper layer */
	int   hang_was_sent;
	int   rxcnt_timeout;		/* counter rxcnt timeout to send HANG */
	int   txcnt_timeout;		/* counter txcnt timeout to send HANG */
#ifdef BCMPCIE
	int   d3ackcnt_timeout;		/* counter d3ack timeout to send HANG */
#endif /* BCMPCIE */
	bool hang_report;		/* enable hang report by default */
#ifdef WLMEDIA_HTSF
	uint8 htsfdlystat_sz; /* Size of delay stats, max 255B */
#endif
#ifdef WLTDLS
	bool tdls_enable;
#endif
	struct reorder_info *reorder_bufs[WLHOST_REORDERDATA_MAXFLOWS];
	#define WLC_IOCTL_MAXBUF_FWCAP	512
	char  fw_capabilities[WLC_IOCTL_MAXBUF_FWCAP];
	#define MAXSKBPEND 1024
	void *skbbuf[MAXSKBPEND];
	uint32 store_idx;
	uint32 sent_idx;
#ifdef DHDTCPACK_SUPPRESS
	uint8 tcpack_sup_mode;		/* TCPACK suppress mode */
	void *tcpack_sup_module;	/* TCPACK suppress module */
	uint32 tcpack_sup_ratio;
	uint32 tcpack_sup_delay;
#endif /* DHDTCPACK_SUPPRESS */
#if defined(ARP_OFFLOAD_SUPPORT)
	uint32 arp_version;
#endif
#ifdef CUSTOM_SET_CPUCORE
	struct task_struct * current_dpc;
	struct task_struct * current_rxf;
	int chan_isvht80;
#endif /* CUSTOM_SET_CPUCORE */


	void    *sta_pool;          /* pre-allocated pool of sta objects */
	void    *staid_allocator;   /* allocator of sta indexes */
#ifdef PCIE_FULL_DONGLE
	bool	flow_rings_inited;	/* set this flag after initializing flow rings */
#endif /* PCIE_FULL_DONGLE */
	void    *flowid_allocator;  /* unique flowid allocator */
	void	*flow_ring_table;   /* flow ring table, include prot and bus info */
	void	*if_flow_lkup;      /* per interface flowid lkup hash table */
	void    *flowid_lock;       /* per os lock for flowid info protection */
	void    *flowring_list_lock;       /* per os lock for flowring list protection */
	uint32  num_flow_rings;
	cumm_ctr_t cumm_ctr;        /* cumm queue length placeholder  */
	cumm_ctr_t l2cumm_ctr;      /* level 2 cumm queue length placeholder */
	uint32 d2h_sync_mode;       /* D2H DMA completion sync mode */
	uint8  flow_prio_map[NUMPRIO];
	uint8	flow_prio_map_type;
	char enable_log[MAX_EVENT];
	bool dma_d2h_ring_upd_support;
	bool dma_h2d_ring_upd_support;

#ifdef DHD_WMF
	bool wmf_ucast_igmp;
#ifdef DHD_IGMP_UCQUERY
	bool wmf_ucast_igmp_query;
#endif
#ifdef DHD_UCAST_UPNP
	bool wmf_ucast_upnp;
#endif
#endif /* DHD_WMF */
#if defined(TRAFFIC_MGMT_DWM)
	dhd_trf_mgmt_dwm_tbl_t dhd_tm_dwm_tbl;
#endif 
#ifdef DHD_L2_FILTER
	unsigned long l2_filter_cnt;	/* for L2_FILTER ARP table timeout */
#endif /* DHD_L2_FILTER */
	uint8 *soc_ram;
	uint32 soc_ram_length;
	uint32 memdump_type;
#ifdef DHD_FW_COREDUMP
	uint32 memdump_enabled;
#endif /* DHD_FW_COREDUMP */
#ifdef PCIE_FULL_DONGLE
#ifdef WLTDLS
	tdls_peer_tbl_t peer_tbl;
#endif /* WLTDLS */
	uint8 tx_in_progress;
#endif /* PCIE_FULL_DONGLE */
#ifdef DHD_ULP
	void *dhd_ulp;
#endif
#ifdef CACHE_FW_IMAGES
	char	*cached_fw;
	int	cached_fw_length;
	char	*cached_nvram;
	int	cached_nvram_length;
#endif
#ifdef WLTDLS
	uint32 tdls_mode;
#endif
#ifdef DHD_LOSSLESS_ROAMING
	uint8 dequeue_prec_map;
#endif
#ifdef GSCAN_SUPPORT
	bool lazy_roam_enable;
#endif
	uint8 rand_mac_oui[DOT11_OUI_LEN];
	void *macdbg_info;
#ifdef DHD_WET
	void *wet_info;
#endif
} dhd_pub_t;

#if defined(PCIE_FULL_DONGLE)

/* Packet Tag for PCIE Full Dongle DHD */
typedef struct dhd_pkttag_fd {
	uint16    flowid;   /* Flowring Id */
	uint16    dataoff;  /* start of packet */
	uint16    dma_len;  /* pkt len for DMA_MAP/UNMAP */
	dmaaddr_t pa;       /* physical address */
	void      *dmah;    /* dma mapper handle */
	void      *secdma; /* secure dma sec_cma_info handle */
} dhd_pkttag_fd_t;

/* Packet Tag for DHD PCIE Full Dongle */
#define DHD_PKTTAG_FD(pkt)          ((dhd_pkttag_fd_t *)(PKTTAG(pkt)))

#define DHD_PKT_GET_FLOWID(pkt)     ((DHD_PKTTAG_FD(pkt))->flowid)
#define DHD_PKT_SET_FLOWID(pkt, pkt_flowid) \
	DHD_PKTTAG_FD(pkt)->flowid = (uint16)(pkt_flowid)

#define DHD_PKT_GET_DATAOFF(pkt)    ((DHD_PKTTAG_FD(pkt))->dataoff)
#define DHD_PKT_SET_DATAOFF(pkt, pkt_dataoff) \
	DHD_PKTTAG_FD(pkt)->dataoff = (uint16)(pkt_dataoff)

#define DHD_PKT_GET_DMA_LEN(pkt)    ((DHD_PKTTAG_FD(pkt))->dma_len)
#define DHD_PKT_SET_DMA_LEN(pkt, pkt_dma_len) \
	DHD_PKTTAG_FD(pkt)->dma_len = (uint16)(pkt_dma_len)

#define DHD_PKT_GET_PA(pkt)         ((DHD_PKTTAG_FD(pkt))->pa)
#define DHD_PKT_SET_PA(pkt, pkt_pa) \
	DHD_PKTTAG_FD(pkt)->pa = (dmaaddr_t)(pkt_pa)

#define DHD_PKT_GET_DMAH(pkt)       ((DHD_PKTTAG_FD(pkt))->dmah)
#define DHD_PKT_SET_DMAH(pkt, pkt_dmah) \
	DHD_PKTTAG_FD(pkt)->dmah = (void *)(pkt_dmah)

#define DHD_PKT_GET_SECDMA(pkt)    ((DHD_PKTTAG_FD(pkt))->secdma)
#define DHD_PKT_SET_SECDMA(pkt, pkt_secdma) \
	DHD_PKTTAG_FD(pkt)->secdma = (void *)(pkt_secdma)
#endif /* PCIE_FULL_DONGLE */

#if defined(BCMWDF)
typedef struct {
	dhd_pub_t *dhd_pub;
} dhd_workitem_context_t;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(dhd_workitem_context_t, dhd_get_dhd_workitem_context)
#endif /* (BCMWDF)  */

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)

	#define DHD_PM_RESUME_WAIT_INIT(a) DECLARE_WAIT_QUEUE_HEAD(a);
	#define _DHD_PM_RESUME_WAIT(a, b) do {\
			int retry = 0; \
			SMP_RD_BARRIER_DEPENDS(); \
			while (dhd_mmc_suspend && retry++ != b) { \
				SMP_RD_BARRIER_DEPENDS(); \
				wait_event_interruptible_timeout(a, !dhd_mmc_suspend, 1); \
			} \
		} 	while (0)
	#define DHD_PM_RESUME_WAIT(a) 		_DHD_PM_RESUME_WAIT(a, 200)
	#define DHD_PM_RESUME_WAIT_FOREVER(a) 	_DHD_PM_RESUME_WAIT(a, ~0)
	#ifdef CUSTOMER_HW4
		#define DHD_PM_RESUME_RETURN_ERROR(a)   do { \
				if (dhd_mmc_suspend) { \
					printf("%s[%d]: mmc is still in suspend state!!!\n", \
							__FUNCTION__, __LINE__); \
					return a; \
				} \
			} while (0)
	#else
		#define DHD_PM_RESUME_RETURN_ERROR(a)	do { \
			if (dhd_mmc_suspend) return a; } while (0)
	#endif 
	#define DHD_PM_RESUME_RETURN		do { if (dhd_mmc_suspend) return; } while (0)

	#define DHD_SPINWAIT_SLEEP_INIT(a) DECLARE_WAIT_QUEUE_HEAD(a);
	#define SPINWAIT_SLEEP(a, exp, us) do { \
		uint countdown = (us) + 9999; \
		while ((exp) && (countdown >= 10000)) { \
			wait_event_interruptible_timeout(a, FALSE, 1); \
			countdown -= 10000; \
		} \
	} while (0)

	#else

	#define DHD_PM_RESUME_WAIT_INIT(a)
	#define DHD_PM_RESUME_WAIT(a)
	#define DHD_PM_RESUME_WAIT_FOREVER(a)
	#define DHD_PM_RESUME_RETURN_ERROR(a)
	#define DHD_PM_RESUME_RETURN

	#define DHD_SPINWAIT_SLEEP_INIT(a)
	#define SPINWAIT_SLEEP(a, exp, us)  do { \
		uint countdown = (us) + 9; \
		while ((exp) && (countdown >= 10)) { \
			OSL_DELAY(10);  \
			countdown -= 10;  \
		} \
	} while (0)

	#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP) */

#ifndef OSL_SLEEP
#define OSL_SLEEP(ms)		OSL_DELAY(ms*1000)
#endif /* OSL_SLEEP */

#define DHD_IF_VIF	0x01	/* Virtual IF (Hidden from user) */

#ifdef PNO_SUPPORT
int dhd_pno_clean(dhd_pub_t *dhd);
#endif /* PNO_SUPPORT */
/*
 *  Wake locks are an Android power management concept. They are used by applications and services
 *  to request CPU resources.
 */
extern int dhd_os_wake_lock(dhd_pub_t *pub);
extern int dhd_os_wake_unlock(dhd_pub_t *pub);
extern int dhd_os_wake_lock_timeout(dhd_pub_t *pub);
extern int dhd_os_wake_lock_rx_timeout_enable(dhd_pub_t *pub, int val);
extern int dhd_os_wake_lock_ctrl_timeout_enable(dhd_pub_t *pub, int val);
extern int dhd_os_wake_lock_ctrl_timeout_cancel(dhd_pub_t *pub);
extern int dhd_os_wd_wake_lock(dhd_pub_t *pub);
extern int dhd_os_wd_wake_unlock(dhd_pub_t *pub);
extern int dhd_os_wake_lock_waive(dhd_pub_t *pub);
extern int dhd_os_wake_lock_restore(dhd_pub_t *pub);
#ifdef BCMPCIE_OOB_HOST_WAKE
extern void dhd_os_oob_irq_wake_lock_timeout(dhd_pub_t *pub, int val);
extern void dhd_os_oob_irq_wake_unlock(dhd_pub_t *pub);
#endif /* BCMPCIE_OOB_HOST_WAKE */
#ifdef DHD_USE_SCAN_WAKELOCK
extern void dhd_os_scan_wake_lock_timeout(dhd_pub_t *pub, int val);
extern void dhd_os_scan_wake_unlock(dhd_pub_t *pub);
#endif /* BCMPCIE_SCAN_WAKELOCK */

inline static void MUTEX_LOCK_SOFTAP_SET_INIT(dhd_pub_t * dhdp)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25))
	mutex_init(&dhdp->wl_softap_lock);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
}

inline static void MUTEX_LOCK_SOFTAP_SET(dhd_pub_t * dhdp)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25))
	mutex_lock(&dhdp->wl_softap_lock);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
}

inline static void MUTEX_UNLOCK_SOFTAP_SET(dhd_pub_t * dhdp)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25))
	mutex_unlock(&dhdp->wl_softap_lock);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
}

#define DHD_OS_WAKE_LOCK(pub)			dhd_os_wake_lock(pub)
#define DHD_OS_WAKE_UNLOCK(pub)		dhd_os_wake_unlock(pub)
#define DHD_OS_WAKE_LOCK_TIMEOUT(pub)		dhd_os_wake_lock_timeout(pub)
#define DHD_OS_WAKE_LOCK_RX_TIMEOUT_ENABLE(pub, val) \
	dhd_os_wake_lock_rx_timeout_enable(pub, val)
#define DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE(pub, val) \
	dhd_os_wake_lock_ctrl_timeout_enable(pub, val)
#define DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_CANCEL(pub) \
	dhd_os_wake_lock_ctrl_timeout_cancel(pub)
#define DHD_OS_WAKE_LOCK_WAIVE(pub)			dhd_os_wake_lock_waive(pub)
#define DHD_OS_WAKE_LOCK_RESTORE(pub)		dhd_os_wake_lock_restore(pub)

#define DHD_OS_WD_WAKE_LOCK(pub)		dhd_os_wd_wake_lock(pub)
#define DHD_OS_WD_WAKE_UNLOCK(pub)		dhd_os_wd_wake_unlock(pub)

#ifdef BCMPCIE_OOB_HOST_WAKE
#define OOB_WAKE_LOCK_TIMEOUT 500
#define DHD_OS_OOB_IRQ_WAKE_LOCK_TIMEOUT(pub, val)	dhd_os_oob_irq_wake_lock_timeout(pub, val)
#define DHD_OS_OOB_IRQ_WAKE_UNLOCK(pub)			dhd_os_oob_irq_wake_unlock(pub)
#endif /* BCMPCIE_OOB_HOST_WAKE */
#ifdef DHD_USE_SCAN_WAKELOCK
#ifdef DHD_DEBUG_SCAN_WAKELOCK
#define DHD_OS_SCAN_WAKE_LOCK_TIMEOUT(pub, val) \
	do { \
		printf("call wake_lock_scan: %s %d\n", \
			__FUNCTION__, __LINE__); \
		dhd_os_scan_wake_lock_timeout(pub, val); \
	} while (0)
#define DHD_OS_SCAN_WAKE_UNLOCK(pub) \
	do { \
		printf("call wake_unlock_scan: %s %d\n", \
			__FUNCTION__, __LINE__); \
		dhd_os_scan_wake_unlock(pub); \
	} while (0)
#else
#define DHD_OS_SCAN_WAKE_LOCK_TIMEOUT(pub, val)		dhd_os_scan_wake_lock_timeout(pub, val)
#define DHD_OS_SCAN_WAKE_UNLOCK(pub)			dhd_os_scan_wake_unlock(pub)
#endif /* DHD_DEBUG_SCAN_WAKELOCK */
#else
#define DHD_OS_SCAN_WAKE_LOCK_TIMEOUT(pub, val)
#define DHD_OS_SCAN_WAKE_UNLOCK(pub)
#endif /* DHD_USE_SCAN_WAKELOCK */
#define DHD_PACKET_TIMEOUT_MS	500
#define DHD_EVENT_TIMEOUT_MS	1500
#define SCAN_WAKE_LOCK_TIMEOUT	10000

/* Enum for IOCTL recieved status */
typedef enum dhd_ioctl_recieved_status
{
	IOCTL_WAIT = 0,
	IOCTL_RETURN_ON_SUCCESS,
	IOCTL_RETURN_ON_TRAP,
	IOCTL_RETURN_ON_BUS_STOP
} dhd_ioctl_recieved_status_t;

/* interface operations (register, remove) should be atomic, use this lock to prevent race
 * condition among wifi on/off and interface operation functions
 */
void dhd_net_if_lock(struct net_device *dev);
void dhd_net_if_unlock(struct net_device *dev);


typedef enum dhd_attach_states
{
	DHD_ATTACH_STATE_INIT = 0x0,
	DHD_ATTACH_STATE_NET_ALLOC = 0x1,
	DHD_ATTACH_STATE_DHD_ALLOC = 0x2,
	DHD_ATTACH_STATE_ADD_IF = 0x4,
	DHD_ATTACH_STATE_PROT_ATTACH = 0x8,
	DHD_ATTACH_STATE_WL_ATTACH = 0x10,
	DHD_ATTACH_STATE_THREADS_CREATED = 0x20,
	DHD_ATTACH_STATE_WAKELOCKS_INIT = 0x40,
	DHD_ATTACH_STATE_CFG80211 = 0x80,
	DHD_ATTACH_STATE_EARLYSUSPEND_DONE = 0x100,
	DHD_ATTACH_STATE_DONE = 0x200
} dhd_attach_states_t;

/* Value -1 means we are unsuccessful in creating the kthread. */
#define DHD_PID_KT_INVALID 	-1
/* Value -2 means we are unsuccessful in both creating the kthread and tasklet */
#define DHD_PID_KT_TL_INVALID	-2

/*
 * Exported from dhd OS modules (dhd_linux/dhd_ndis)
 */

/* Indication from bus module regarding presence/insertion of dongle.
 * Return dhd_pub_t pointer, used as handle to OS module in later calls.
 * Returned structure should have bus and prot pointers filled in.
 * bus_hdrlen specifies required headroom for bus module header.
 */
extern dhd_pub_t *dhd_attach(osl_t *osh, struct dhd_bus *bus, uint bus_hdrlen);
#if defined(WLP2P) && defined(WL_CFG80211)
/* To allow attach/detach calls corresponding to p2p0 interface  */
extern int dhd_attach_p2p(dhd_pub_t *);
extern int dhd_detach_p2p(dhd_pub_t *);
#endif /* WLP2P && WL_CFG80211 */
extern int dhd_register_if(dhd_pub_t *dhdp, int idx, bool need_rtnl_lock);

/* Indication from bus module regarding removal/absence of dongle */
extern void dhd_detach(dhd_pub_t *dhdp);
extern void dhd_free(dhd_pub_t *dhdp);
extern void dhd_clear(dhd_pub_t *dhdp);

/* Indication from bus module to change flow-control state */
extern void dhd_txflowcontrol(dhd_pub_t *dhdp, int ifidx, bool on);

/* Store the status of a connection attempt for later retrieval by an iovar */
extern void dhd_store_conn_status(uint32 event, uint32 status, uint32 reason);

extern bool dhd_prec_enq(dhd_pub_t *dhdp, struct pktq *q, void *pkt, int prec);

/* Receive frame for delivery to OS.  Callee disposes of rxp. */
extern void dhd_rx_frame(dhd_pub_t *dhdp, int ifidx, void *rxp, int numpkt, uint8 chan);

/* Return pointer to interface name */
extern char *dhd_ifname(dhd_pub_t *dhdp, int idx);

/* Request scheduling of the bus dpc */
extern void dhd_sched_dpc(dhd_pub_t *dhdp);

/* Notify tx completion */
extern void dhd_txcomplete(dhd_pub_t *dhdp, void *txp, bool success);
extern void dhd_dpc_enable(dhd_pub_t *dhdp);

#define WIFI_FEATURE_INFRA              0x0001      /* Basic infrastructure mode        */
#define WIFI_FEATURE_INFRA_5G           0x0002      /* Support for 5 GHz Band           */
#define WIFI_FEATURE_HOTSPOT            0x0004      /* Support for GAS/ANQP             */
#define WIFI_FEATURE_P2P                0x0008      /* Wifi-Direct                      */
#define WIFI_FEATURE_SOFT_AP            0x0010      /* Soft AP                          */
#define WIFI_FEATURE_GSCAN              0x0020      /* Google-Scan APIs                 */
#define WIFI_FEATURE_NAN                0x0040      /* Neighbor Awareness Networking    */
#define WIFI_FEATURE_D2D_RTT            0x0080      /* Device-to-device RTT             */
#define WIFI_FEATURE_D2AP_RTT           0x0100      /* Device-to-AP RTT                 */
#define WIFI_FEATURE_BATCH_SCAN         0x0200      /* Batched Scan (legacy)            */
#define WIFI_FEATURE_PNO                0x0400      /* Preferred network offload        */
#define WIFI_FEATURE_ADDITIONAL_STA     0x0800      /* Support for two STAs             */
#define WIFI_FEATURE_TDLS               0x1000      /* Tunnel directed link setup       */
#define WIFI_FEATURE_TDLS_OFFCHANNEL    0x2000      /* Support for TDLS off channel     */
#define WIFI_FEATURE_EPR                0x4000      /* Enhanced power reporting         */
#define WIFI_FEATURE_AP_STA             0x8000      /* Support for AP STA Concurrency   */
#define WIFI_FEATURE_LINKSTAT           0x10000     /* Support for Linkstats            */
#define WIFI_FEATURE_HAL_EPNO           0x40000     /* WiFi PNO enhanced                */
#define WIFI_FEATURE_RSSI_MONITOR       0x80000     /* RSSI Monitor                     */

#define MAX_FEATURE_SET_CONCURRRENT_GROUPS  3

extern int dhd_dev_get_feature_set(struct net_device *dev);
extern int *dhd_dev_get_feature_set_matrix(struct net_device *dev, int *num);
#ifdef CUSTOM_FORCE_NODFS_FLAG
extern int dhd_dev_set_nodfs(struct net_device *dev, uint nodfs);
#endif /* CUSTOM_FORCE_NODFS_FLAG */
extern int dhd_dev_cfg_rand_mac_oui(struct net_device *dev, uint8 *oui);
extern int dhd_set_rand_mac_oui(dhd_pub_t *dhd);

#ifdef GSCAN_SUPPORT
extern int dhd_dev_set_lazy_roam_cfg(struct net_device *dev,
             wlc_roam_exp_params_t *roam_param);
extern int dhd_dev_lazy_roam_enable(struct net_device *dev, uint32 enable);
extern int dhd_dev_set_lazy_roam_bssid_pref(struct net_device *dev,
       wl_bssid_pref_cfg_t *bssid_pref, uint32 flush);
extern int dhd_dev_set_blacklist_bssid(struct net_device *dev, maclist_t *blacklist,
    uint32 len, uint32 flush);
extern int dhd_dev_set_whitelist_ssid(struct net_device *dev, wl_ssid_whitelist_t *whitelist,
    uint32 len, uint32 flush);
#endif /* GSCAN_SUPPORT */

extern int dhd_dev_set_rssi_monitor_cfg(struct net_device *dev, int start,
             int8 max_rssi, int8 min_rssi);

#define DHD_RSSI_MONITOR_EVT_VERSION   1
typedef struct {
	uint8 version;
	int8 cur_rssi;
	struct ether_addr BSSID;
} dhd_rssi_monitor_evt_t;

/* OS independent layer functions */
extern int dhd_os_proto_block(dhd_pub_t * pub);
extern int dhd_os_proto_unblock(dhd_pub_t * pub);
extern int dhd_os_ioctl_resp_wait(dhd_pub_t * pub, uint * condition);
extern int dhd_os_ioctl_resp_wake(dhd_pub_t * pub);
extern unsigned int dhd_os_get_ioctl_resp_timeout(void);
extern void dhd_os_set_ioctl_resp_timeout(unsigned int timeout_msec);
extern void dhd_os_ioctl_resp_lock(dhd_pub_t * pub);
extern void dhd_os_ioctl_resp_unlock(dhd_pub_t * pub);
extern void dhd_wakeup_ioctl_event(dhd_pub_t *pub, dhd_ioctl_recieved_status_t reason);

#define DHD_OS_IOCTL_RESP_LOCK(x)
#define DHD_OS_IOCTL_RESP_UNLOCK(x)


extern int dhd_os_get_image_block(char * buf, int len, void * image);
extern void * dhd_os_open_image(char * filename);
extern void dhd_os_close_image(void * image);
extern void dhd_os_wd_timer(void *bus, uint wdtick);
extern void dhd_os_sdlock(dhd_pub_t * pub);
extern void dhd_os_sdunlock(dhd_pub_t * pub);
extern void dhd_os_sdlock_txq(dhd_pub_t * pub);
extern void dhd_os_sdunlock_txq(dhd_pub_t * pub);
extern void dhd_os_sdlock_rxq(dhd_pub_t * pub);
extern void dhd_os_sdunlock_rxq(dhd_pub_t * pub);
extern void dhd_os_sdlock_sndup_rxq(dhd_pub_t * pub);
#ifdef DHDTCPACK_SUPPRESS
extern unsigned long dhd_os_tcpacklock(dhd_pub_t *pub);
extern void dhd_os_tcpackunlock(dhd_pub_t *pub, unsigned long flags);
#endif /* DHDTCPACK_SUPPRESS */

extern int dhd_customer_oob_irq_map(void *adapter, unsigned long *irq_flags_ptr);
extern int dhd_customer_gpio_wlan_ctrl(void *adapter, int onoff);
extern int dhd_custom_get_mac_address(void *adapter, unsigned char *buf);
#ifdef CUSTOM_COUNTRY_CODE
extern void get_customized_country_code(void *adapter, char *country_iso_code,
wl_country_t *cspec, u32 flags);
#else
extern void get_customized_country_code(void *adapter, char *country_iso_code, wl_country_t *cspec);
#endif /* CUSTOM_COUNTRY_CODE */
extern void dhd_os_sdunlock_sndup_rxq(dhd_pub_t * pub);
extern void dhd_os_sdlock_eventq(dhd_pub_t * pub);
extern void dhd_os_sdunlock_eventq(dhd_pub_t * pub);
extern bool dhd_os_check_hang(dhd_pub_t *dhdp, int ifidx, int ret);
extern int dhd_os_send_hang_message(dhd_pub_t *dhdp);
extern void dhd_set_version_info(dhd_pub_t *pub, char *fw);
extern bool dhd_os_check_if_up(dhd_pub_t *pub);
extern int dhd_os_check_wakelock(dhd_pub_t *pub);
extern int dhd_os_check_wakelock_all(dhd_pub_t *pub);
extern int dhd_get_instance(dhd_pub_t *pub);
#ifdef CUSTOM_SET_CPUCORE
extern void dhd_set_cpucore(dhd_pub_t *dhd, int set);
#endif /* CUSTOM_SET_CPUCORE */

#if defined(KEEP_ALIVE)
extern int dhd_keep_alive_onoff(dhd_pub_t *dhd);
#endif /* KEEP_ALIVE */

#ifdef SUPPORT_AP_POWERSAVE
extern int dhd_set_ap_powersave(dhd_pub_t *dhdp, int ifidx, int enable);
#endif

#if defined(DHD_FW_COREDUMP)
void dhd_schedule_memdump(dhd_pub_t *dhdp, uint8 *buf, uint32 size);
#endif /* DHD_FW_COREDUMP */

#ifdef SUPPORT_AP_POWERSAVE
extern int dhd_set_ap_powersave(dhd_pub_t *dhdp, int ifidx, int enable);
#endif /* SUPPORT_AP_POWERSAVE */


#ifdef PKT_FILTER_SUPPORT
#define DHD_UNICAST_FILTER_NUM		0
#define DHD_BROADCAST_FILTER_NUM	1
#define DHD_MULTICAST4_FILTER_NUM	2
#define DHD_MULTICAST6_FILTER_NUM	3
#define DHD_MDNS_FILTER_NUM		4
#define DHD_ARP_FILTER_NUM		5
extern int 	dhd_os_enable_packet_filter(dhd_pub_t *dhdp, int val);
extern void dhd_enable_packet_filter(int value, dhd_pub_t *dhd);
extern int net_os_enable_packet_filter(struct net_device *dev, int val);
extern int net_os_rxfilter_add_remove(struct net_device *dev, int val, int num);
#endif /* PKT_FILTER_SUPPORT */

extern int dhd_get_suspend_bcn_li_dtim(dhd_pub_t *dhd);
extern bool dhd_support_sta_mode(dhd_pub_t *dhd);

extern int write_to_file(dhd_pub_t *dhd, uint8 *buf, int size);
typedef struct {
	uint32 limit;		/* Expiration time (usec) */
	uint32 increment;	/* Current expiration increment (usec) */
	uint32 elapsed;		/* Current elapsed time (usec) */
	uint32 tick;		/* O/S tick time (usec) */
} dhd_timeout_t;

#ifdef SHOW_LOGTRACE
typedef struct {
	int  num_fmts;
	char **fmts;
	char *raw_fmts;
	char *raw_sstr;
	uint32 ramstart;
	uint32 rodata_start;
	uint32 rodata_end;
	char *rom_raw_sstr;
	uint32 rom_ramstart;
	uint32 rom_rodata_start;
	uint32 rom_rodata_end;
} dhd_event_log_t;
#endif /* SHOW_LOGTRACE */

#if defined(KEEP_ALIVE)
extern int dhd_dev_start_mkeep_alive(dhd_pub_t *dhd_pub, u8 mkeep_alive_id, u8 *ip_pkt,
	u16 ip_pkt_len, u8* src_mac_addr, u8* dst_mac_addr, u32 period_msec);
extern int dhd_dev_stop_mkeep_alive(dhd_pub_t *dhd_pub, u8 mkeep_alive_id);
#endif /* defined(KEEP_ALIVE) */

extern void dhd_timeout_start(dhd_timeout_t *tmo, uint usec);
extern int dhd_timeout_expired(dhd_timeout_t *tmo);

extern int dhd_ifname2idx(struct dhd_info *dhd, char *name);
extern int dhd_net2idx(struct dhd_info *dhd, struct net_device *net);
extern struct net_device * dhd_idx2net(void *pub, int ifidx);
extern int net_os_send_hang_message(struct net_device *dev);
extern bool dhd_wowl_cap(void *bus);

extern int wl_host_event(dhd_pub_t *dhd_pub, int *idx, void *pktdata,
                         wl_event_msg_t *, void **data_ptr,  void *);
extern void wl_event_to_host_order(wl_event_msg_t * evt);
extern int wl_host_event_get_data(void *pktdata, wl_event_msg_t *event, void **data_ptr);

extern int dhd_wl_ioctl(dhd_pub_t *dhd_pub, int ifindex, wl_ioctl_t *ioc, void *buf, int len);
extern int dhd_wl_ioctl_cmd(dhd_pub_t *dhd_pub, int cmd, void *arg, int len, uint8 set,
                            int ifindex);
extern int dhd_wl_ioctl_get_intiovar(dhd_pub_t *dhd_pub, char *name, uint *pval,
	int cmd, uint8 set, int ifidx);
extern int dhd_wl_ioctl_set_intiovar(dhd_pub_t *dhd_pub, char *name, uint val,
	int cmd, uint8 set, int ifidx);
extern void dhd_common_init(osl_t *osh);

extern int dhd_do_driver_init(struct net_device *net);
extern int dhd_event_ifadd(struct dhd_info *dhd, struct wl_event_data_if *ifevent,
	char *name, uint8 *mac);
extern int dhd_event_ifdel(struct dhd_info *dhd, struct wl_event_data_if *ifevent,
	char *name, uint8 *mac);
extern struct net_device* dhd_allocate_if(dhd_pub_t *dhdpub, int ifidx, char *name,
	uint8 *mac, uint8 bssidx, bool need_rtnl_lock, char *dngl_name);
extern int dhd_remove_if(dhd_pub_t *dhdpub, int ifidx, bool need_rtnl_lock);
extern void dhd_vif_add(struct dhd_info *dhd, int ifidx, char * name);
extern void dhd_vif_del(struct dhd_info *dhd, int ifidx);
extern void dhd_event(struct dhd_info *dhd, char *evpkt, int evlen, int ifidx);
extern void dhd_vif_sendup(struct dhd_info *dhd, int ifidx, uchar *cp, int len);

/* Send packet to dongle via data channel */
extern int dhd_sendpkt(dhd_pub_t *dhdp, int ifidx, void *pkt);

/* send up locally generated event */
extern void dhd_sendup_event_common(dhd_pub_t *dhdp, wl_event_msg_t *event, void *data);
/* Send event to host */
extern void dhd_sendup_event(dhd_pub_t *dhdp, wl_event_msg_t *event, void *data);
#ifdef LOG_INTO_TCPDUMP
extern void dhd_sendup_log(dhd_pub_t *dhdp, void *data, int len);
#endif /* LOG_INTO_TCPDUMP */
extern int dhd_bus_devreset(dhd_pub_t *dhdp, uint8 flag);
extern uint dhd_bus_status(dhd_pub_t *dhdp);
extern int  dhd_bus_start(dhd_pub_t *dhdp);
extern int dhd_bus_suspend(dhd_pub_t *dhdpub);
extern int dhd_bus_resume(dhd_pub_t *dhdpub, int stage);
extern int dhd_bus_membytes(dhd_pub_t *dhdp, bool set, uint32 address, uint8 *data, uint size);
extern void dhd_print_buf(void *pbuf, int len, int bytes_per_line);
extern bool dhd_is_associated(dhd_pub_t *dhd, void *bss_buf, int *retval);
#if defined(BCMSDIO) || defined(BCMPCIE)
extern uint dhd_bus_chip_id(dhd_pub_t *dhdp);
extern uint dhd_bus_chiprev_id(dhd_pub_t *dhdp);
extern uint dhd_bus_chippkg_id(dhd_pub_t *dhdp);
#endif /* defined(BCMSDIO) || defined(BCMPCIE) */

#if defined(KEEP_ALIVE)
extern int dhd_keep_alive_onoff(dhd_pub_t *dhd);
#endif /* KEEP_ALIVE */

/* OS spin lock API */
extern void *dhd_os_spin_lock_init(osl_t *osh);
extern void dhd_os_spin_lock_deinit(osl_t *osh, void *lock);
extern unsigned long dhd_os_spin_lock(void *lock);
void dhd_os_spin_unlock(void *lock, unsigned long flags);

/*
 * Manage sta objects in an interface. Interface is identified by an ifindex and
 * sta(s) within an interfaces are managed using a MacAddress of the sta.
 */
struct dhd_sta;
extern struct dhd_sta *dhd_find_sta(void *pub, int ifidx, void *ea);
extern struct dhd_sta *dhd_findadd_sta(void *pub, int ifidx, void *ea);
extern void dhd_del_sta(void *pub, int ifidx, void *ea);
extern int dhd_get_ap_isolate(dhd_pub_t *dhdp, uint32 idx);
extern int dhd_set_ap_isolate(dhd_pub_t *dhdp, uint32 idx, int val);
#if defined(BCM_GMAC3)
extern int dhd_set_dev_def(dhd_pub_t *dhdp, uint32 idx, int val);
#endif
extern int dhd_bssidx2idx(dhd_pub_t *dhdp, uint32 bssidx);
extern int dhd_os_d3ack_wait(dhd_pub_t * pub, uint * condition);
extern int dhd_os_d3ack_wake(dhd_pub_t * pub);
extern struct net_device *dhd_linux_get_primary_netdev(dhd_pub_t *dhdp);
extern int dhd_os_busbusy_wait_negation(dhd_pub_t * pub, uint * condition);
extern int dhd_os_busbusy_wake(dhd_pub_t * pub);

extern bool dhd_is_concurrent_mode(dhd_pub_t *dhd);
int dhd_iovar(dhd_pub_t *pub, int ifidx, char *name, char *param_buf, uint param_len,
		char *res_buf, uint res_len, int set);
extern int dhd_getiovar(dhd_pub_t *pub, int ifidx, char *name, char *cmd_buf,
		uint cmd_len, char **resptr, uint resp_len);

#ifdef DHD_MCAST_REGEN
extern int dhd_get_mcast_regen_bss_enable(dhd_pub_t *dhdp, uint32 idx);
extern int dhd_set_mcast_regen_bss_enable(dhd_pub_t *dhdp, uint32 idx, int val);
#endif
typedef enum cust_gpio_modes {
	WLAN_RESET_ON,
	WLAN_RESET_OFF,
	WLAN_POWER_ON,
	WLAN_POWER_OFF
} cust_gpio_modes_t;

extern int wl_iw_iscan_set_scan_broadcast_prep(struct net_device *dev, uint flag);
extern int wl_iw_send_priv_event(struct net_device *dev, char *flag);
/*
 * Insmod parameters for debug/test
 */

/* Watchdog timer interval */
extern uint dhd_watchdog_ms;

#if defined(DHD_DEBUG)
/* Console output poll interval */
extern uint dhd_console_ms;
extern uint wl_msg_level;
#endif /* defined(DHD_DEBUG) */

extern uint dhd_slpauto;

/* Use interrupts */
extern uint dhd_intr;

/* Use polling */
extern uint dhd_poll;

/* ARP offload agent mode */
extern uint dhd_arp_mode;

/* ARP offload enable */
extern uint dhd_arp_enable;

/* Pkt filte enable control */
extern uint dhd_pkt_filter_enable;

/*  Pkt filter init setup */
extern uint dhd_pkt_filter_init;

/* Pkt filter mode control */
extern uint dhd_master_mode;

/* Roaming mode control */
extern uint dhd_roam_disable;

/* Roaming mode control */
extern uint dhd_radio_up;

/* Initial idletime ticks (may be -1 for immediate idle, 0 for no idle) */
extern int dhd_idletime;
#ifdef DHD_USE_IDLECOUNT
#define DHD_IDLETIME_TICKS 5
#else
#define DHD_IDLETIME_TICKS 1
#endif /* DHD_USE_IDLECOUNT */

/* SDIO Drive Strength */
extern uint dhd_sdiod_drive_strength;

/* triggers bcm_bprintf to print to kernel log */
extern bool bcm_bprintf_bypass;

/* Override to force tx queueing all the time */
extern uint dhd_force_tx_queueing;
/* Default KEEP_ALIVE Period is 55 sec to prevent AP from sending Keep Alive probe frame */
#define DEFAULT_KEEP_ALIVE_VALUE 	55000 /* msec */
#ifndef CUSTOM_KEEP_ALIVE_SETTING
#define CUSTOM_KEEP_ALIVE_SETTING 	DEFAULT_KEEP_ALIVE_VALUE
#endif /* DEFAULT_KEEP_ALIVE_VALUE */

#define NULL_PKT_STR	"null_pkt"

/* hooks for custom glom setting option via Makefile */
#define DEFAULT_GLOM_VALUE 	-1
#ifndef CUSTOM_GLOM_SETTING
#define CUSTOM_GLOM_SETTING 	DEFAULT_GLOM_VALUE
#endif
#define WL_AUTO_ROAM_TRIGGER -75
/* hooks for custom Roaming Trigger  setting via Makefile */
#define DEFAULT_ROAM_TRIGGER_VALUE -75 /* dBm default roam trigger all band */
#define DEFAULT_ROAM_TRIGGER_SETTING 	-1
#ifndef CUSTOM_ROAM_TRIGGER_SETTING
#define CUSTOM_ROAM_TRIGGER_SETTING 	DEFAULT_ROAM_TRIGGER_VALUE
#endif

/* hooks for custom Roaming Romaing  setting via Makefile */
#define DEFAULT_ROAM_DELTA_VALUE  10 /* dBm default roam delta all band */
#define DEFAULT_ROAM_DELTA_SETTING 	-1
#ifndef CUSTOM_ROAM_DELTA_SETTING
#define CUSTOM_ROAM_DELTA_SETTING 	DEFAULT_ROAM_DELTA_VALUE
#endif

/* hooks for custom PNO Event wake lock to guarantee enough time
	for the Platform to detect Event before system suspended
*/
#define DEFAULT_PNO_EVENT_LOCK_xTIME 	2 	/* multiplay of DHD_PACKET_TIMEOUT_MS */
#ifndef CUSTOM_PNO_EVENT_LOCK_xTIME
#define CUSTOM_PNO_EVENT_LOCK_xTIME	 DEFAULT_PNO_EVENT_LOCK_xTIME
#endif
/* hooks for custom dhd_dpc_prio setting option via Makefile */
#define DEFAULT_DHP_DPC_PRIO  1
#ifndef CUSTOM_DPC_PRIO_SETTING
#define CUSTOM_DPC_PRIO_SETTING 	DEFAULT_DHP_DPC_PRIO
#endif

#ifndef CUSTOM_LISTEN_INTERVAL
#define CUSTOM_LISTEN_INTERVAL 		LISTEN_INTERVAL
#endif /* CUSTOM_LISTEN_INTERVAL */

#define DEFAULT_SUSPEND_BCN_LI_DTIM		3
#ifndef CUSTOM_SUSPEND_BCN_LI_DTIM
#define CUSTOM_SUSPEND_BCN_LI_DTIM		DEFAULT_SUSPEND_BCN_LI_DTIM
#endif

#ifndef CUSTOM_RXF_PRIO_SETTING
#define CUSTOM_RXF_PRIO_SETTING		MAX((CUSTOM_DPC_PRIO_SETTING - 1), 1)
#endif

#define DEFAULT_WIFI_TURNOFF_DELAY		0
#define WIFI_TURNOFF_DELAY		DEFAULT_WIFI_TURNOFF_DELAY

#define DEFAULT_WIFI_TURNON_DELAY		200
#ifndef WIFI_TURNON_DELAY
#define WIFI_TURNON_DELAY		DEFAULT_WIFI_TURNON_DELAY
#endif /* WIFI_TURNON_DELAY */

#define DEFAULT_DHD_WATCHDOG_INTERVAL_MS	10 /* msec */
#ifndef CUSTOM_DHD_WATCHDOG_MS
#define CUSTOM_DHD_WATCHDOG_MS			DEFAULT_DHD_WATCHDOG_INTERVAL_MS
#endif /* DEFAULT_DHD_WATCHDOG_INTERVAL_MS */

#define DEFAULT_ASSOC_RETRY_MAX			3
#ifndef CUSTOM_ASSOC_RETRY_MAX
#define CUSTOM_ASSOC_RETRY_MAX			DEFAULT_ASSOC_RETRY_MAX
#endif /* DEFAULT_ASSOC_RETRY_MAX */


#ifdef WLTDLS
#ifndef CUSTOM_TDLS_IDLE_MODE_SETTING
#define CUSTOM_TDLS_IDLE_MODE_SETTING  60000 /* 60sec to tear down TDLS of not active */
#endif
#ifndef CUSTOM_TDLS_RSSI_THRESHOLD_HIGH
#define CUSTOM_TDLS_RSSI_THRESHOLD_HIGH -70 /* rssi threshold for establishing TDLS link */
#endif
#ifndef CUSTOM_TDLS_RSSI_THRESHOLD_LOW
#define CUSTOM_TDLS_RSSI_THRESHOLD_LOW -80 /* rssi threshold for tearing down TDLS link */
#endif
#endif /* WLTDLS */

#define DEFAULT_BCN_TIMEOUT		8
#ifndef CUSTOM_BCN_TIMEOUT
#define CUSTOM_BCN_TIMEOUT		DEFAULT_BCN_TIMEOUT
#endif

#define MAX_DTIM_SKIP_BEACON_INTERVAL	100 /* max allowed associated AP beacon for DTIM skip */
#ifndef MAX_DTIM_ALLOWED_INTERVAL
#define MAX_DTIM_ALLOWED_INTERVAL 600 /* max allowed total beacon interval for DTIM skip */
#endif
#define NO_DTIM_SKIP 1
#ifdef SDTEST
/* Echo packet generator (SDIO), pkts/s */
extern uint dhd_pktgen;

/* Echo packet len (0 => sawtooth, max 1800) */
extern uint dhd_pktgen_len;
#define MAX_PKTGEN_LEN 1800
#endif


/* optionally set by a module_param_string() */
#define MOD_PARAM_PATHLEN	2048
#define MOD_PARAM_INFOLEN	512

#ifdef SOFTAP
extern char fw_path2[MOD_PARAM_PATHLEN];
#endif

/* Flag to indicate if we should download firmware on driver load */
extern uint dhd_download_fw_on_driverload;
#ifndef BCMDBUS
extern int allow_delay_fwdl;
#endif /* !BCMDBUS */


extern void dhd_wait_for_event(dhd_pub_t *dhd, bool *lockvar);
extern void dhd_wait_event_wakeup(dhd_pub_t*dhd);

#define IFLOCK_INIT(lock)       *lock = 0
#define IFLOCK(lock)    while (InterlockedCompareExchange((lock), 1, 0))	\
	NdisStallExecution(1);
#define IFUNLOCK(lock)  InterlockedExchange((lock), 0)
#define IFLOCK_FREE(lock)
#define FW_SUPPORTED(dhd, capa) ((strstr(dhd->fw_capabilities, " " #capa " ") != NULL))
#ifdef ARP_OFFLOAD_SUPPORT
#define MAX_IPV4_ENTRIES	8
void dhd_arp_offload_set(dhd_pub_t * dhd, int arp_mode);
void dhd_arp_offload_enable(dhd_pub_t * dhd, int arp_enable);

/* dhd_commn arp offload wrapers */
void dhd_aoe_hostip_clr(dhd_pub_t *dhd, int idx);
void dhd_aoe_arp_clr(dhd_pub_t *dhd, int idx);
int dhd_arp_get_arp_hostip_table(dhd_pub_t *dhd, void *buf, int buflen, int idx);
void dhd_arp_offload_add_ip(dhd_pub_t *dhd, uint32 ipaddr, int idx);
#endif /* ARP_OFFLOAD_SUPPORT */
#ifdef WLTDLS
int dhd_tdls_enable(struct net_device *dev, bool tdls_on, bool auto_on, struct ether_addr *mac);
int dhd_tdls_set_mode(dhd_pub_t *dhd, bool wfd_mode);
#ifdef PCIE_FULL_DONGLE
void dhd_tdls_update_peer_info(struct net_device *dev, bool connect_disconnect, uint8 *addr);
#endif /* PCIE_FULL_DONGLE */
#endif /* WLTDLS */
/* Neighbor Discovery Offload Support */
int dhd_ndo_enable(dhd_pub_t * dhd, int ndo_enable);
int dhd_ndo_add_ip(dhd_pub_t *dhd, char* ipaddr, int idx);
int dhd_ndo_remove_ip(dhd_pub_t *dhd, int idx);
/* ioctl processing for nl80211 */
int dhd_ioctl_process(dhd_pub_t *pub, int ifidx, struct dhd_ioctl *ioc, void *data_buf);

void dhd_bus_update_fw_nv_path(struct dhd_bus *bus, char *pfw_path, char *pnv_path);
void dhd_set_bus_state(void *bus, uint32 state);

/* Remove proper pkts(either one no-frag pkt or whole fragmented pkts) */
typedef int (*f_droppkt_t)(dhd_pub_t *dhdp, int prec, void* p, bool bPktInQ);
extern bool dhd_prec_drop_pkts(dhd_pub_t *dhdp, struct pktq *pq, int prec, f_droppkt_t fn);

#ifdef PROP_TXSTATUS
int dhd_os_wlfc_block(dhd_pub_t *pub);
int dhd_os_wlfc_unblock(dhd_pub_t *pub);
extern const uint8 prio2fifo[];
#endif /* PROP_TXSTATUS */

void dhd_save_fwdump(dhd_pub_t *dhd_pub, void * buffer, uint32 length);
void dhd_schedule_memdump(dhd_pub_t *dhdp, uint8 *buf, uint32 size);
int dhd_os_socram_dump(struct net_device *dev, uint32 *dump_size);
int dhd_os_get_socram_dump(struct net_device *dev, char **buf, uint32 *size);
int dhd_common_socram_dump(dhd_pub_t *dhdp);
int dhd_os_get_version(struct net_device *dev, bool dhd_ver, char **buf, uint32 size);

uint8* dhd_os_prealloc(dhd_pub_t *dhdpub, int section, uint size, bool kmalloc_if_fail);
void dhd_os_prefree(dhd_pub_t *dhdpub, void *addr, uint size);

int dhd_process_cid_mac(dhd_pub_t *dhdp, bool prepost);

#if defined(CONFIG_DHD_USE_STATIC_BUF)
#define DHD_OS_PREALLOC(dhdpub, section, size) dhd_os_prealloc(dhdpub, section, size, FALSE)
#define DHD_OS_PREFREE(dhdpub, addr, size) dhd_os_prefree(dhdpub, addr, size)
#else
#define DHD_OS_PREALLOC(dhdpub, section, size) MALLOC(dhdpub->osh, size)
#define DHD_OS_PREFREE(dhdpub, addr, size) MFREE(dhdpub->osh, addr, size)
#endif /* defined(CONFIG_DHD_USE_STATIC_BUF) */


#define dhd_add_flowid(pub, ifidx, ac_prio, ea, flowid)  do {} while (0)
#define dhd_del_flowid(pub, ifidx, flowid)               do {} while (0)
bool dhd_wet_chainable(dhd_pub_t *dhdp);

extern unsigned long dhd_os_general_spin_lock(dhd_pub_t *pub);
extern void dhd_os_general_spin_unlock(dhd_pub_t *pub, unsigned long flags);

/** Miscellaenous DHD Spin Locks */

/* Disable router 3GMAC bypass path perimeter lock */
#define DHD_PERIM_LOCK(dhdp)              do {} while (0)
#define DHD_PERIM_UNLOCK(dhdp)            do {} while (0)
#define DHD_PERIM_LOCK_ALL(processor_id)    do {} while (0)
#define DHD_PERIM_UNLOCK_ALL(processor_id)  do {} while (0)

/* Enable DHD general spin lock/unlock */
#define DHD_GENERAL_LOCK(dhdp, flags) \
	(flags) = dhd_os_general_spin_lock(dhdp)
#define DHD_GENERAL_UNLOCK(dhdp, flags) \
	dhd_os_general_spin_unlock((dhdp), (flags))

/* Enable DHD flowring spin lock/unlock */
#define DHD_FLOWRING_LOCK(lock, flags)     (flags) = dhd_os_spin_lock(lock)
#define DHD_FLOWRING_UNLOCK(lock, flags)   dhd_os_spin_unlock((lock), (flags))

/* Enable DHD common flowring info spin lock/unlock */
#define DHD_FLOWID_LOCK(lock, flags)       (flags) = dhd_os_spin_lock(lock)
#define DHD_FLOWID_UNLOCK(lock, flags)     dhd_os_spin_unlock((lock), (flags))

/* Enable DHD common flowring list spin lock/unlock */
#define DHD_FLOWRING_LIST_LOCK(lock, flags)       (flags) = dhd_os_spin_lock(lock)
#define DHD_FLOWRING_LIST_UNLOCK(lock, flags)     dhd_os_spin_unlock((lock), (flags))

extern void dhd_dump_to_kernelog(dhd_pub_t *dhdp);

#ifdef BCMDBUS
extern void dhd_bus_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf);
extern void dhd_bus_clearcounts(dhd_pub_t *dhdp);
#endif /* BCMDBUS */

#ifdef DHD_L2_FILTER
extern int dhd_get_parp_status(dhd_pub_t *dhdp, uint32 idx);
extern int dhd_set_parp_status(dhd_pub_t *dhdp, uint32 idx, int val);
extern int dhd_get_dhcp_unicast_status(dhd_pub_t *dhdp, uint32 idx);
extern int dhd_set_dhcp_unicast_status(dhd_pub_t *dhdp, uint32 idx, int val);
extern int dhd_get_block_ping_status(dhd_pub_t *dhdp, uint32 idx);
extern int dhd_set_block_ping_status(dhd_pub_t *dhdp, uint32 idx, int val);
extern int dhd_get_grat_arp_status(dhd_pub_t *dhdp, uint32 idx);
extern int dhd_set_grat_arp_status(dhd_pub_t *dhdp, uint32 idx, int val);
#endif /* DHD_L2_FILTER */


typedef struct wl_io_pport {
	dhd_pub_t *dhd_pub;
	uint ifidx;
} wl_io_pport_t;

typedef struct wl_evt_pport {
	dhd_pub_t *dhd_pub;
	int *ifidx;
	void *pktdata;
	void **data_ptr;
	void *raw_event;
} wl_evt_pport_t;

extern void *dhd_pub_shim(dhd_pub_t *dhd_pub);
#ifdef DHD_FW_COREDUMP
void* dhd_get_fwdump_buf(dhd_pub_t *dhd_pub, uint32 length);
#endif /* DHD_FW_COREDUMP */

#if defined(SET_RPS_CPUS)
int dhd_rps_cpus_enable(struct net_device *net, int enable);
int custom_rps_map_set(struct netdev_rx_queue *queue, char *buf, size_t len);
void custom_rps_map_clear(struct netdev_rx_queue *queue);
#define PRIMARY_INF 0
#define VIRTUAL_INF 1
#if defined(CONFIG_MACH_UNIVERSAL5433) || defined(CONFIG_MACH_UNIVERSAL7420) || \
	defined(CONFIG_SOC_EXYNOS8890)
#define RPS_CPUS_MASK "10"
#define RPS_CPUS_MASK_P2P "10"
#define RPS_CPUS_MASK_IBSS "10"
#define RPS_CPUS_WLAN_CORE_ID 4
#else
#define RPS_CPUS_MASK "6"
#define RPS_CPUS_MASK_P2P "6"
#define RPS_CPUS_MASK_IBSS "6"
#endif /* CONFIG_MACH_UNIVERSAL5433 || CONFIG_MACH_UNIVERSAL7420 || CONFIG_SOC_EXYNOS8890 */
#endif 

int dhd_get_download_buffer(dhd_pub_t	*dhd, char *file_path, download_type_t component,
	char ** buffer, int *length);

void dhd_free_download_buffer(dhd_pub_t	*dhd, void *buffer, int length);

/* blob support */
int dhd_download_clm_blob(dhd_pub_t *dhd, unsigned char *buf, uint32 len);

int dhd_apply_default_clm(dhd_pub_t *dhd, char *clm_path);

#define dhd_is_device_removed(x) FALSE
#define dhd_os_ind_firmware_stall(x)

#if defined(DHD_FW_COREDUMP)
extern void dhd_get_memdump_info(dhd_pub_t *dhd);
#endif 


#if defined(DHD_LB_STATS)
#include <bcmutils.h>
extern void dhd_lb_stats_init(dhd_pub_t *dhd);
extern void dhd_lb_stats_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf);
extern void dhd_lb_stats_update_napi_histo(dhd_pub_t *dhdp, uint32 count);
extern void dhd_lb_stats_update_txc_histo(dhd_pub_t *dhdp, uint32 count);
extern void dhd_lb_stats_update_rxc_histo(dhd_pub_t *dhdp, uint32 count);
extern void dhd_lb_stats_txc_percpu_cnt_incr(dhd_pub_t *dhdp);
extern void dhd_lb_stats_rxc_percpu_cnt_incr(dhd_pub_t *dhdp);
#define DHD_LB_STATS_INIT(dhdp) dhd_lb_stats_init(dhdp)
/* Reset is called from common layer so it takes dhd_pub_t as argument */
#define DHD_LB_STATS_RESET(dhdp) dhd_lb_stats_init(dhdp)
#define DHD_LB_STATS_CLR(x)     (x) = 0U
#define DHD_LB_STATS_INCR(x)    (x) = (x) + 1
#define DHD_LB_STATS_ADD(x, c)  (x) = (x) + (c)
#define DHD_LB_STATS_PERCPU_ARR_INCR(x) \
	{ \
		int cpu = get_cpu(); put_cpu(); \
		DHD_LB_STATS_INCR(x[cpu]); \
	}
#define DHD_LB_STATS_UPDATE_NAPI_HISTO(dhdp, x) dhd_lb_stats_update_napi_histo(dhdp, x)
#define DHD_LB_STATS_UPDATE_TXC_HISTO(dhdp, x)  dhd_lb_stats_update_txc_histo(dhdp, x)
#define DHD_LB_STATS_UPDATE_RXC_HISTO(dhdp, x)  dhd_lb_stats_update_rxc_histo(dhdp, x)
#define DHD_LB_STATS_TXC_PERCPU_CNT_INCR(dhdp)  dhd_lb_stats_txc_percpu_cnt_incr(dhdp)
#define DHD_LB_STATS_RXC_PERCPU_CNT_INCR(dhdp)  dhd_lb_stats_rxc_percpu_cnt_incr(dhdp)
#else /* !DHD_LB_STATS */
#define DHD_LB_STATS_NOOP       do { /* noop */ } while (0)
#define DHD_LB_STATS_INIT(dhdp)  DHD_LB_STATS_NOOP
#define DHD_LB_STATS_RESET(dhdp) DHD_LB_STATS_NOOP
#define DHD_LB_STATS_CLR(x)      DHD_LB_STATS_NOOP
#define DHD_LB_STATS_INCR(x)     DHD_LB_STATS_NOOP
#define DHD_LB_STATS_ADD(x, c)   DHD_LB_STATS_NOOP
#define DHD_LB_STATS_PERCPU_ARR_INCR(x)  DHD_LB_STATS_NOOP
#define DHD_LB_STATS_UPDATE_NAPI_HISTO(dhd, x) DHD_LB_STATS_NOOP
#define DHD_LB_STATS_UPDATE_TXC_HISTO(dhd, x) DHD_LB_STATS_NOOP
#define DHD_LB_STATS_UPDATE_RXC_HISTO(dhd, x) DHD_LB_STATS_NOOP
#define DHD_LB_STATS_TXC_PERCPU_CNT_INCR(dhdp) DHD_LB_STATS_NOOP
#define DHD_LB_STATS_RXC_PERCPU_CNT_INCR(dhdp) DHD_LB_STATS_NOOP
#endif /* !DHD_LB_STATS */

extern int dhd_prot_debug_info_print(dhd_pub_t *dhd);

#define htod32(i) (i)
#define htod16(i) (i)
#define dtoh32(i) (i)
#define dtoh16(i) (i)
#define htodchanspec(i) (i)
#define dtohchanspec(i) (i)

#endif /* _dhd_h_ */
