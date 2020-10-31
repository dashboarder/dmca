/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#ifndef _SDIO_CIS_H
#define _SDIO_CIS_H


#ifdef PLATFORM_VARIANT_IOP
#include <sdiocommon/sdio_types.h>
#include <sys/types.h>
#else
#include <IOKit/sdio/sdio_types.h>
#include <IOKit/IOTypes.h>
#endif


#ifdef __cplusplus
extern "C"  {
#endif

enum SDIOCISTupleCode {
	kSDIOCISTupleNull		= 0x00,
	kSDIOCISTupleChecksum	= 0x10,
	kSDIOCISTupleVers1		= 0x15,
	kSDIOCISTupleManfID		= 0x20,
	kSDIOCISTupleFuncID		= 0x21,
	kSDIOCISTupleFuncE		= 0x22,
	
	kSDIOCISTupleSDIOStd	= 0x91,
	kSDIOCISTupleSDIOExt	= 0x92,
	kSDIOCISTupleEnd		= 0xFF,
	
	kSDIOCISTupleVendor80	= 0x80,
	kSDIOCISTupleVendor81	= 0x81,
	kSDIOCISTupleVendor82	= 0x82,
	kSDIOCISTupleVendor83	= 0x83,
	kSDIOCISTupleVendor84	= 0x84,
	kSDIOCISTupleVendor85	= 0x85,
	kSDIOCISTupleVendor86	= 0x86,
	kSDIOCISTupleVendor87 	= 0x87,
	kSDIOCISTupleVendor88	= 0x88,
	kSDIOCISTupleVendor89	= 0x89,
	kSDIOCISTupleVendor8A	= 0x8A,
	kSDIOCISTupleVendor8B	= 0x8B,
	kSDIOCISTupleVendor8C	= 0x8C,
	kSDIOCISTupleVendor8D	= 0x8D,
	kSDIOCISTupleVendor8E	= 0x8E,
	kSDIOCISTupleVendor8F	= 0x8F,
};
	
enum SDIOCISTupleHeaderSize
{
	kSDIOCISTupleSpecialHeaderSize  = 1 * sizeof(UInt8),
	kSDIOCISTupleStandardHeaderSize = 2 * sizeof(UInt8),
};


struct SDIOCISTupleChecksum
{
	UInt16 address;
	UInt16 length;
	UInt8 checksum;
} SDIO_PACKED;

	
struct SDIOCISTupleVers1
{
	UInt8 major;
	UInt8 minor;
	UInt8 strings[0];
} SDIO_PACKED;

enum
{
	kSDIOCISTupleVers1NumStrings = 4,
	
	/** Minimum size of the Vers1 body. 
	 * This is: major + minor + NULL for each string + end of chain
	 */
	kSDIOCISTupleVers1MinBodySize = 2 + kSDIOCISTupleVers1NumStrings + 1,
};


struct SDIOCISTupleManfID
{
	UInt16 manufacturerID;
	UInt16 cardID;
} SDIO_PACKED;

struct SDIOCISTupleFuncID
{
	UInt8 functionCode;
	UInt8 sysVInitMask;
} SDIO_PACKED;

struct SDIOCISTupleFunc0E
{
	UInt8 type;
	UInt16 fn0BlockSize;
	UInt8 maxTransferSpeed;
} SDIO_PACKED;

struct SDIOCISTupleFuncE
{
	UInt8 type;
	UInt8 functionInfo;
	UInt8 stdIORev;
	UInt32 cardPSN;
	UInt32 csaSize;
	UInt8 csaProperty;
	UInt16 maxBlockSize;
	UInt32 ocr;
	UInt8 operatingMinPower;
	UInt8 operatingpAvgPower;
	UInt8 operatingMaxPower;
	UInt8 standbyMinPower;
	UInt8 standbyAvgPower;
	UInt8 standbyMaxPower;
	UInt16 minBandwidth;
	UInt16 optimalBandwidth;
	UInt16 enableTimeoutValue;
	UInt16 spAvgPower33;
	UInt16 spMaxPower33;
	UInt16 highCurrentAvgPower33;
	UInt16 highCurrentMaxPower33;
	UInt16 lowCurrentAvgPower33;
	UInt16 lowCurrentMaxPower33;
} SDIO_PACKED;

struct SDIOCISTupleStd
{
	UInt8 stdID;
	UInt8 stdType;
	UInt8 stdData[0];
} SDIO_PACKED;

struct SDIOCISTupleExt
{
	UInt8 extData[0];
} SDIO_PACKED;


struct SDIOCisTupleHeader
{
	UInt8 code;
	UInt8 link;
} SDIO_PACKED;

union SDIOCisTupleBody
{
	struct SDIOCISTupleChecksum	checksumTpl;
	struct SDIOCISTupleVers1 vers1Tpl;
	struct SDIOCISTupleManfID manfIDTpl;
	struct SDIOCISTupleFuncID funcIDTpl;
	struct SDIOCISTupleFunc0E func0ETpl;
	struct SDIOCISTupleFuncE funcETpl;
	struct SDIOCISTupleStd stdTpl;
	struct SDIOCISTupleExt extTpl;
	UInt8 byte[0];
} SDIO_PACKED;

struct SDIOCISTuple
{
	struct SDIOCisTupleHeader header;
	union SDIOCisTupleBody body;
} SDIO_PACKED;


#ifdef __cplusplus
}
#endif


#endif /* _SDIO_CIS_H */

