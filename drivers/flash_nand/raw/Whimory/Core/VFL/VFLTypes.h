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
#ifndef _VFL_TYPES_H_
#define _VFL_TYPES_H_

typedef struct
{
    UInt16 wPagesPerBlock;             /* the count of pages per block		 */
    UInt16 wPagesPerSuBlk;             /* the count of pages per virtual block */
    UInt32 dwPagesTotal;               /* the total number of pages		 */
    UInt16 wUserSuBlkTotal;            /* the total number of data virtual block */
    UInt32 dwUserPagesTotal;           /* the total number of data sector   */

    UInt16 wBlocksPerBank;             /* the count of blocks per bank		 */

    UInt16 wBytesPerPage;              /* bytes per page (main)			 */

    UInt32 dwValidMetaPerLogicalPage;
    UInt32 dwTotalMetaPerLogicalPage;
    UInt32 dwLogicalPageSize;

    UInt16 wNumOfBank;                 /* the number of banks				 */
    UInt8 bRefreshThreshold;          // number of bit flips beyond which we want to rewrite the page

    // taken from WMRLayout
    UInt16 dwVFLAreaStart;             /* the block number where VFL area starts */
    UInt16 wVFLAreaSize;               /* the block number of VFL area size */
    UInt16 wReservedSecSize;           /* the size of reserved section		 */

    UInt16 wFTLCxtStart;               /* FTL cxt section start			 */
    UInt16 wFreeSecStart;              /* free section start				 */
} VFLWMRDeviceInfo;

#define     PAGES_PER_BLOCK         (stVFLDeviceInfo.wPagesPerBlock)
#define     PAGES_PER_SUBLK         (stVFLDeviceInfo.wPagesPerSuBlk)
#define     PAGES_TOTAL             (stVFLDeviceInfo.dwPagesTotal)
#define     USER_SUBLKS_TOTAL       (stVFLDeviceInfo.wUserSuBlkTotal)
#define     USER_PAGES_TOTAL        (stVFLDeviceInfo.dwUserPagesTotal)
#define     BLOCKS_PER_BANK         (stVFLDeviceInfo.wBlocksPerBank)
#define     BYTES_PER_PAGE          (stVFLDeviceInfo.wBytesPerPage)
#define     BANKS_TOTAL             (stVFLDeviceInfo.wNumOfBank)

#define     VFL_AREA_START          (stVFLDeviceInfo.dwVFLAreaStart)
#define     VFL_AREA_SIZE           (stVFLDeviceInfo.wVFLAreaSize)

#define     VFL_INFO_SECTION_START  (VFL_AREA_START)
#define     VFL_INFO_SECTION_SIZE   (4)
#define     RESERVED_SECTION_SIZE   (stVFLDeviceInfo.wReservedSecSize)

#define     FTL_CXT_SECTION_START   (stVFLDeviceInfo.wFTLCxtStart)
#define     FTL_INFO_SECTION_SIZE   FTL_CXT_SECTION_SIZE
#define     LOG_SECTION_SIZE        (FREE_SECTION_SIZE - FREE_LIST_SIZE)
#define     FREE_SECTION_SIZE       (20)
#define     FREE_LIST_SIZE          (3)

#define     FTL_AREA_START          (VFL_AREA_START + VFL_AREA_SIZE)
#define     FTL_AREA_SIZE           (FTL_INFO_SECTION_SIZE + FREE_SECTION_SIZE + USER_SUBLKS_TOTAL)
#define     FIL_ECC_REFRESH_TRESHOLD (stVFLDeviceInfo.bRefreshThreshold)

#define     BBT_SIZE_PER_BANK       ((BLOCKS_PER_BANK / 8) + (BLOCKS_PER_BANK % 8 ? 1 : 0))
#define     BYTES_PER_SECTOR        (512)
#define     WMR_MAX_DEVICE          (8) /* the maximum number of banks	*/

/*****************************************************************************/
/* VFL constant definitions					                                 */
/*****************************************************************************/
#define     INIT_BAD_MARK               (0)  /* bad mark position in spare	 */
#define     INIT_BAD_MARK_SECOND        (64) /* bad mark position in spare	 */
                                             /* (2 plane programming)		 */

#define     BAD_MARK_COMPRESS_SIZE      (8)

/*****************************************************************************/
/* VFL context status(confirm) mark definition                               */
/*****************************************************************************/
#define     PAGE_INCOMPLETE             (0xFF)
#define     PAGE_VALID                  (0x00)

#define VFL_BAD_MARK_INFO_TABLE_SIZE (WMR_MAX_VB / 8 / BAD_MARK_COMPRESS_SIZE)
/*****************************************************************************/
/* Data structure for storing the VFL context definition					 */
/*****************************************************************************/
/* NOTICE !!!														*/
/* this structure is used directly to load VFL context by WMR_MEMCPY*/
/* so the byte pad of this structure must be 0		   				*/
typedef struct
{
    UInt32 dwGlobalCxtAge;                     /* age for FTL meta information search  */
    UInt16 aFTLCxtVbn[FTL_CXT_SECTION_SIZE];   /* page address (FTL virtual addressing space) of FTL Cxt */
    UInt16 wPadding;                           /* padding (align to UInt32) */

    UInt32 dwCxtAge;                      /* context age 0xFFFFFFFF --> 0x0 - NirW - check if the variable is used anywhere - what is the difference between dwGlobalCxtAge and dwCxtAge */
    UInt16 wCxtLocation;                  /* current context block location  (Physical) */
    UInt16 wNextCxtPOffset;               /* current context page offset information */

    /* this data is used for summary 					*/
    UInt16 wNumOfInitBadBlk;       /* the number of initial bad blocks - used for VFL format */
    UInt16 wNumOfWriteFail;        /* the number of write fail - currently not used */
    UInt16 wNumOfEraseFail;        /* the number of erase fail - updated (++) every time there is an erase failure that causes remapping of a block */

    /* bad blocks management table & good block pointer */
    UInt16 wBadMapTableMaxIdx;     /* index to the last bad block updated in the aBadMapTable */
    UInt16 wReservedSecStart;      /* index of the first physical block that will be used as reserved in the bank (the first block after VFL Cxt Info) */
    UInt16 wReservedSecSize;       /* number of physical blocks that are available as reserved (some might be bad blocks) */
    UInt16 aBadMapTable[WMR_MAX_RESERVED_SIZE]; /* remapping table of bad blocks - the UInt 16 value set here is the virtual block (vfl address space) that is being replaced */
    UInt8 aBadMark[VFL_BAD_MARK_INFO_TABLE_SIZE]; /* compact bitmap presentation of bad block (initial and accumulated) */

    /* bad blocks management table within VFL info area */
    UInt16 awInfoBlk[VFL_INFO_SECTION_SIZE];      /* physical block addresses where Cxt information is stored */
    UInt16 wBadMapTableScrubIdx;                  /* Index for the scrub list (start from the top of aBadMapTable */
} VFLCxt;

/* VFLMeta size is 2048 bytes(4 sector) */
#define VFL_META_VERSION 0x00000001
typedef struct
{
    VFLCxt stVFLCxt;
    UInt8 abReserved[((BYTES_PER_SECTOR - 3) * sizeof(UInt32)) - sizeof(VFLCxt)];
    UInt32 dwVersion;
    UInt32 dwCheckSum;
    UInt32 dwXorSum;
} VFLMeta;

/*****************************************************************************/
/* Data structure for VFL context spare area								 */
/*****************************************************************************/
/* spare layout for SLC & MLC							*/
typedef struct
{
    UInt32 dwCxtAge;               /* context age 0xFFFFFFFF --> 0x0   */
    UInt32 dwReserved;             /* reserved			 		     */
    UInt8 cStatusMark;            /* status (confirm) mark - currently not used for anything */
    UInt8 bSpareType;             /* spare type */
    /* reserved for main ECC */
} VFLSpare;

#define VFL_SPARE_TYPE_CXT  0x80

/*****************************************************************************/
/* Asynchronous operation management structure & enum definition			 */
/*****************************************************************************/

/* Device Information Structures and defines */

#define VFL_META_STRUCT_VERSION     0x00000004 /* changed Oct 6th 2006 */
#define VFL_BBT_STRUCT_VERSION      0x00000004 /* changed Oct 6th 2006 */

#define BBT_BLOCK_SIGNATURE                     (UInt8*)"DEVICEINFOBBT\0\0\0"
#define BBT_BLOCK_SIGNATURE_SIZE                16

#define UNIQUE_INFO_BLOCK_SIGNATURE             (UInt8*)"DEVICEUNIQUEINFO"
#define UNIQUE_INFO_BLOCK_SIGNATURE_SIZE        16

#define NAND_DRIVER_SIGN_BLOCK_SIGNATURE        (UInt8*)"NANDDRIVERSIGN\0\0"
#define NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE   16

#define NUMBER_OF_SPECIAL_BLOCKS            (5) /* discuss what is the right value here with Mike Smith */

#define BBT_SIZE_PER_BANK           ((BLOCKS_PER_BANK / 8) + (BLOCKS_PER_BANK % 8 ? 1 : 0))

#define WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS (4)

typedef struct
{
    UInt8 abSignature[16];
    UInt32 dwVersion;
    UInt32 adwSpecialBlocks[8];
    UInt32 dwBBTSize;
} BBTInfoHeaderStruct;

#ifdef AND_COLLECT_STATISTICS
typedef struct
{
    UInt64 ddwPagesWrittenCnt;
    UInt64 ddwPagesReadCnt;
    UInt64 ddwBlocksErasedCnt;
    UInt64 ddwSingleWriteCallCnt;
    UInt64 ddwSingleReadCallCnt;
    UInt64 ddwSequetialReadCallCnt;
    UInt64 ddwScatteredReadCallCnt;
    UInt64 ddwMultipleWriteCallCnt;
    UInt64 ddwEraseCallCnt;
} VFLStatistics;

#define VFL_STATISTICS_DESCREPTION { \
        "ddwPagesWrittenCnt", \
        "ddwPagesReadCnt", \
        "ddwBlocksErasedCnt", \
        "ddwSingleWriteCallCnt", \
        "ddwSingleReadCallCnt", \
        "ddwSequetialReadCallCnt", \
        "ddwScatteredReadCallCnt", \
        "ddwMultipleWriteCallCnt", \
        "ddwEraseCallCnt", \
}
#endif /* AND_COLLECT_STATISTICS */

#endif /* _VFL_TYPES_H_ */
