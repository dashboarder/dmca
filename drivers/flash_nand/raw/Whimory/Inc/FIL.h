/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : FIL				                                         */
/* NAME    	   : Flash Interface Layer header file						     */
/* FILE        : FIL.h			                                             */
/* PURPOSE 	   : This file contains the definition and protypes of exported  */
/*           	 functions for FIL of Whimory.							     */
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
/*   19-JUL-2005 [Jaesung Jung] : first writing                              */
/*                                                                           */
/*****************************************************************************/

#ifndef _FIL_H_
#define _FIL_H_

#include "ANDTypes.h"
#include "WMRFeatures.h"
#include "PPN_FIL.h"

/*****************************************************************************/
/* Return value of FIL_XXX() & physical interface function pointer           */
/*****************************************************************************/
#define     FIL_SUCCESS                     ANDErrorCodeOk
#define     FIL_SUCCESS_CLEAN               ANDErrorCodeCleanOk
#define     FIL_CRITICAL_ERROR              ANDErrorCodeHwErr
#define     FIL_U_ECC_ERROR                 ANDErrorCodeUserDataErr
#define     FIL_WRITE_FAIL_ERROR            ANDErrorWriteFailureErr
#define     FIL_UNSUPPORTED_ERROR           ANDErrorCodeNotSupportedErr

/*****************************************************************************/
/* FIL Function Table Data Structures                                        */
/*****************************************************************************/
typedef struct
{
    void (*Reset)(void);
    Int32 (*ReadWithECC)(UInt16 wBank, UInt32 nPpn, UInt8 *pDBuf, UInt8 *pSBuf, UInt8 * pbCorrectedBits, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening);
    Int32 (*ReadNoECC)(UInt16 wBank, UInt32 nPpn, UInt8 *pDBuf, UInt8 *pSBuf);
    Int32 (*ReadMultiplePages)(UInt32 * padwPpn, UInt8* pDBuf, UInt8* pSBuf, UInt16 wNumOfPagesToRead, UInt8 * pbCorrectedBits, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening);
    Int32 (*ReadScatteredPages)(UInt16 * pawBank, UInt32 * padwPpn, UInt8* pDBuf, UInt8* pSBuf, UInt16 wNumOfPagesToRead, UInt8 * pbCorrectedBits, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening);
    Int32 (*ReadMaxECC)(UInt16 wBank, UInt32 nPpn, UInt8 *pDBuf);
    Int32 (*ReadBLPage)(UInt16 wCS, UInt32 dwPpn, UInt8 *pabData);
    Int32 (*WriteBLPage)(UInt16 wCS, UInt32 dwPpn, UInt8 *pabData);
    BOOL32 (*Write2ndBootLoader)(UInt8* padw2ndBootLoader, UInt32 dwBLPageCount, UInt32 dwBLOffsetPage);
    BOOL32 (*Read2ndBootLoader)(UInt8* padw2ndBootLoader, UInt32 dwBLPageCount);
    Int32 (*Erase)(UInt16 wBank, UInt32 wPbn);
    Int32 (*Write)(UInt16 wBank, UInt32 nPpn, UInt8 *pDBuf, UInt8 *pSBuf, BOOL32 bDisableWhitening);
    Int32 (*WriteScatteredPages)(UInt16 * pawBanks, UInt32 * padwPpn, UInt8 * pabDataBuf, UInt8 * pabMetaBuf, UInt16 wNumOfPagesToWrite, UInt16 * pwFailingCE, BOOL32 bDisableWhitening, UInt32 *pwFailingPageNum);
    Int32 (*WriteMultiplePages)(UInt32 * padwPpn, UInt8* pDBuf, UInt8* pSBuf, UInt16 wNumOfPagesToWrite, BOOL32 boolAligned, BOOL32 boolCorrectBank, UInt16 * pwFailingCE, BOOL32 bDisableWhitening, UInt32 *pwFailingPageNum);
    Int32 (*EraseMultiple)(UInt16 *pawPbn, BOOL32 boolAligned, BOOL32 boolCorrectBank, UInt16 *failingCE, UInt32 *failingBlock);
    Int32 (*WriteNoECC)(UInt16 wBank, UInt32 nPpn, UInt8 *pDBuf, UInt8 *pSBuf);
    Int32 (*WriteMaxECC)(UInt16 wBank, UInt32 nPpn, UInt8 *pDBuf);
    void (*CalcCurrentTiming)(void);
    void (*Notify)(UInt32 notifyCode);
    BOOL32  (*Get2ndBootLoaderInfo)(UInt32 * pdwMax2ndLoaderSize, UInt32 * pdw2ndLoaderPageSize);
    UInt32 (*GetDeviceInfo)(UInt32 dwParamType);
    void (*SetDeviceInfo)(UInt32 dwParamType, UInt32 dwVal);
    void (*SetWhiteningState)(BOOL32 enable);
    void (*SetWhiteningMetadataState)(BOOL32 enable);
    void (*RegisterCurrentTransaction)(UInt64 ddwLba, UInt32 dwNum, void * pDest);
    Int32 (*UpdateFirmware)(UInt16 wCS, UInt8 *pDBuf, UInt32 length);
    void (*PerformCommandList)(PPNCommandStruct **commands, UInt32 num_commands);
} LowFuncTbl;

/*****************************************************************************/
/* Notification values                                                       */
/*****************************************************************************/
#define AND_NOTIFY_REFORMAT    1
#define AND_NOTIFY_RECOVER     2

/*****************************************************************************/
/* Statistics gathering definitions                                          */
/*****************************************************************************/
typedef struct _FILStatistics
{
    UInt64 ddwPagesWrittenCnt;
    UInt64 ddwPagesReadCnt;
    UInt64 ddwBlocksErasedCnt;
    UInt64 ddwResetCallCnt;
    UInt64 ddwReadWithECCCallCnt;
    UInt64 ddwReadMaxECCCallCnt;
    UInt64 ddwReadNoECCCallCnt;
    UInt64 ddwReadMultipleCallCnt;
    UInt64 ddwReadScatteredCallCnt;
    UInt64 ddwWriteSingleCallCnt;
    UInt64 ddwWriteMaxECCCallCnt;
    UInt64 ddwWriteMultipleCallCnt;
    UInt64 ddwWriteScatteredCallCnt;
    UInt64 ddwSingleEraseCallCnt;
    UInt64 ddwMultipleEraseCallCnt;
    UInt64 ddwReadCleanCnt;
    UInt64 ddwReadECCErrorCnt;
    UInt64 ddwReadHWErrorCnt;
    UInt64 ddwWriteHWErrCnt;
    UInt64 ddwWriteNANDErrCnt;
    UInt64 ddwEraseHWErrCnt;
    UInt64 ddwEraseNANDErrCnt;
    UInt64 addwReadBitFlipsCnt[FIL_MAX_ECC_CORRECTION + 1]; // 0 to max number of corrected bits
    UInt64 ddwTimeoutCnt;
    UInt64 ddwPartialUECCCnt;
    UInt64 ddwPartialCleanCnt;
    UInt64 ddwReadRefreshCnt;
    UInt64 ddwReadRetireCnt;
} FILStatistics;

#define FIL_STATISTICS_DESCREPTION { \
        "ddwPagesWrittenCnt", \
        "ddwPagesReadCnt", \
        "ddwBlocksErasedCnt", \
        "ddwResetCallCnt", \
        "ddwReadWithECCCallCnt", \
        "ddwReadMaxECCCallCnt", \
        "ddwReadNoECCCallCnt", \
        "ddwReadMultipleCallCnt", \
        "ddwReadScatteredCallCnt", \
        "ddwWriteSingleCallCnt", \
        "ddwWriteMaxECCCallCnt", \
        "ddwWriteMultipleCallCnt", \
        "ddwWriteScatteredCallCnt", \
        "ddwSingleEraseCallCnt", \
        "ddwMultipleEraseCallCnt", \
        "ddwReadCleanCnt", \
        "ddwReadECCErrorCnt", \
        "ddwReadHWErrorCnt", \
        "ddwWriteHWErrCnt", \
        "ddwWriteNANDErrCnt", \
        "ddwEraseHWErrCnt", \
        "ddwEraseNANDErrCnt", \
        "addwReadBitFlipsCnt[0]", \
        "addwReadBitFlipsCnt[1]", \
        "addwReadBitFlipsCnt[2]", \
        "addwReadBitFlipsCnt[3]", \
        "addwReadBitFlipsCnt[4]", \
        "addwReadBitFlipsCnt[5]", \
        "addwReadBitFlipsCnt[6]", \
        "addwReadBitFlipsCnt[7]", \
        "addwReadBitFlipsCnt[8]", \
        "addwReadBitFlipsCnt[9]", \
        "addwReadBitFlipsCnt[10]", \
        "addwReadBitFlipsCnt[11]", \
        "addwReadBitFlipsCnt[12]", \
        "addwReadBitFlipsCnt[13]", \
        "addwReadBitFlipsCnt[14]", \
        "addwReadBitFlipsCnt[15]", \
        "addwReadBitFlipsCnt[16]", \
        "addwReadBitFlipsCnt[17]", \
        "addwReadBitFlipsCnt[18]", \
        "addwReadBitFlipsCnt[19]", \
        "addwReadBitFlipsCnt[20]", \
        "addwReadBitFlipsCnt[21]", \
        "addwReadBitFlipsCnt[22]", \
        "addwReadBitFlipsCnt[23]", \
        "addwReadBitFlipsCnt[24]", \
        "addwReadBitFlipsCnt[25]", \
        "addwReadBitFlipsCnt[26]", \
        "addwReadBitFlipsCnt[27]", \
        "addwReadBitFlipsCnt[28]", \
        "addwReadBitFlipsCnt[29]", \
        "addwReadBitFlipsCnt[30]", \
        "ddwTimeoutCnt", \
        "ddwPartialUECCCnt", \
        "ddwPartialCleanCnt", \
        "ddwReadRefreshCnt", \
        "ddwReadRetireCnt", \
}

// Presented to the FMSS as an 18 word array
typedef struct
{
    UInt32 dwReadNoECCCorrection;
    UInt32 adwReadBitFlipsCnt[FIL_MAX_ECC_CORRECTION];
    UInt32 dwReadUncorrectableECCError;
} CSStatistics;

/*****************************************************************************/
/* Exported Function Prototype of FIL                                        */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Int32        FIL_Init(void);
void         FIL_Close(void);
LowFuncTbl  *FIL_GetFuncTbl(void);
BOOL32       FIL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);
BOOL32       FIL_SetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 dwStructSize);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FIL_H_ */
