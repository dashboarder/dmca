/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : Virtual Flash Layer                                         */
/* NAME    	   : VFL Buffer Management                                       */
/* FILE        : VFLBuffer.h                                                 */
/* PURPOSE     : This file contains routines for managing buffers which      */
/*              whimory uses. 						                         */
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
/*   13-JUL-2005 [Jaesung Jung] : separate from vfl.c & reorganize code      */
/*                                                                           */
/*****************************************************************************/

#ifndef _VFL_BUFFER_H_
#define _VFL_BUFFER_H_

/*****************************************************************************/
/* Return value of BUF_XXX()                                                 */
/*****************************************************************************/
#define     BUF_SUCCESS                     ANDErrorCodeOk
#define     BUF_CRITICAL_ERROR              ANDErrorCodeAllocFailErr

/*****************************************************************************/
/* Buffer state enum definition                                              */
/*****************************************************************************/
typedef enum
{
    BUF_FREE = 0x10000000,   /* the buffer is free			 		 */
    BUF_ALLOCATED = 0x10000001,   /* the buffer is allocated to some layer */
} BUFStat;

/*****************************************************************************/
/* Buffer type enum & macro definition                                       */
/*****************************************************************************/
typedef enum
{
    BUF_MAIN_AND_SPARE = 0x10000000,   /* to get a buffer which has main & spare*/
    BUF_MAIN_ONLY = 0x10000001,   /* to get a buffer which has main only	 */
    BUF_SPARE_ONLY = 0x10000002    /* to get a buffer which has spare only	 */
} BUFType;

#define     BITMAP_FULL_PAGE                ((1 << SECTORS_PER_PAGE) - 1)

/*****************************************************************************/
/* Buffer structure definition	                                             */
/*****************************************************************************/
typedef struct Buf
{
    UInt8   *pData;             /* buffer area for main area			     */
    UInt8   *pSpare;            /* buffer area for spare area				 */
//	UInt32 	 nBitmap;			/* valid sector bitmap for pData			 */
    BUFStat eStatus;           /* the status of buffer						 */
    UInt8   *pDataBak;          /* the original pointer of data buffer		 */
#ifdef WMR_USE_FAKE_POINTERS /* data and spare buffers are not adjacent */
    UInt8   *pSpareBak;         /* the original pointer of spare buffer		 */
#endif
} Buffer;

typedef struct
{
    Buffer  *pBufferList;       /* the list of all the buffers				 */
    UInt32 nFreeBuffers;      /* the count of free buffers				 */
    UInt32 dwNumOfBuffers;
} BUFCxt;

/*****************************************************************************/
/* exported function prototype of VFL buffer manager                         */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Int32    BUF_Init(UInt16 wDataBytesPerPage, UInt16 wMetaBytesPerPage, UInt16 wNumberOfBuffers);
Buffer  *BUF_Get(BUFType eType);
void     BUF_Release(Buffer   *pBuf);
void     BUF_Close(void);
void     BUF_ReleaseAllBuffers(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VFL_BUFFER_H_ */
