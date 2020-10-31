/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : Virtual Flash Layer                                         */
/* NAME    	   : VFL header file			                                 */
/* FILE        : VFL.h		                                                 */
/* PURPOSE 	   : This file contains the definition and protypes of exported  */
/*           	 functions for VFL. 				                         */
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
/*   18-JUL-2005 [Jaesung Jung] : first writing                              */
/*   31-MAR-2006 [Yangsup Lee ] : support ftl meta block wear leveling       */
/*                                                                           */
/*****************************************************************************/

#ifndef _VFL_H_
#define _VFL_H_
#include "FPart.h"
#include "VFLBuffer.h"

/*****************************************************************************/
/* Return value of VFL_XXX()                                                 */
/*****************************************************************************/
#define     VFL_SUCCESS                             ANDErrorCodeOk
#define     VFL_SUCCESS_CLEAN                       ANDErrorCodeCleanOk
#define     VFL_CRITICAL_ERROR                      ANDErrorCodeHwErr
#define     VFL_U_ECC_ERROR                         ANDErrorCodeUserDataErr

/*****************************************************************************/
/* Type definition of checksum (confirm) state                               */
/*****************************************************************************/

/*****************************************************************************/
/* exported function prototype of VFL                                        */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    

#define VFL_SPANS_MAX		     (128)
    
// read result flags - 32 Bit
#define VFL_READ_STATUS_REFRESH      (1 << 0)
#define VFL_READ_STATUS_RETIRE       (1 << 1)
#define VFL_READ_STATUS_UECC         (1 << 2) // UECC = invalid data TRUE and clean FALSE
#define VFL_READ_STATUS_CLEAN        (1 << 3) // UECC = invalid data TRUE and clean TRUE
#define VFL_READ_STATUS_VALID_DATA   (1 << 4) // at least one of the pages had valid data
#define VFL_READ_STATUS_UNIDENTIFIED (1 << 5) // at least one status was something we do not expect
#define VFL_READ_STATUS_ERRS         (VFL_READ_STATUS_UECC | VFL_READ_STATUS_RETIRE | VFL_READ_STATUS_REFRESH)
#define VFL_READ_STATUS_ALL          (0xFFFFFFFF)

typedef UInt32 VFLReadStatusType;
    
typedef void (*VFL_ReadSpansCB_t)(UInt32 vba, VFLReadStatusType status, UInt8 *meta);
typedef struct
{
    BOOL32 bDisableWhitening;
    BOOL32 bMarkECCForScrub;
    UInt32 vba[VFL_SPANS_MAX];
    UInt32 count[VFL_SPANS_MAX];
    UInt8 *data[VFL_SPANS_MAX];
    UInt8 *meta[VFL_SPANS_MAX];
    UInt32 len;
    VFL_ReadSpansCB_t cb;
    UInt32 cbOpt;
    VFLReadStatusType op_status;
} VFL_ReadSpans_t;

typedef struct _VFLFunctions
{
#ifndef AND_READONLY
    Int32 (*Format)(UInt32 dwBootAreaSize, UInt32 dwOptions);
    BOOL32 (*AllocateSpecialBlock)(SpecialBlockAddress *allocated_block, UInt16 type);
    Int32 (*WriteSinglePage)(UInt32 nVpn, Buffer *pBuf, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening);
    Int32 (*Erase)(UInt16 wVbn, BOOL32 bReplaceOnFail);
    Int32 (*ChangeFTLCxtVbn)(UInt16 *aFTLCxtVbn);
    BOOL32 (*WriteMultiplePagesInVb)
    (UInt32 dwVpn, UInt16 wNumPagesToWrite,
     UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening);
    BOOL32 (*MarkBlockAsGrownBad)(SpecialBlockAddress *allocated_block);
	
#if AND_SUPPORT_BLOCK_BORROWING
    BOOL32 (*BorrowSpareBlock)(UInt32 *pdwPhysicalCE, UInt32 *pdwPhysicalBlock, UInt16 wType);
#endif //AND_SUPPORT_BLOCK_BORROWING

    BOOL32 (*ProgramMultipleVbas)(UInt32 vba, UInt16 count, UInt8 *data, UInt8 *meta, 
                                  BOOL32 replaceOnPfail, BOOL32 disableWhitening);
#endif // ! AND_READONLY

    Int32 (*Init)(FPartFunctions * pFPartFunctions);
    Int32 (*Open)(UInt32 dwBootAreaSize, UInt32 minor_ver, UInt32 dwOptions);
    void (*Close)(void);
    BOOL32 (*ReadMultiplePagesInVb)
    (UInt16 wVbn, UInt16 wStartPOffset,
     UInt16 wNumPagesToRead,
     UInt8 * pbaData,
     UInt8 * pbaSpare,
     BOOL32 * pboolNeedRefresh,
     UInt8 * pabSectorStats,
     BOOL32 bDisableWhitening);
    BOOL32 (*ReadScatteredPagesInVb)
    (UInt32 * padwVpn,
     UInt16 wNumPagesToRead,
     UInt8 * pbaData, UInt8 * pbaSpare,
     BOOL32 * pboolNeedRefresh,
     UInt8 * pabSectorStats,
     BOOL32 bDisableWhitening, Int32 *actualStatus);
    Int32 (*ReadSinglePage)(UInt32 nVpn, Buffer *pBuf,
                            BOOL32 bCleanCheck,
                            BOOL32 bMarkECCForScrub,
                            BOOL32 * pboolNeedRefresh,
                            UInt8 * pabSectorStats,
                            BOOL32 bDisableWhitening);
    UInt16* (*GetFTLCxtVbn)(void);
    BOOL32 (*GetStruct)(UInt32 dwStructType,
                        void * pvoidStructBuffer,
                        UInt32 * pdwStructSize);
    BOOL32 (*SetStruct)(UInt32 dwStructType,
                        void * pvoidStructBuffer,
                        UInt32 dwStructSize);

    UInt32 (*GetDeviceInfo)(UInt32 dwParamType);
    UInt32 (*GetMinorVersion)(void);
    UInt16 (*GetVbasPerVb)(UInt16 wVbn);
	
	
	// new func - read
    VFLReadStatusType (*ReadMultipleVbas)(UInt32 vba, UInt16 count, UInt8 *data, UInt8 *meta, 
							BOOL32 bDisableWhitening, BOOL32 bMarkECCForScrub);
	
    void (*ReadSpansInit)(VFL_ReadSpans_t *s, VFL_ReadSpansCB_t cb, UInt32 cbOpt, BOOL32 bDisableWhitening, BOOL32 bMarkECCForScrub);
    void (*ReadSpansAdd)(VFL_ReadSpans_t *s, UInt32 vba, UInt32 count, UInt8 *data, UInt8 *meta);
    VFLReadStatusType (*ReadSpans)(VFL_ReadSpans_t *s);
    
} VFLFunctions;

void    VSVFL_Register(VFLFunctions * pVFL_Functions);
void    VFL_Register(VFLFunctions * pVFL_Functions);
void    PPNVFL_Register(VFLFunctions * pVFL_Functions);
    
#define PPN_VFL_MAJOR_VER (1)
VFLFunctions * PPN_VFL_GetFunctions(void);
#define PPN_SVFL_MAJOR_VER (2)
VFLFunctions * PPN_SVFL_GetFunctions(void);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VFL_H_ */
