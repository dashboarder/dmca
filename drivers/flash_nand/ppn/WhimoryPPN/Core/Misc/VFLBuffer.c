/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : Virtual Flash Layer                                         */
/* NAME    	   : VFL Buffer Management                                       */
/* FILE        : VFLBuffer.c                                                 */
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
/*	 06-SEP-2005 [Jaesung Jung] : fix from code inspection					 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Header file inclusions                                                    */
/*****************************************************************************/
#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"

/*****************************************************************************/
/* Debug Print #defines                                                      */
/*****************************************************************************/
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
#define     BUF_ERR_PRINT(x)            { WMR_RTL_PRINT(x); WMR_PANIC(); }
#else
#define     BUF_ERR_PRINT(x)            WMR_RTL_PRINT(x)
#endif
#define     BUF_RTL_PRINT(x)            WMR_RTL_PRINT(x)

#if defined (BUF_LOG_MSG_ON)
#define     BUF_LOG_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     BUF_LOG_PRINT(x)
#endif

#if defined (BUF_INF_MSG_ON)
#define     BUF_INF_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     BUF_INF_PRINT(x)
#endif

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/
BUFCxt      *pstBufferCxt = NULL;

typedef struct
{
    UInt16 wBytesPerPage;              /* bytes per page (main)			 */
	UInt16 wBytesPerPageMeta;
} VFLBufferDeviceInfo;
static VFLBufferDeviceInfo stVFLBufferDeviceInfo;

#define BYTES_PER_PAGE          (stVFLBufferDeviceInfo.wBytesPerPage)
#define BYTES_PER_PAGE_META     (stVFLBufferDeviceInfo.wBytesPerPageMeta)

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      BUF_Init                                                             */
/* DESCRIPTION                                                               */
/*      This function initializes the buffer manager & the buffer pool.	     */
/* PARAMETERS                                                                */
/*      none			                                                     */
/* RETURN VALUES                                                             */
/* 		BUF_SUCCESS                                                          */
/*            BUF_Init is completed.                                         */
/*      BUF_CRITICAL_ERROR                                                   */
/*            BUF_Init is failed    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
Int32
BUF_Init(UInt16 wDataBytesPerPage, UInt16 wMetaBytesPerPage, UInt16 wNumberOfBuffers)
{
    UInt32 nBufIdx;
    Buffer *pBufIdx;

    if (pstBufferCxt != NULL)
    {
        return BUF_SUCCESS;
    }

    BUF_LOG_PRINT((TEXT("[BUF: IN] ++BUF_Init(void)\n")));

    stVFLBufferDeviceInfo.wBytesPerPage = wDataBytesPerPage;
    stVFLBufferDeviceInfo.wBytesPerPageMeta = wMetaBytesPerPage;

    /* Initialize buffer context */
    pstBufferCxt = (BUFCxt *)WMR_MALLOC(sizeof(BUFCxt));
    pstBufferCxt->dwNumOfBuffers = wNumberOfBuffers;
    if (pstBufferCxt == NULL)
    {
        BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Init(void) BUFCxt Init Failure\n")));
        BUF_Close();
        return BUF_CRITICAL_ERROR;
    }

    /* Initialize buffer list */
    pstBufferCxt->pBufferList = (Buffer *)WMR_MALLOC(sizeof(Buffer) * pstBufferCxt->dwNumOfBuffers);

    if (pstBufferCxt->pBufferList == NULL)
    {
        BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Init(void) BUFCxt->pBufferList Init Failure\n")));
        BUF_Close();
        return BUF_CRITICAL_ERROR;
    }

    pBufIdx = pstBufferCxt->pBufferList;

    for (nBufIdx = 0; nBufIdx < pstBufferCxt->dwNumOfBuffers; nBufIdx++)
    {
#ifdef WMR_USE_FAKE_POINTERS
        /*
         * In order to make it possible to DMA to the contents of the buffer pool,
         * OAM_Init has already allocated an IOBufferMemoryDescriptor of the appropriate
         * size.  Thus, we just have to index into it...
         */
        extern UInt8 *_OAM_PageBufferPoolBase;
        extern UInt8 *_OAM_SpareBufferPoolBase;
        pBufIdx->pData = _OAM_PageBufferPoolBase + (nBufIdx * BYTES_PER_PAGE);
        pBufIdx->pSpare = _OAM_SpareBufferPoolBase + (nBufIdx * BYTES_PER_PAGE_META);
#else /* !WMR_USE_FAKE_POINTERS */
        pBufIdx->pData = (UInt8 *)WMR_MALLOC(BYTES_PER_PAGE + BYTES_PER_PAGE_META);
        pBufIdx->pSpare = pBufIdx->pData + BYTES_PER_PAGE;
#endif

        if (pBufIdx->pData == NULL || pBufIdx->pSpare == NULL)
        {
            BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Init(void) pBufIdx->pData, pBufIdx->pSpare Init Failure\n")));
            BUF_Close();
            return BUF_CRITICAL_ERROR;
        }

        pBufIdx->pDataBak = pBufIdx->pData;
#ifdef WMR_USE_FAKE_POINTERS    /* need to save the spare pointer as it is not contiguous */
        pBufIdx->pSpareBak = pBufIdx->pSpare;
#endif
        pBufIdx->eStatus = BUF_FREE;

        pBufIdx++;
    }

    pstBufferCxt->nFreeBuffers = pstBufferCxt->dwNumOfBuffers;

    BUF_LOG_PRINT((TEXT("[BUF:OUT] --BUF_Init(void) nRe=0x%x\n"), BUF_SUCCESS));
    return BUF_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      BUF_Get	                                                             */
/* DESCRIPTION                                                               */
/*      This function returns a new free buffer. 						     */
/*		if there is not a free buffer, this function calls the sync function */
/*		and generates a new free buffer.									 */
/* PARAMETERS                                                                */
/*		eType			[IN]												 */
/*				buffer type													 */
/* RETURN VALUES                                                             */
/* 		Buffer	                                                             */
/*            BUF_Get is completed.                                          */
/*      NULL			                                                     */
/*            BUF_Get is failed.    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
Buffer *
BUF_Get(BUFType eType)
{
    UInt32 nBufIdx;
    Buffer  *pBufRet;

    BUF_LOG_PRINT((TEXT("[BUF: IN] ++BUF_Get(eType = %d)\n"), eType));

    /* Check buffer context was initialized */
    if (pstBufferCxt == NULL)
    {
        BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Get(eType = %d) Buffer manager is not initialized\n"), eType));
        return NULL;
    }

    /* NirW - from the checked in M1 code - Check buffer context was not written over */
    if (pstBufferCxt->pBufferList == NULL)
    {
        BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Get(eType = %d) Buffer pointer was overwritten!\n"), eType));
        return NULL;
    }

    if (pstBufferCxt->nFreeBuffers == 0)
    {
        BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Get(eType = %d) No free buffer\n"), eType));
        return NULL;
    }

    /* Find a free buffer from the buffer pool. */
    pBufRet = pstBufferCxt->pBufferList;

    for (nBufIdx = 0; nBufIdx < pstBufferCxt->dwNumOfBuffers; nBufIdx++, pBufRet++)
    {
        if (pBufRet->eStatus == BUF_FREE)
        {
            break;
        }
    }

    if (nBufIdx >= pstBufferCxt->dwNumOfBuffers)
    {
        BUF_ERR_PRINT((TEXT("[BUF:ERR]  BUF_Get(eType = %d) Can't find a free buffer\n"), eType));
        return NULL;
    }

    /* Check the buffer type */
    if (eType == BUF_SPARE_ONLY)
    {
        pBufRet->pData = NULL;
    }
    else if (eType == BUF_MAIN_ONLY)
    {
        pBufRet->pSpare = NULL;
    }

    /* Initialize the spare buffer */
    if (eType != BUF_MAIN_ONLY)
    {
        WMR_MEMSET(pBufRet->pSpare, 0xFF, BYTES_PER_PAGE_META);
    }

    /* Change the buffer status & bitmap */
    pBufRet->eStatus = BUF_ALLOCATED;
/*	pBufRet->nBitmap = 0; */

    /* Decrease the number of free buffers */
    pstBufferCxt->nFreeBuffers--;

    BUF_LOG_PRINT((TEXT("[BUF:OUT] --BUF_Get(eType = %d)\n"), eType));

    return pBufRet;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      BUF_Release                                                          */
/* DESCRIPTION                                                               */
/*      This function releases the buffer to the buffer pool.			     */
/* PARAMETERS                                                                */
/*      pBuf			[IN]	                                             */
/*				buffer pointer												 */
/* RETURN VALUES                                                             */
/*		none																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
void
BUF_Release(Buffer *pBuf)
{
    UInt16 wIdx;

    BUF_LOG_PRINT((TEXT("[BUF: IN] ++BUF_Release(pBuf = %x)\n"), pBuf));

    if (pBuf == NULL)
    {
        return;
    }

    //WMR_ASSERT((pBuf - pstBufferCxt->pBufferList) >= 0 &&
//		  	  (pBuf - pstBufferCxt->pBufferList) < pstBufferCxt->dwNumOfBuffers);
    for (wIdx = 0; wIdx < pstBufferCxt->dwNumOfBuffers; wIdx++)
    {
        if (pBuf == &pstBufferCxt->pBufferList[wIdx])
        {
            break;
        }
    }
    WMR_ASSERT(wIdx < pstBufferCxt->dwNumOfBuffers);
    /* NirW */
    if (pBuf->eStatus == BUF_FREE)
    {
        return;
    }

    WMR_ASSERT(pBuf->eStatus == BUF_ALLOCATED);

    /* Reinitialize the buffer */
    pBuf->pData = pBuf->pDataBak;
#ifdef WMR_USE_FAKE_POINTERS    /* need to restore non-adjacent spare pointer */
    pBuf->pSpare = pBuf->pSpareBak;
#else
    pBuf->pSpare = pBuf->pDataBak + BYTES_PER_PAGE;
#endif

    pBuf->eStatus = BUF_FREE;

    pstBufferCxt->nFreeBuffers++;

    WMR_ASSERT(pstBufferCxt->nFreeBuffers <= pstBufferCxt->dwNumOfBuffers);

    BUF_LOG_PRINT((TEXT("[BUF:OUT] --BUF_Release(pBuf = %x)\n"), pBuf));

    return;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      BUF_Close                                                            */
/* DESCRIPTION                                                               */
/*      This function releases buffer manager context.					     */
/* PARAMETERS                                                                */
/*      none			                                                     */
/* RETURN VALUES                                                             */
/* 		none		                                                         */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
void
BUF_Close(void)
{
    UInt32 nBufIdx;
    Buffer *pBufIdx;

    BUF_LOG_PRINT((TEXT("[BUF: IN] ++BUF_Close(void)\n")));

    if (pstBufferCxt != NULL)
    {
        if (pstBufferCxt->pBufferList != NULL)
        {
            pBufIdx = pstBufferCxt->pBufferList;

            for (nBufIdx = 0; nBufIdx < pstBufferCxt->dwNumOfBuffers; nBufIdx++)
            {
#ifndef WMR_USE_FAKE_POINTERS
                WMR_FREE(pBufIdx->pDataBak, (BYTES_PER_PAGE + BYTES_PER_PAGE_META));
#endif
                pBufIdx->pData = NULL;
                pBufIdx->pSpare = NULL;

                pBufIdx++;
            }

            WMR_FREE(pstBufferCxt->pBufferList, (sizeof(Buffer) * pstBufferCxt->dwNumOfBuffers));

            pstBufferCxt->pBufferList = NULL;
        }

        WMR_FREE(pstBufferCxt, sizeof(BUFCxt));

        pstBufferCxt = NULL;
    }

    BUF_LOG_PRINT((TEXT("[BUF:OUT] --BUF_Close(void)\n")));

    return;
}

void
BUF_ReleaseAllBuffers(void)
{
    UInt32 nBufIdx;
    Buffer *pBufIdx;

    pBufIdx = pstBufferCxt->pBufferList;

    for (nBufIdx = 0; nBufIdx < pstBufferCxt->dwNumOfBuffers; nBufIdx++)
    {
        BUF_Release(pBufIdx);
        pBufIdx++;
    }
}

