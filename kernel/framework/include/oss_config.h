/* oss_config.h
 *
 * Purpose: The top level header file for compiling OSS itself.
 *
 * This is the mother of all headers in OSS. All core files should include this
 * file.
 *
 * However low level drivers must never include this file. Instead they should
 * use the autogenerated drv_cfg.h file.
 */
#define COPYING16 Copyright (C) Hannu Savolainen and Dev Mazumdar 1996-2007. All rights reserved.

#ifndef OSS_CONFIG_H
#define OSS_CONFIG_H

#define NEW_DEVICE_NAMING	/* /dev/sblive0_dsp0 instead of /dev/dsp0 and so on */

#include "soundcard.h"

struct _oss_device_t;
typedef int *ioctl_arg;

/* Check various wrappers when changing these two function pointer defs */
typedef int (*oss_tophalf_handler_t) (struct _oss_device_t * osdev);
typedef void (*oss_bottomhalf_handler_t) (struct _oss_device_t * osdev);

#include <ossddk/oss_exports.h>
#include <os.h>

#include <ossddk/ossddk.h>

#ifndef OS_VERSION
#error OS_VERSION not defined in os.h
#endif

/*
 * Memory block allocation (oss_memblk.h/c)
 */
#include <oss_memblk.h>

/*
 * Memory allocation/free routines for structures to be freed automatically
 * when OSS is unloaded.
 */
#define PMALLOC(osdev, size) oss_memblk_malloc(&oss_global_memblk, size)
#define PMFREE(osdev, addr) oss_memblk_free(&oss_global_memblk, addr)

/*
 * Currently /dev/dsp is managed in user land by ossdevlinks instead of
 * using the previous kernel level device lists feature (earlier maintained by
 * the ossctl utility). Define MANAGE_DEV_DSP to return back this functionality
 * in the driver. Also remove the related code in the user land tools
 * (ossdevlinks).
 */
#undef  MANAGE_DEV_DSP

#ifndef ERESTARTSYS
#define ERESTARTSYS EAGAIN
#endif

#ifndef OSS_HZ
#define OSS_HZ HZ
#endif

#ifndef OSS_PAGE_MASK
#define OSS_PAGE_MASK (-1)
#endif

#ifndef HW_PRINTF
#define HW_PRINTF(x)
#endif

#ifndef GET_PROCESS_NAME
#define GET_PROCESS_NAME() NULL
#endif

#ifndef GET_PROCESS_PID
#define GET_PROCESS_PID() -1
#endif

#ifndef FMA_EREPORT
/* FMA is only available under Solaris */
#define FMA_EREPORT(osdev, detail, name, type, value)
#define FMA_IMPACT(osdev, impact)
#endif

#include "oss_version.h"
/* #include "aw.h" */
#define EXCLUDE_GUS_IODETECT

#ifndef IOC_IS_OUTPUT
#define IOC_IS_OUTPUT(cmd) cmd & SIOC_IN
#endif

#define DSP_DEFAULT_SPEED	48000
#define DSP_DEFAULT_FMT		AFMT_S16_LE
#define AUDIO_DEFAULT_SPEED	8000

#define ON		1
#define OFF		0

#define SYNTH_MAX_VOICES	64

struct voice_alloc_info
{
  int max_voice;
  int used_voices;
  int ptr;			/* For device specific use */
  unsigned short map[SYNTH_MAX_VOICES];	/* (ch << 8) | (note+1) */
  int timestamp;
  int alloc_times[SYNTH_MAX_VOICES];
};

struct channel_info
{
  int pgm_num;
  int bender_value;
  int bender_range;
  unsigned char controllers[128];
};


typedef struct oss_wait_queue oss_wait_queue_t;

#include "audio_core.h"
#include "mixer_core.h"
#include "oss_calls.h"
#include "internal.h"

#ifndef DEB
#define DEB(x)
#endif

#ifndef DDB
#define DDB(x)  {}
#endif

#ifndef CDB
#define CDB(x)  {}
#endif

#define TIMER_ARMED	121234
#define TIMER_NOT_ARMED	1

#ifdef DO_MEMDEBUG
extern void oss_vmem_check (char *file, int line, oss_native_word offs,
			    int bytes);
extern void oss_pmem_check (char *file, int line, oss_native_word offs,
			    int bytes);
# define VMEM_CHECK(offs, bytes) oss_vmem_check(__FILE__, __LINE__, (oss_native_word)offs, bytes)
# define PMEM_CHECK(offs, bytes) oss_pmem_check(__FILE__, __LINE__, (oss_native_word)offs, bytes)
#else
# define VMEM_CHECK(offs, bytes)
# define PMEM_CHECK(offs, bytes)
#endif

#ifndef OSS_BIG_ENDIAN
#define USE_REMUX
#endif

typedef struct
{
  char *const key, *const name;
} oss_device_table_t;


typedef struct
{
#ifdef _KERNEL
  int (*open) (int dev, int dev_class, struct fileinfo * file,
	       int recursive, int open_flags, int *redirect);
  void (*close) (int dev, struct fileinfo * file);
  int (*read) (int dev, struct fileinfo * file, uio_t * buf, int count);
  int (*write) (int dev, struct fileinfo * file, uio_t * buf, int count);
  int (*ioctl) (int dev, struct fileinfo * file,
		unsigned int cmd, ioctl_arg arg);
  int (*chpoll) (int dev, struct fileinfo * file, oss_poll_event_t * ev);
#else
  int dummy;
#endif
} oss_cdev_drv_t;

typedef struct
{
  int dev_class;
  int instance;
  oss_cdev_drv_t *d;
  char name[32];
  oss_device_t *osdev;
  void *info;
  int active;
  int open_count;
} oss_cdev_t;

extern oss_cdev_t *oss_cdevs[OSS_MAX_CDEVS];
extern int oss_num_cdevs;

/*
 * Global initialization and cleanup functions (common to all operating systems)
 */
extern void oss_common_init (oss_device_t * osdev);
extern void oss_unload_drivers (void);
/*
 * The following calls are operating system dependent. They should be defined
 * in os.c for each operating system.
 */
extern int oss_register_device (oss_device_t * osdev, const char *name);
extern void oss_unregister_device (oss_device_t * osdev);
extern int oss_find_minor (int dev_class, int instance);
extern int oss_disable_device (oss_device_t * osdev);
extern void oss_install_chrdev (oss_device_t * osdev, char *name,
				int dev_class, int instance,
				oss_cdev_drv_t * drv, int flags);
#define CHDEV_VIRTUAL	0x00000001
#define CHDEV_REPLUG	0x00000002 /* Hot (un)pluggable device got replugged */
extern int oss_register_interrupts (oss_device_t * osdev, int intrnum,
				    oss_tophalf_handler_t top,
				    oss_bottomhalf_handler_t bottom);
extern void oss_unregister_interrupts (oss_device_t * osdev);

/*
 * DMA buffer management
 */
extern int oss_alloc_dmabuf (int dev, dmap_p dmap, int direction);
extern int __oss_alloc_dmabuf (int dev, dmap_p dmap, unsigned int alloc_flags,
			       oss_uint64_t maxaddr, int direction);
/* Alloc flags */
#define DMABUF_NONCONTIG	0x00000001
#define DMABUF_ISA		0x00000002
#define DMABUF_QUIET		0x00000004
#define DMABUF_SIZE_16BITS	0x00000008	/* Max buffer size 64k-1 */
#define DMABUF_LARGE		0x00000010	/* Allocate a 356k buffer */
/* Maxaddr defs */
#define MEMLIMIT_ISA		0x0000000000ffffffLL
#define MEMLIMIT_28BITS		0x000000000fffffffLL
#define MEMLIMIT_30BITS		0x000000003fffffffLL
#define MEMLIMIT_31BITS		0x000000007fffffffLL
#define MEMLIMIT_32BITS		0x00000000ffffffffLL
#define MEMLIMIT_64BITS		0xffffffffffffffffLL
extern void oss_free_dmabuf (int dev, dmap_p dmap);

/*
 * Device management routines
 */
extern void *oss_get_osid (oss_device_t * osdev);
extern void oss_reserve_device (oss_device_t * osdev);
extern void oss_unreserve_device (oss_device_t * osdev, int decrement);
/*
 * Sleep/wakeup support (os.c)
 */
extern oss_wait_queue_t *oss_create_wait_queue (oss_device_t * osdev,
						const char *name);
extern void oss_reset_wait_queue (oss_wait_queue_t * wq);
extern void oss_remove_wait_queue (oss_wait_queue_t * wq);
extern int oss_sleep (oss_wait_queue_t * wq, oss_mutex_t * mutex, int ticks,
		      oss_native_word * flags, unsigned int *status);
extern int oss_register_poll (oss_wait_queue_t * wq, oss_mutex_t * mutex,
			      oss_native_word * flags, oss_poll_event_t * ev);
extern void oss_wakeup (oss_wait_queue_t * wq, oss_mutex_t * mutex,
			oss_native_word * flags, short events);
/*
 * Wait status flags (make sure these match osscore.c for Linux)
 */
#define WK_NONE		0x00
#define WK_WAKEUP	0x01
#define WK_TIMEOUT	0x02
#define WK_SIGNAL	0x04
#define WK_SLEEP	0x08
#define WK_SELECT	0x10

/*
 * Mutex hierarchy levels
 * Used by LOCK_ALLOC in UnixWare/OpenServer
 */
#define MH_GLOBAL	1
#define MH_FRAMEW	5
#define MH_DRV		10
#define MH_TOP		20

#define OSS_HISTORY_SIZE 5
#define OSS_HISTORY_STRSIZE 256
typedef char oss_history_t[OSS_HISTORY_STRSIZE];
extern oss_history_t oss_history[OSS_HISTORY_SIZE];
extern int oss_history_p;

#ifdef LICENSED_VERSION
#include <license.h>
#endif

extern void create_new_card (char *shortname, char *longname);

/*
 * Config option support.
 */
typedef struct
{
  char *name;
  int *ptr;
} oss_option_map_t;

extern void oss_load_options (oss_device_t * osdev, oss_option_map_t map[]);
extern oss_option_map_t oss_global_options[];

extern int detect_trace;

extern int oss_num_cards;
extern int oss_get_cardinfo (int cardnum, oss_card_info * ci);
extern oss_device_t *osdev_create (dev_info_t * dip, int dev_type,
				   int instance, const char *nick,
				   const char *handle);
extern oss_device_t *osdev_clone (oss_device_t * orig_osdev,
				  int new_instance);
extern void osdev_delete (oss_device_t * osdev);

/*
 * oss_pci_byteswap() turns on/off hardware level byte swapping.
 * mode=0 : Don't do byte swapping in the bus interface (default)
 * mode=1 : Do automatic byte swapping (BE machines) on the bus interface.
 */
extern void oss_pci_byteswap (oss_device_t * osdev, int mode);

/*
 * Error reporting
 */
#define E_REC	1
#define E_PLAY	2
extern void oss_audio_set_error (int dev, int mode, int err, int parm);
#define OSSERR(cntx, msg) cntx

/*
 * Ensure that various obsolete OSS 3.x features are not used any more
 * in the drivers.
 */
#define osp osp_is_obsolete[]
#define CREATE_OSP(osdev) CREATE_OSP_is_obsolete
#define printk() 	printk_is_obsolete
#define REQUEST_REGION(start, len, name) REQUEST_REGION_is_obsolete
#define CHECK_REGION(start, len) 	CHECK_REGION_is_obsolete
#define RELEASE_REGION(start, len)	RELEASE_REGION_is_obsolete
#define ALLOCATE_DMA_CHN	ALLOCATE_DMA_CHN_is_obsolete
#define FREE_DMA_CHN		FREE_DMA_CHN_is_obsolete
#define OPEN_DMA_CHN		OPEN_DMA_CHN_is_obsolete
#define CLOSE_DMA_CHN		OPEN_DMA_CHN_is_obsolete
#if !defined(__bsdi__)
#define tenmicrosec(x) tenmicrosec_is_obsolete()
#endif
#define request_sound_timer(x) request_sound_timer_is_obsolete
#define sound_stop_timer(x) sound_stop_timer_is_obsolete
#define snd_set_irq_handler(x) snd_set_irq_handler_is_obsolete
#define snd_release_irq (vect, x) snd_release_irq_is_obsolete
#define conf_printf (name, hw_config) conf_printf_is_obsolete
#define conf_printf2 (name, base, irq, dma, dma2) conf_printf2_is_obsolete
#define sound_disable_module (void) sound_disable_module_is_obsolete
#define IOCTL_GET IOCTL_GET_is_obsolete
#define IOCTL_OUT IOCTL_OUT_is_obsolete
#define CONTIG_MALLOC_ISA CONTIG_MALLOC_ISA_is_obsolete()

#endif
