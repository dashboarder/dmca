// *****************************************************************************
//
// File: H2fmi.h
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#ifndef _H2FMI_H_
#define _H2FMI_H_

#include <WMRTypes.h>

// =============================================================================
// public interface function declarations
// =============================================================================

Int32 h2fmiInit(void);
void h2fmiPrintConfig(void);
BOOL32 h2fmi_ppn_fil_init(void);
void h2fmi_ppn_recover_nand(void); //PPN

void h2fmiSetDeviceInfo(UInt32 dwParamType, UInt32 dwVal);
UInt32 h2fmiGetDeviceInfo(UInt32 dwParamType);
void h2fmiSetWhiteningState(BOOL32 enable);
void h2fmiSetWhiteningMetadataState(BOOL32 enable);

void h2fmiReset(void);

Int32 h2fmiReadNoECC(UInt16 wBank, UInt32 dwPpn, UInt8* pabDataBuf, UInt8* pabSpareBuf);

Int32 h2fmiReadSequentialPages(UInt32* padwPpn, UInt8* pabDataBuf, UInt8* pabMetaBuf, UInt16 wNumOfPagesToRead, UInt8* pbMaxCorrectedBits, UInt8* pdwaSectorStats, BOOL32 bDisableWhitening);
Int32 h2fmiReadScatteredPages(UInt16* pawBanks, UInt32* padwPpn, UInt8* pabDataBuf, UInt8* pabMetaBuf, UInt16 wNumOfPagesToRead, UInt8* pbMaxCorrectedBits, UInt8* pdwaSectorStats, BOOL32 bDisableWhitening);
Int32 h2fmiReadSinglePage(UInt16 wBank, UInt32 dwPpn, UInt8* pabDataBuf, UInt8* pabMetaBuf, UInt8* pbMaxCorrectedBits, UInt8* pdwaSectorStats, BOOL32 bDisableWhitening);
Int32 h2fmiReadSinglePageMaxECC(UInt16 wBank, UInt32 dwPpn, UInt8* pabDataBuf);
Int32 h2fmiPrintParameters(void);

Int32    h2fmiWriteMultiplePages(UInt32* padwPpn, UInt8* pabDataBuf, UInt8* pabMetaBuf, UInt16 wNumOfPagesToWrite, BOOL32 boolAligned, BOOL32 boolCorrectBank, UInt16* pwFailingCE, BOOL32 bDisableWhitening, UInt32 *pwFailingPageNum);
Int32    h2fmiWriteScatteredPages(UInt16 *pawBanks, UInt32 *padwPpn, UInt8 *pabDataBuf, UInt8 *pabMetaBuf, UInt16 wNumOfPagesToWrite, UInt16 *pawFailingCE, BOOL32 bDisableWhitening, UInt32 *pwFailingPageNum);
Int32 h2fmiWriteSinglePage(UInt16 wBank, UInt32 dwPpn, UInt8* pabDataBuf, UInt8* pabMetaBuf, BOOL32 bDisableWhitening);
Int32 h2fmiWriteSinglePageMaxECC(UInt16 wBank, UInt32 dwPpn, UInt8* pabDataBuf);

Int32 h2fmiEraseSingleBlock(UInt16 wBank, UInt32 wPbn);
void     h2fmiCalcCurrentTimings(void);
BOOL32   h2fmiGetStruct(UInt32 dwStructType, void* pvoidStructBuffer, UInt32* pdwStructSize);
BOOL32   h2fmiSetStruct(UInt32 dwStructType, void* pvoidStructBuffer, UInt32 pdwStructSize);
void    h2fmiRegisterCurrentTransaction(UInt64 ddwLba, UInt32 dwNum, void* pDest);

Int32 h2fmiWriteBootpage(UInt16 wCS, UInt32 dwPpn, UInt8 *pabData);
Int32 h2fmiReadBootpage(UInt16 wCS, UInt32 dwPpn, UInt8* pabData);
Int32 h2fmiReadBootpagePio(UInt16 wCS, UInt32 dwPpn, UInt8* pabData);


#endif // _H2FMI_H_

// ********************************** EOF **************************************
