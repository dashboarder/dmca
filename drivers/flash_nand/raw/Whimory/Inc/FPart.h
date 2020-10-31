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

#ifndef _UNDER_VFL_H_
#define _UNDER_VFL_H_
#include "FIL.h"
/*****************************************************************************/
/* Type definition of checksum (confirm) state                               */
/*****************************************************************************/

/*****************************************************************************/
/* exported function prototype of VFL                                        */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    SIGNATURE_STYLE_BLOCKZERO_NOHEADER = 100,
    SIGNATURE_STYLE_RESERVEDBLOCK_HEADER = 101,
    SIGNATURE_STYLE_NOTSET = 250,
} FPartSignatureStyle;
    
// Forward declare VFL
//typedef struct _VFLFunctions VFLFunctions;
struct _VFLFunctions;
    
#define FPART_INIT_OPTION_DEV_UNIQUE (1 << 0)
#define FPART_INIT_OPTION_DIAG_CTRL  (1 << 1)

typedef struct
{
#ifndef AND_READONLY
    BOOL32 (*WriteInitialBBT)(void);
    Int32 (*FactoryReformat)(BOOL32 Force, BOOL32 boolEraseBlockZero, UInt32 dwBootAreaSize);
    BOOL32 (*WriteDeviceUniqueInfo)(UInt8 * pabData, UInt32 dwDataBufferSize);
    BOOL32 (*WriteDiagnosticInfo)(UInt8 * pabData, UInt32 dwDataBufferSize);	
    BOOL32 (*WriteSignature)(void * pvoidSignature, UInt32 dwSignatureSize);
    BOOL32 (*WriteSpecialBlock)(void * data, UInt32 data_size, UInt16 type);
#endif // ! AND_READONLY
    BOOL32 (*Neuralize)(void);
    BOOL32 (*ReadDeviceUniqueInfo)(UInt8 * pabData, UInt32 dwDataBufferSize);
    BOOL32 (*ReadDiagnosticInfo)(UInt8 * pabData, UInt32 dwDataBufferSize);
    BOOL32 (*VerifyProductionFormat)(void);
    BOOL32 (*ReadSignature)(void * pvoidSignature, UInt32 dwSignatureSize);
    BOOL32 (*Init)(LowFuncTbl *pFILFunctions, UInt32 dwOptions);
    void (*SetSignatureStyle)(void * pvoidFPartFunctions,
                              FPartSignatureStyle fpartSignatureStyle);
    BOOL32 (*validateSignatureLocation)(void * signLocationPtr );
    LowFuncTbl* (*GetLowFuncTbl)(void);
    UInt32 (*GetMinorVersion)(void);
    BOOL32 (*Format)(struct _VFLFunctions *pVFL, UInt32 dwOptions);
    BOOL32 (*AllocateSpecialBlockType)(SpecialBlockAddress *blocks, UInt16 count, UInt16 type);
    BOOL32 (*IsSpecialBlockTypeAllocated)(UInt16 type);
    BOOL32 (*MapSpecialBlocks)(SpecialBlockAddress *blocks, UInt16 *types, UInt16 *count);
    BOOL32 (*ReadSpecialBlock)(void *data, UInt32 data_size, UInt16 type);
    void   (*ChangeCacheState)(BOOL32 validate);
    void*  (*GetSingleCommandStruct)(void); // allow vfl and fpart to share single operation struct
    void   (*Close)(void);
} FPartFunctions;

void    FPart_Register(FPartFunctions * pFPartFunctions);
void    PPNFPart_Register(FPartFunctions * pFPartFunctions);
    
#define PPN_FPart_MAJOR_VER (0)
FPartFunctions *PPN_FPart_GetFunctions(void);

BOOL32   VFL_ReadBBT(UInt16 wCS, UInt8 * pBBT);
BOOL32   VFL_ReadBBTWithoutSpecial(UInt16 wCS, UInt8 * pBBT);

/*

    NAME
        _MarkBlockAsBadInBBT
    DESCRIPTION
        This define marks block as bad in the bbt.
    PARAMETERS
        pBBT 	 [IN]	bbt buffer
        idx		 [IN]	block index
    RETURN VALUES
        none
    NOTES

 */

#define _MarkBlockAsBadInBBT(pBBT, idx) \
    { \
        UInt16 dByteLocation; \
        UInt8 bBit; \
        dByteLocation = ((UInt16)(idx)) >> 3; \
        bBit = (UInt8)(~(1 << (((UInt8)(idx)) & 0x07))); \
        ((UInt8 *)(pBBT))[dByteLocation] &= bBit; \
    }

/*

    NAME
        _MarkBlockAsGoodInBBT
    DESCRIPTION
        This define marks block as good in the bbt.
    PARAMETERS
        pBBT 	 [IN]	bbt buffer
        idx		 [IN]	block index
    RETURN VALUES
        none
    NOTES

 */
#define _MarkBlockAsGoodInBBT(pBBT, idx) \
    { \
        UInt16 dByteLocation; \
        UInt8 bBit; \
        dByteLocation = ((UInt16)(idx)) >> 3; \
        bBit = (1 << (((UInt8)(idx)) & 0x07)); \
        ((UInt8 *)(pBBT))[dByteLocation] |= bBit; \
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _UNDER_VFL_H_ */
