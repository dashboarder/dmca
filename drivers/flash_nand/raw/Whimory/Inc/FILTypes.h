#ifndef _FIL_TYPES_H_
#define _FIL_TYPES_H_

#include "ANDTypes.h"

// physical device info (replacing stDeviceInfo)
typedef struct
{
    UInt32 dwDevID;                            /* Device ID				 */
    UInt16 wNumOfCS;                           /* number of CS				 */
    UInt16 wBlocksPerCS;                       /* Number of Blocks			 */
    UInt16 wPagesPerBlock;                     /* Pages per block			 */
    UInt16 wSectorsPerPage;                    /* Sectors per page			 */
    UInt16 wSpareBytesPerPage;                 /* Spare bytes per page		 */
    UInt16 wBanksPerCS;
    UInt16 wDiesPerCS;
    UInt16 wBlocksPerDie;
    UInt16 wDieStride;
    UInt16 wBlockStride;
    UInt16 wLastBlock;
    UInt16 wUserBlocksPerCS;

    CheckInitialBadType checkInitialBadType;
    VendorSpecificType vendorSpecificType;
    UInt8 bECCThresholdBits;
    UInt8 bECCCorrectableBits;

    // timing
    UInt8 bWriteCycleNanosecs;
    UInt8 bWriteSetupNanosecs;
    UInt8 bWriteHoldNanosecs;

    // read cycle
    UInt8 bReadCycleNanosecs;
    UInt8 bReadSetupNanosecs;
    UInt8 bReadHoldNanosecs;

    // read window
    UInt8 bReadDelayNanosecs;
    UInt8 bReadValidNanosecs;

    // Additions, to keep above footprint the same
    UInt32 wNumOfBusses;                       /* number of busses           */
    UInt32 dwBlocksPerCAU;
    UInt32 dwCAUsPerCE;
    UInt32 dwSLCPagesPerBlock;
    UInt32 dwMLCPagesPerBlock;
    UInt32 dwMatchOddEvenCAUs;
    UInt32 dwBitsPerCAUAddress;
    UInt32 dwBitsPerPageAddress;
    UInt32 dwBitsPerBlockAddress;
    UInt32 ppn;
    UInt32 dwPpnVersion;
    UInt32 toggle;
    UInt32 dwValidMetaPerLogicalPage;
    UInt32 dwTotalMetaPerLogicalPage;
    UInt32 dwLogicalPageSize;
    UInt32 dwMaxTransactionSize;
    UInt8  bAddrBitsForBitsPerCell;
} FILDeviceInfo;

#endif /* _FIL_TYPES_H_ */
