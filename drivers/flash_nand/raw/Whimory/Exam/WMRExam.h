/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : Example			                                         */
/* NAME    	   : Example					                                 */
/* FILE        : WMRExam.h		                                             */
/* PURPOSE 	   : the example for whimory initialization						 */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2005 SAMSUNG ELECTRONICS CO., LTD.                */
/*                          ALL RIGHTS RESERVED                              */
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
/*   12-SEP-2005 [Jaesung Jung] : first writing                              */
/*                                                                           */
/*****************************************************************************/
#if (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)
#include "FTL.h"
#include "VFL.h"
#include "FIL.h"
#else
#include "FPart.h"
#include "FIL.h"
#endif

#ifndef _WMR_EXAM_H_
#define _WMR_EXAM_H_

/*****************************************************************************/
/* whimory version signature							*/
// Platform Types
#define    WMR_PLATFORM_SHIFT  (24)
#define    WMR_PLATFORM_DARWIN ('C')

// Layout Types
#define    WMR_LAYOUT_SHIFT  (16)
#define    WMR_LAYOUT_SIMPLE ('0')
#define    WMR_LAYOUT_VS     ('1')

// VFL types
#define     WMR_VFL_SHIFT    (8)
#define     WMR_M68_VFL             ('0')
#define     WMR_VSVFL               ('1')

// default signature goes here
#define     WMR_VFL_SIGNATURE           ((WMR_PLATFORM_DARWIN << WMR_PLATFORM_SHIFT) | (WMR_LAYOUT_SIMPLE << WMR_LAYOUT_SHIFT) | (WMR_M68_VFL << 8)) /* 'C0' for P2 */
#define     WMR_VSVFL_SIGNATURE         ((WMR_PLATFORM_DARWIN << WMR_PLATFORM_SHIFT) | (WMR_LAYOUT_SIMPLE << WMR_LAYOUT_SHIFT) | (WMR_VSVFL << 8)) /* 'C0' for P2 */
#define     WMR_VSVFL_VS_SIGNATURE      ((WMR_PLATFORM_DARWIN << WMR_PLATFORM_SHIFT) | (WMR_LAYOUT_VS << WMR_LAYOUT_SHIFT)     | (WMR_VSVFL << 8)) /* 'C1' for N72+ */

#define     WMR_SIGNATURE_EPOCH_MASK    (0x000000FF)
#define     WMR_SIGNATURE_VFL_MASK      (0x0000FF00)
#define     WMR_SIGNATURE_LAYOUT_MASK   (0x00FF0000)
#define     WMR_SIGNATURE_PLATFORM_MASK (0xFF000000)
// signature bit masks for 2nd ID
#define		WMR_SIGNATURE_FORMAT_VERSION_MASK	(0x0000FFFF)
#define		WMR_SIGNATURE_METADATA_MASK			(0x00010000)

#define     WMR_SIGNATURE_VFL(_x)           (UInt8)(((_x) & WMR_SIGNATURE_VFL_MASK) >> WMR_VFL_SHIFT)
#define     WMR_SIGNATURE_LAYOUT(_x)        (UInt8)(((_x) & WMR_SIGNATURE_LAYOUT_MASK) >> WMR_LAYOUT_SHIFT)
#define     WMR_SIGNATURE_PLATFORM(_x)      (UInt8)(((_x) & WMR_SIGNATURE_PLATFORM_MASK) >> WMR_PLATFORM_SHIFT)
#define     WMR_SIGNATURE_FORMATTING_VERSION(_x) (UInt8)((_x) & WMR_SIGNATURE_FORMAT_VERSION_MASK)
#define     WMR_SIGNATURE_METADATA(_x)      (UInt8)(((_x) & WMR_SIGNATURE_METADATA_MASK) >> WMR_METADATA_SHIFT)

#define     WMR_FORMATTING_VERSION_2    ((UInt32)0x00000002) /* May 20th 2007 */
#define     WMR_FORMATTING_VERSION_3  ((UInt32)0x00000003) /* August 22nd 2007 */
#define     WMR_FORMATTING_VERSION_4  ((UInt32)0x00000004) /* July 23rd 2008 */
#define     WMR_FORMATTING_VERSION_5  ((UInt32)0x00000005) /* Dec 1st 2009 */
#define     WMR_FORMATTING_VERSION  ((UInt32)0x00000006) /* July 30th 2010 */

// Metadata Whitening
#define     WMR_METADATA_SHIFT                  (16)
#define     WMR_METADATA_WHITENING_NONE         ((UInt32)0x0)
#define     WMR_METADATA_WHITENING              ((UInt32)0x1)
#define     WMR_METADATA_SIGNATURE              (WMR_METADATA_WHITENING << WMR_METADATA_SHIFT)

#define     WMR_FORMAT_SUPPORTED(_x)        ((_x) <= WMR_FORMATTING_VERSION)

/* used during epoch updates */
#define     WMR_TEMP_SIGNATURE      (('X' << 24) | ('X' << 16) | ('X' << 8) | ('X' << 0))
/* Return value of WMR_XXX()                                                 */
/*****************************************************************************/
#define     WMR_SUCCESS                     ANDErrorCodeOk
#define     WMR_CRITICAL_ERROR              ANDErrorCodeHwErr
#define     WMR_UNRECOVERABLE_ERROR         ANDErrorCodeUserDataErr

#define     WMR_MAX_STR_BUILD_SIZE          (256)

typedef struct
{
    UInt32 dwMainSignature;                             // main driver version number
    UInt32 dw2ndId;                                     // to be used for undetifying sub versions
    UInt8 strBuildInfo[WMR_MAX_STR_BUILD_SIZE];        // kernel / build version that formatted the device
} ANDDriverSignature;

/*****************************************************************************/
/* exported function prototype of WMR example                                */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Int32    WMR_PreInit(UInt32 dwFormatType, UInt32 dwBootAreaSize);

#if (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)
Int32    WMR_Init(UInt32 *pTotalScts, UInt32 * pdwSectorSize, UInt32 * tmpIndexCache, UInt32 dwFormatType, UInt32 dwBootAreaSize);
void     WMR_Close(void);
BOOL32   WMR_CtrlIO(UInt32 dwCtlrIOType, void * pvoidDataBuffer, UInt32 * pdwDataSize);
BOOL32   WMRGetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);

FTLFunctions * WMR_GetFTL(void);
VFLFunctions * WMR_GetVFL(void);
#endif // (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)

FPartFunctions * WMR_GetFPart(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FTL_H_ */
