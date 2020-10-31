/*****************************************************************************/
/*                                                                           */
/* PROJECT : Rainbow	                                                     */
/* MODULE  : Whimory configuration definition heade file                     */
/* NAME    : Whimory configuration definition                                */
/* FILE    : WMRConfig.h                                                     */
/* PURPOSE : Configuation definition for Whimory                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*        COPYRIGHT 2003-2005, SAMSUNG ELECTRONICS CO., LTD.                 */
/*                      ALL RIGHTS RESERVED                                  */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing a product based on Samsung                 */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representation or warranties     */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/*   12-JUL-2005 [Jaesung Jung] : first writing                              */
/*   03-NOV-2005 [Yangsup Lee ] : Add wear-leveling algorithm				 */
/*                                                                           */
/*****************************************************************************/

#ifndef _WMR_CONFIG_H_
#define _WMR_CONFIG_H_

#include "WMRFeatures.h"

#if (defined (AND_SUPPORT_2ND_BOOTLOADER) && AND_SUPPORT_2ND_BOOTLOADER && defined (AND_SIGNATURE_IN_BLOCK_ZERO) && AND_SIGNATURE_IN_BLOCK_ZERO)
#error "AND_SUPPORT_2ND_BOOTLOADER & AND_SIGNATURE_IN_BLOCK_ZERO can't both be defined"
#endif

/*****************************************************************************/
/* Global Config Definition which should be shared by FTL, VFL, FIL          */
/*****************************************************************************/
#define     WMR_BOOT_AREA_DEFAULT_SIZE (1)  /* this will be value used unless unit does boot-from-nand */
#define     AND_MAX_BANKS_PER_CS    (8)
#define     WMR_SECTOR_SIZE         (512)   /* the size of sector				 */
#if defined(AND_SUPPORT_YAFTL) && AND_SUPPORT_YAFTL
#define     WMR_NUM_BUFFERS         (10)    /* the number of buffers - used by VFLBuffer */
#else /* defined(AND_SUPPORT_YAFTL) && AND_SUPPORT_YAFTL */
#if !(defined(AND_READONLY) && AND_READONLY)
#define     WMR_NUM_BUFFERS         (5)     /* the number of buffers - used by VFLBuffer */
#elif !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY) || \
       (defined(AND_COLLECT_STATISTICS) && AND_COLLECT_STATISTICS)
#define     WMR_NUM_BUFFERS         (2)     /* the number of buffers - used by VFLBuffer */
#else
#define     WMR_NUM_BUFFERS         (1)     /* Used in _FTLRead and VFL_Open */
#endif
#endif /* defined(AND_SUPPORT_YAFTL) && AND_SUPPORT_YAFTL */

#define     WMR_SECTORS_PER_PAGE_MIN (4)

#define     WMR_MAX_PAGES_PER_BLOCK     (0x100)

#define     WMR_MAX_VB          (18000) /* the maximum number of virtual block*/

#define     WMR_MAX_RESERVED_SIZE   (820)   /* the maximum count of reserved blocks */

#define     FTL_METAWEARLEVEL_RATIO         (10) // consider bringing this number up
#define     WMR_WEAR_LEVEL_FREQUENCY        (20)
#define     WMR_WEAR_LEVEL_FREQUENCY_DELAY  (300)
#define     WMR_WEAR_LEVEL_MAX_DIFF         (5)
#define     WMR_WEAR_LEVEL_MIN_EC_VALUE     (3)

#define DEFAULT_NUMBER_OF_SPECIAL_BLOCKS_CS0 (3)
#define NUMBER_OF_SPECIAL_BLOCKS_NOTCS0      (2)

#define FTL_READ_REFRESH_LIST_SIZE          (5)
#define FTL_RC_TRESHOLD                     (10000) // to review for real world drivers
#define FTL_READS_SINCE_LAST_REFRESH_MARK_TRASHOLD (200)

#define YAFTL_RC_THRESHOLD                  (0xfffd)
#define YAFTL_RC_MULTIPLIER                 (16)

#define S_RC_THRESHOLD                      (250000)

#define BYTES_PER_METADATA_RAW              (12)

#define VSVFL_USER_BLKS_PER_1K              (976)
#define PPNVFL_USER_BLKS_PER_256B           (244)
#define VFL_USER_BLKS_PER_1K                (968)
#define FTL_CXT_SECTION_SIZE                (3) // number of FTL Cxt locations are saved by VFL
#define WMR_BLOCK_ADDRESS_MSB               (1<<15)

#endif /* _WMR_CONFIG_H_ */
