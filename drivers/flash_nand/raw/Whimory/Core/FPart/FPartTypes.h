/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : Virtual Flash Layer                                         */
/* NAME    	   : VFL types definition header                                 */
/* FILE        : VFLTypes.h	                                                 */
/* PURPOSE 	   : This header defines Data types which are shared             */
/*               by all VFL submodules                                       */
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
/*   04-AUG-2003 [HM Jung]		: first writing								 */
/*   14-JUL-2005 [Jaesung Jung] : reorganize code                            */
/*	 06-SEP-2005 [Jaesung Jung] : fix from code inspection					 */
/*   24-JAN-2006 [Yangsup Lee ] : support un-pair bad block management       */
/*   31-MAR-2006 [Yangsup Lee ] : support ftl meta block wear leveling       */
/*                                                                           */
/*****************************************************************************/
#ifndef _UNDER_VFL_TYPES_H_
#define _UNDER_VFL_TYPES_H_

/* Device Information Structures and defines */

#define VFL_META_STRUCT_VERSION     0x00000004 /* changed Oct 6th 2006 */
#define VFL_BBT_STRUCT_VERSION      0x00000004 /* changed Oct 6th 2006 */

#define BBT_BLOCK_SIGNATURE                     (UInt8*)"DEVICEINFOBBT\0\0\0"
#define BBT_BLOCK_SIGNATURE_SIZE                16

#define UNIQUE_INFO_BLOCK_SIGNATURE             (UInt8*)"DEVICEUNIQUEINFO"
#define UNIQUE_INFO_BLOCK_SIGNATURE_SIZE        16

#define NAND_DRIVER_SIGN_BLOCK_SIGNATURE        (UInt8*)"NANDDRIVERSIGN\0\0"
#define NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE   16

#define DIAG_INFO_BLOCK_SIGNATURE               (UInt8*)"DIAGCONTROLINFO\0"
#define DIAG_INFO_BLOCK_SIGNATURE_SIZE           16

#define BBT_SIZE_PER_CS                         ((BLOCKS_PER_CS / 8) + (BLOCKS_PER_CS % 8 ? 1 : 0))

#define WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS (4)

#define VFL_BLOCK_INDEX_MASK                        (0xFFFF)
#define VFL_INVALID_INFO_INDEX                      (0xFFFF)

#define FPART_SPECIAL_BLK_BBT_PRIMARY_IDX        (0) // Primary copy of bad block table
#define FPART_SPECIAL_BLK_BBT_BACKUP_IDX         (1) // Backup copy of bad block table
#define FPART_SPECIAL_BLK_DEV_UNIQUE_PRIMARY_IDX (2) // Primary copy of device-unique information
#define FPART_SPECIAL_BLK_DEV_UNIQUE_BACKUP_IDX  (3) // Backup copy of device-unique information
#define FPART_SPECIAL_BLK_DEV_SIGNATURE_IDX      (4) // Driver signature
#define FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX  (5) // Primary copy of diag control bits
#define FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX   (6) // Backup copy of diag control bits

#define FPART_COPIES_OF_SPECIAL_INFO             (2) // Copies of syscfg and diag bits

#define MAX_NUM_SPECIAL_BLOCKS_PER_CS            (8)    /* This should always be 8 as BBTInfoHeaderStruct and SpecialBlockHeaderStruct should be of the same size*/
#define MAX_NUM_OF_SPECIALS                      (5)    /* no of special blocks ( backups not included )*/
#define MAX_SPECIAL_BLOCK_COPIES                 (2)
#define SPECIAL_BLOCK_SIGNATURE_SIZE             BBT_BLOCK_SIGNATURE_SIZE

typedef struct
{
    UInt8 abSignature[16];
    UInt32 dwVersion;
    UInt32 adwSpecialBlocks[MAX_NUM_SPECIAL_BLOCKS_PER_CS];
    UInt32 dwBBTSize;
} BBTInfoHeaderStruct;

typedef struct
{
    UInt32 location; 
    BOOL32 locationValid;    
} specialBlockLocation;
typedef struct
{
    UInt8 abSignature[SPECIAL_BLOCK_SIGNATURE_SIZE];
    specialBlockLocation sbLocation[MAX_SPECIAL_BLOCK_COPIES];
} specialBlockCache;

typedef struct
{
    UInt8 abSignature[16];
    UInt32 dwVersion;
    UInt32 adwReserved[MAX_NUM_SPECIAL_BLOCKS_PER_CS];
    UInt32 dwDataSize;
} SpecialBlockHeaderStruct;

typedef struct
{
    UInt16 wPagesPerBlock;             /* the count of pages per block		 */
    UInt16 wNumOfCEs;
    CheckInitialBadType checkInitialBadType;
    UInt16 wDiesPerCS;
    UInt16 wBlocksPerCS;               /* the count of blocks per CS		 */

    UInt16 wBlocksPerDie;
    UInt16 wDieStride;
    UInt16 wBlockStride;
    UInt16 wLastBlock;
    UInt16 wUserBlocksPerCS;
    UInt16 wBytesPerPage;              /* bytes per page (main)			 */
    UInt16 wBytesPerSpare;
    UInt16 wBytesPerBLPage;
} UnderVFLDeviceInfo;

#endif /* _UNDER_VFL_TYPES_H_ */
