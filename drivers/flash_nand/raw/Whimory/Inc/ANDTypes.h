/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

#ifndef _AND_TYPES_H_
#define _AND_TYPES_H_

#include "WMRTypes.h"

typedef     Int32 ANDStatus;

#define ANDErrorCodeOk                              (Int32)0
#define ANDErrorCodeCleanOk                         (Int32)1
#define ANDErrorCodeHwErr                           (Int32)0x80000001
#define ANDErrorCodeUserDataErr                     (Int32)0x80000002
#define ANDErrorCodeOutOfRangeErr                   (Int32)0x80000003
#define ANDErrorCodeAlignmentErr                    (Int32)0x80000004
#define ANDErrorWriteFailureErr                     (Int32)0x80000005
#define ANDErrorCodeAllocFailErr                    (Int32)0x80000006
#define ANDErrorCodeNotSupportedErr                 (Int32)0x80000007

#define NAND_UID_RAW_PAGE_BYTES_TO_READ     (256)
#define NAND_UID_PPN_BYTES_TO_READ          (16)
#define NAND_UID_LEN_RAW                    (16)
#define NAND_UID_LEN_PPN                    (16)
//Length of PPN Firmware Version, Package Assembly Code, Controller UID, Controller HW ID
#define NAND_DEV_PARAM_LEN_PPN              (16)

typedef struct
{
    UInt8 ce;
    UInt8 cmdSize;
    UInt8 addrSize;
    UInt8 confirmCmdSize;
    UInt8 arrCmd[10];
    UInt8 arrAddr[10];
    UInt8 arrConfirmCmd[10];
    UInt8 buf[NAND_UID_RAW_PAGE_BYTES_TO_READ];
    UInt32 bytesToRead;
} GenericReadRequest;

typedef struct
{
    UInt8 ce;
    UInt8 die;
    UInt8 buf[NAND_UID_PPN_BYTES_TO_READ];
} PpnUIDReadRequest;

typedef union
{
    struct {
        UInt8 DCCYCLE;
        UInt8 tRP;
        UInt8 tREH;
        UInt8 tWP;
        UInt8 tWH;
    }sdrTimings;
    UInt8 ddr_tHALFCYCLE;
}NandTimingParams;

// ANDAddressStruct - is used to translate logical and virtual
// addresses to physical addresses
typedef struct
{
    UInt32 dwLpn;
    UInt32 dwVpn;
    UInt32 dwCS;
    UInt32 dwPpn;
    UInt32 dwColumn;
    UInt16 dwCau;
} ANDAddressStruct;

// Describe the sections of a NAND page that contain data
typedef struct
{
    UInt32 dwNumSegments;
    UInt32 dwMetaSegmentIndex;
    struct {
        UInt32 dwOffset;
        UInt32 dwLength;
    } pastrSegments[32];
    
} ANDNandLayoutStruct;

// use this structure to export all the information from the AND driver in a single buffer / file.
typedef struct 
    {
        UInt32 dwStructID;
        UInt32 dwStructureVersion;
        UInt32 dwDataSize;
        UInt32 dwIndex; 
    } ANDExportStruct;

typedef struct
{
    UInt16 bank;
    UInt16 block;
} SpecialBlockAddress; 

typedef struct
{
    UInt32 dwPhysicalCE;
    UInt32 dwPhysicalBlock;
    UInt16 wType;
} BorrowBlockAddress; 

#define VFL_VALUE_UNKNOWN (~0)
typedef enum
{
    VFLFailNone,
    VFLFailUECC,
    VFLFailErase,
    VFLFailWrite,
    VFLFailEraseTimeOut,
    VFLFailWriteTimeOut,
    VFLFailRefresh,
    VFLFailEnd = 0x7FFFFFFF
} VFLFailureMode;

#define VFL_MAX_FAILURE_REGIONS (16)
typedef struct
{
    VFLFailureMode mode;
    UInt16         wCE[VFL_MAX_FAILURE_REGIONS];
    UInt16         wPhysicalBlock[VFL_MAX_FAILURE_REGIONS];
    UInt32         dwPhysicalPage;
} VFLFailureDetails;

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
} genVFLStatistics;

#define TL_BLOCK_FREE 0x1
#define TL_BLOCK_DATA 0x2
#define TL_BLOCK_INDEX 0x4
#define TL_BLOCK_CXT 0x8
#define TL_BLOCK_DEAD 0x10
#define TL_BLOCK_DATA_STATIC 0x20
#define TL_BLOCK_UNKNOWN 0x80

typedef struct
{
    UInt8 blockType;
    UInt32 blockSizeInLbas;
    UInt32 validLbas;
} ANDFTLBlockStruct;

/*****************************************************************************/
/* Global Structure Types which should be shared by FTL, VFL, FIL            */
/*****************************************************************************/

#define FTL_TYPE_FTL 1
#define FTL_TYPE_YAFTL 2
#define FTL_TYPE_SFTL 3

#define VFL_TYPE_VFL 1
#define VFL_TYPE_VSVFL 2
#define VFL_TYPE_PPNVFL 3
#define VFL_TYPE_SWISSPPNVFL 4


#define AND_EXPORT_STRUCTURE_VERSION 0x00020001
#define AND_EXPORT_STRUCTURE_MAJVER(v) (((v) >> 16) & 0xFFFF)
#define AND_EXPORT_STRUCTURE_MINVER(v) (((v) >>  0) & 0xFFFF)
#define AND_EXPORT_STRUCTURE_IS_1_X(v) (1 == AND_EXPORT_STRUCTURE_MAJVER(v))
#define AND_EXPORT_STRUCTURE_IS_2_X(v) (2 == AND_EXPORT_STRUCTURE_MAJVER(v))

#define AND_STATISTICS_SIZE_PER_LAYER   (0x200)

#define LOCKDOWN_EXTRACT_ALL  (0xFE000000 | 0x00000100)
#define LOCKDOWN_GET_ALL_SIZE (0xFE000000 | 0x00000200)

// CtrlIO defines and structures
// defines for data structues queries from user applications
#define AND_STRUCT_SIZE_MASK                (0x80000000) // MSb indicates we're requesting the size
#define AND_STRUCT_LAYER_MASK               (0x3F000000) // use with & to get the layer struct type
#define AND_STRUCT_INDEX_MASK               (0xFFFFFF00) // use with & to clear the index (LSB)
#define AND_STRUCT_CS_MASK                  (0x000000FF) // use with & to clear the CS
#define AND_STRUCT_SET                      (0x40000000) // Use this bit to indicate a set operation
#define AND_STRUCT_LAYER_OS                 (0x0A000000)

#define AND_STRUCT_LAYER_FTL                (0x01000000)
#define AND_STRUCT_LAYER_VFL                (0x02000000)
#define AND_STRUCT_LAYER_FIL                (0x03000000)
#define AND_STRUCT_LAYER_WMR                (0x04000000)
#define AND_STRUCT_LAYER_FUNCTIONAL         (0x10000000)
// Operations
#define _AND_FUNCTION_COMPLETE_EPOCH_UPDATE (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000001) // deprecated
#define AND_FUNCTION_INDEX_CACHE_UPDATE     (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000200)
#define AND_FUNCTION_CHANGE_FTL_TYPE        (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000300)
#define AND_FUNCTION_CHANGE_NUM_OF_SUBLKS   (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000400)
#define AND_FUNCTION_SET_BURNIN_CODE        (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000500)
#define AND_FUNCTION_ERASE_SIGNATURE        (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000600)
#define AND_FUNCTION_SET_POWER_MODE         (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000700)
#define AND_FUNCTION_NEURALIZE              (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000800)
#define AND_FUNCTION_SET_TIMINGS            (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000900)
#define AND_FUNCTION_SAVE_STATS             (AND_STRUCT_LAYER_FUNCTIONAL | 0x00000A00)

// the below is defined in ANDExport.h
#define AND_STRUCT_LAYER_GETALL             (0x30000000)

#define AND_STRUCT_WMR_EXPORT_ALL           (AND_STRUCT_LAYER_GETALL | 0x00000100)
#define AND_STRUCT_WMR_EXPORT_ALL_GET_SIZE  (AND_STRUCT_LAYER_GETALL | 0x00000200)
#define AND_STRUCT_WMR_EXPORT_ALL_END       (AND_STRUCT_LAYER_GETALL | 0x0000FF00)

// use the LSB for indexing (do not use it for the identifiers)
// OS structures
#define AND_STRUCT_OS_GET_IOS                   (AND_STRUCT_LAYER_OS  | 0x00000100)
#define AND_STRUCT_OS_GET_HIST                  (AND_STRUCT_LAYER_OS  | 0x00000200)
#define AND_STRUCT_OS_SET_LOG_TAG               (AND_STRUCT_LAYER_OS  | 0x00000300)
#define AND_STRUCT_OS_SET_GMT_TAG               (AND_STRUCT_LAYER_OS  | 0x00000400)

// FTL structures
#define AND_STRUCT_FTL_STATISTICS               (AND_STRUCT_LAYER_FTL | 0x00000200)
#define AND_STRUCT_FTL_FTLCXT                   (AND_STRUCT_LAYER_FTL | 0x00000300)
#define AND_STRUCT_FTL_MAPTABLE                 (AND_STRUCT_LAYER_FTL | 0x00000400)
#define AND_STRUCT_FTL_LOGCXTTABLE              (AND_STRUCT_LAYER_FTL | 0x00000500)
#define AND_STRUCT_FTL_ECTABLE                  (AND_STRUCT_LAYER_FTL | 0x00000600)
#define AND_STRUCT_FTL_RCTABLE                  (AND_STRUCT_LAYER_FTL | 0x00000700)
#define AND_STRUCT_FTL_GETADDRESS               (AND_STRUCT_LAYER_FTL | 0x00000800)
#define AND_STRUCT_FTL_GET_TYPE                 (AND_STRUCT_LAYER_FTL | 0x00000900)
#define AND_STRUCT_FTL_GET_FTL_STATS_SIZE       (AND_STRUCT_LAYER_FTL | 0x00000A00)
#define AND_STRUCT_FTL_GET_FTL_DEVICEINFO_SIZE  (AND_STRUCT_LAYER_FTL | 0x00000B00)
#define AND_STRUCT_FTL_GET_FTL_DEVICEINFO       (AND_STRUCT_LAYER_FTL | 0x00000C00)
#define AND_STRUCT_FTL_GET_FTL_EC_BINS          (AND_STRUCT_LAYER_FTL | 0x00000D00)
#define AND_STRUCT_FTL_GET_FTL_RC_BINS          (AND_STRUCT_LAYER_FTL | 0x00000E00)
#define AND_STRUCT_FTL_RECOMMEND_CONTENT_DELETION (AND_STRUCT_LAYER_FTL | 0x00000F00)
#define AND_STRUCT_FTL_IDEAL_SIZE_1K            (AND_STRUCT_LAYER_FTL | 0x00001000)
#define AND_STRUCT_FTL_GET_FTL_BLOCK_COUNT     (AND_STRUCT_LAYER_FTL | 0x00001100) 
#define AND_STRUCT_FTL_GET_FTL_BLOCK_STAT       (AND_STRUCT_LAYER_FTL | 0x00001200)
#define AND_STRUCT_FTL_L2V_STRUCT               (AND_STRUCT_LAYER_FTL | 0x00001300)
#define AND_STRUCT_FTL_L2V_ROOTS                (AND_STRUCT_LAYER_FTL | 0x00001400)
#define AND_STRUCT_FTL_L2V_POOL                 (AND_STRUCT_LAYER_FTL | 0x00001500)
#define AND_STRUCT_FTL_SB_CYCLES                (AND_STRUCT_LAYER_FTL | 0x00001600)
#define AND_STRUCT_FTL_SB_READ_CYCLES           (AND_STRUCT_LAYER_FTL | 0x00001700)

// VFL structures
#define AND_STRUCT_VFL_MASK                 (AND_STRUCT_LAYER_VFL | 0x0000FF00)
#define AND_STRUCT_VFL_STATISTICS           (AND_STRUCT_LAYER_VFL | 0x00000200)
#define AND_STRUCT_VFL_VFLMETA              (AND_STRUCT_LAYER_VFL | 0x00000300)
#define AND_STRUCT_VFL_BBT                  (AND_STRUCT_LAYER_VFL | 0x00000400)
#define AND_STRUCT_VFL_FILSTATISTICS        (AND_STRUCT_LAYER_VFL | 0x00000500)
#define AND_STRUCT_VFL_GETADDRESS           (AND_STRUCT_LAYER_VFL | 0x00000600)
#define AND_STRUCT_VFL_GROWNBBT             (AND_STRUCT_LAYER_VFL | 0x00000700)
#define AND_STRUCT_VFL_GET_TYPE             (AND_STRUCT_LAYER_VFL | 0x00000800)
#define AND_STRUCT_VFL_STATISTICS_SIZE      (AND_STRUCT_LAYER_VFL | 0x00000900)
#define AND_STRUCT_VFL_DEVICEINFO_SIZE      (AND_STRUCT_LAYER_VFL | 0x00000A00)
#define AND_STRUCT_VFL_DEVICEINFO           (AND_STRUCT_LAYER_VFL | 0x00000B00)
#define AND_STRUCT_VFL_FTLTYPE              (AND_STRUCT_LAYER_VFL | 0x00000C00)
#define AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS    (AND_STRUCT_LAYER_VFL | 0x00000D00)
#define AND_STRUCT_VFL_BYTES_PER_PAGE       (AND_STRUCT_LAYER_VFL | 0x00000E00)
#define AND_STRUCT_VFL_PAGES_PER_SUBLK      (AND_STRUCT_LAYER_VFL | 0x00000F00)
#define AND_STRUCT_VFL_BYTES_PER_META       (AND_STRUCT_LAYER_VFL | 0x00001000)
#define AND_STRUCT_VFL_CORRECTABLE_BITS     (AND_STRUCT_LAYER_VFL | 0x00001100)
#define AND_STRUCT_VFL_CORRECTABLE_SIZE     (AND_STRUCT_LAYER_VFL | 0x00001200)
#define AND_STRUCT_VFL_NUM_OF_SUBLKS        (AND_STRUCT_LAYER_VFL | 0x00001300)
#define AND_STRUCT_VFL_NEW_FTLTYPE          (AND_STRUCT_LAYER_VFL | 0x00001400)
#define AND_STRUCT_VFL_NEW_NUM_OF_FTL_SUBLKS    (AND_STRUCT_LAYER_VFL | 0x00001500)
#define AND_STRUCT_VFL_BURNIN_CODE         (AND_STRUCT_LAYER_VFL | 0x00001600)
#define AND_STRUCT_VFL_FACTORY_BBT          (AND_STRUCT_LAYER_VFL | 0x00001700)
#define AND_STRUCT_VFL_SIGNATURE_LOCATION   (AND_STRUCT_LAYER_VFL | 0x00001800)
#define AND_STRUCT_VFL_PAGES_PER_BLOCK      (AND_STRUCT_LAYER_VFL | 0x00001900)
#define AND_STRUCT_VFL_LAST_ERROR           (AND_STRUCT_LAYER_VFL | 0x00001A00)
#define AND_STRUCT_VFL_NAND_TYPE            (AND_STRUCT_LAYER_VFL | 0x00001B00)
#define AND_STRUCT_VFL_VALID_BYTES_PER_META (AND_STRUCT_LAYER_VFL | 0x00001C00)
#define AND_STRUCT_VFL_FIND_BORROWED_BLOCKS (AND_STRUCT_LAYER_VFL | 0x00001D00)
#define AND_STRUCT_VFL_EXPORTED_LBA_NO      (AND_STRUCT_LAYER_VFL | 0x00001E00)
#define AND_STRUCT_VFL_EXPORTED_L2V_POOL    (AND_STRUCT_LAYER_VFL | 0x00001F00)
#define AND_STRUCT_VFL_PUBLIC_FBBT          (AND_STRUCT_LAYER_VFL | 0x00002000)
#define AND_STRUCT_VFL_NUM_CHANNELS         (AND_STRUCT_LAYER_VFL | 0x00002100)
#define AND_STRUCT_VFL_CES_PER_CHANNEL      (AND_STRUCT_LAYER_VFL | 0x00002200)
#define AND_STRUCT_VFL_CAUS_PER_CE          (AND_STRUCT_LAYER_VFL | 0x00002300)
#define AND_STRUCT_VFL_MAX_TRANS_IN_PAGES   (AND_STRUCT_LAYER_VFL | 0x00002400)

// FIL structures
#define AND_STRUCT_FIL_MASK                 (AND_STRUCT_LAYER_FIL | 0x0000FF00) // LSB used for bank info
#define AND_STRUCT_FIL_STATISTICS           (AND_STRUCT_LAYER_FIL | 0x00000200) // LSB used for bank info
#define AND_STRUCT_FIL_READPAGE             (AND_STRUCT_LAYER_FIL | 0x00000300) // LSB used for bank info
#define AND_STRUCT_FIL_STATISTICS_SIZE      (AND_STRUCT_LAYER_FIL | 0x00000400) // LSB used for bank info
#define AND_STRUCT_FIL_READTIMING           (AND_STRUCT_LAYER_FIL | 0x00000500)
#define AND_STRUCT_FIL_WRITETIMING          (AND_STRUCT_LAYER_FIL | 0x00000600)
#define AND_STRUCT_FIL_CHIPID               (AND_STRUCT_LAYER_FIL | 0x00000700)
#define AND_STRUCT_FIL_CHANNEL_BITMAP       (AND_STRUCT_LAYER_FIL | 0x00000800)
#define AND_STRUCT_FIL_UNIQUEID             (AND_STRUCT_LAYER_FIL | 0x00000900)
#define AND_STRUCT_FIL_HYNIX_BBT            (AND_STRUCT_LAYER_FIL | 0x00000A00)
#define AND_STRUCT_FIL_POWER_MODE           (AND_STRUCT_LAYER_FIL | 0x00000B00)
#define AND_STRUCT_FIL_NAND_LAYOUT          (AND_STRUCT_LAYER_FIL | 0x00000C00)
#define AND_STRUCT_FIL_SPARE_SIZE           (AND_STRUCT_LAYER_FIL | 0x00000D00)
#define AND_STRUCT_FIL_BITS_PER_CELL        (AND_STRUCT_LAYER_FIL | 0x00000E00)
#define AND_STRUCT_FIL_DEVICE_INFO          (AND_STRUCT_LAYER_FIL | 0x00000F00)
#define AND_STRUCT_FIL_DIES_PER_CE          (AND_STRUCT_LAYER_FIL | 0x00001000)
#define AND_STRUCT_FIL_BLOCKS_PER_CE        (AND_STRUCT_LAYER_FIL | 0x00001100)
#define AND_STRUCT_FIL_SET_TIMINGS          (AND_STRUCT_LAYER_FIL | 0x00001200)
#define AND_STRUCT_FIL_GET_TIMINGS          (AND_STRUCT_LAYER_FIL | 0x00001300)
#define AND_STRUCT_FIL_GET_CE_INFO          (AND_STRUCT_LAYER_FIL | 0x00001400)
#define AND_STRUCT_FIL_MFG_ID               (AND_STRUCT_LAYER_FIL | 0x00001500)
#define AND_STRUCT_FIL_FW_VERSION           (AND_STRUCT_LAYER_FIL | 0x00001600)
#define AND_STRUCT_FIL_PKG_ASSEMBLY_CODE    (AND_STRUCT_LAYER_FIL | 0x00001700)
#define AND_STRUCT_FIL_CONTROLLER_UID       (AND_STRUCT_LAYER_FIL | 0x00001800)
#define AND_STRUCT_FIL_CONTROLLER_HW_ID     (AND_STRUCT_LAYER_FIL | 0x00001900)
#define AND_STRUCT_FIL_LBAS_PER_PAGE        (AND_STRUCT_LAYER_FIL | 0x00001A00)

// WMR structures
#define AND_STRUCT_WMR_VERSION              (AND_STRUCT_LAYER_WMR | 0x00000100)
#define AND_STRUCT_WMR_DEVICEINFO           (AND_STRUCT_LAYER_WMR | 0x00000200)
#define AND_STRUCT_WMR_LAYOUT               (AND_STRUCT_LAYER_WMR | 0x00000300)

// defines for device info types, these might move to an h file shared by all layers
#define     AND_DEVINFO_PAGES_PER_BLOCK         (100)
#define     AND_DEVINFO_NUM_OF_CS               (101)
#define     AND_DEVINFO_BBT_TYPE                (102)
#define     AND_DEVINFO_BLOCKS_PER_CS           (103)
#define     AND_DEVINFO_BYTES_PER_PAGE          (104)
#define     AND_DEVINFO_BYTES_PER_SPARE         (105)
#define     AND_DEVINFO_USER_BLOCKS_PER_CS      (106)
#define     AND_DEVINFO_VENDOR_SPECIFIC_TYPE    (107)
#define     AND_DEVINFO_REFRESH_THRESHOLD       (108)
#define     AND_DEVINFO_BANKS_PER_CS            (109)
#define     AND_DEVINFO_PAGES_PER_SUBLK         (110)
#define     AND_DEVINFO_NUM_OF_BANKS            (111)
#define     AND_DEVINFO_NUM_OF_USER_SUBLK       (112)
#define     AND_DEVINFO_FTL_TYPE                (113)
#define     AND_DEVINFO_BYTES_PER_BL_PAGE       (114)
#define     AND_DEVINFO_CORRECTABLE_BITS        (115)
#define     AND_DEVINFO_CORRECTABLE_SIZE        (116)
#define     AND_DEVINFO_FTL_NEED_FORMAT         (117)
#define     AND_DEVINFO_DIES_PER_CS             (118)
#define     AND_DEVINFO_BLOCKS_PER_DIE          (119)
#define     AND_DEVINFO_DIE_STRIDE              (120)
#define     AND_DEVINFO_BLOCK_STRIDE            (121)
#define     AND_DEVINFO_LAST_BLOCK              (122)
#define     AND_DEVINFO_BLOCKS_PER_CAU          (123)
#define     AND_DEVINFO_CAUS_PER_CE             (124)
#define     AND_DEVINFO_SLC_PAGES_PER_BLOCK     (125)
#define     AND_DEVINFO_MLC_PAGES_PER_BLOCK     (126)
#define     AND_DEVINFO_MATCH_ODDEVEN_CAUS      (127)
#define     AND_DEVINFO_BITS_PER_CAU_ADDRESS    (128)
#define     AND_DEVINFO_BITS_PER_PAGE_ADDRESS   (130)
#define     AND_DEVINFO_BITS_PER_BLOCK_ADDRESS  (131)
#define     AND_DEVINFO_NUM_OF_CHANNELS         (132)
#define     AND_DEVINFO_NUM_OF_CES_PER_CHANNEL  (133)
#define     AND_DEVINFO_PPN_DEVICE              (134)
#define     AND_DEVINFO_FIL_LBAS_PER_PAGE       (135)
#define     AND_DEVINFO_FIL_META_VALID_BYTES    (136)
#define     AND_DEVINFO_FIL_META_BUFFER_BYTES   (137)
#define     AND_DEVINFO_FIL_MATCH_ODD_EVEN_BLKS (138)
#define     AND_DEVINFO_FIL_PREP_BUFFER_ENTRIES (139)
#define     AND_DEVINFO_VFL_AVAILABLE_VBAS      (140)
#define     AND_DEVINFO_ADDR_BITS_BITS_PER_CELL (141)
#define     AND_DEVINFO_TOGGLE_DEVICE           (142)
#define     AND_DEVINFO_L2V_POOL_SIZE           (143)
#define     AND_DEVINFO_STREAM_BUFFER_MAX       (144)


// format params
#define     WMR_INIT_DISABLE_FORMAT         (0x00000000) // default init value
#define     WMR_INIT_ALLOW_FORMAT           (0x00000001) // allow normal unit format
#define     WMR_INIT_RUN_PRODUCTION_FORMAT  (0x00000002) // call production format
#define     WMR_INIT_ALLOW_VSVFL            (0x00000004) // allow VSVFL in case we format
#define     WMR_INIT_ALLOW_YAFTL            (0x00000008) // allow VSVFL in case we format
#define     WMR_INIT_RUN_FTL_FORMAT         (0x00000010) // call ftl format and change the FTL size
#define     WMR_INIT_SIGNATURE_IN_BLOCKZERO (0x00000020) // signature in block zero
#define     WMR_INIT_LOW_POWER_MODE         (0x00000040) // driver should use minimum power until notified
#define     WMR_INIT_NAND_RESTORE           (0x00000080) // force full nand restore during power up
#define     WMR_INIT_USE_DEV_UNIQUE_INFO    (0x00000100) // allocate a partition for device unique info
#define     WMR_INIT_USE_DIAG_CTRL_INFO     (0x00000200) // allocate a partition for diag control bits
#define     WMR_INIT_METADATA_WHITENING     (0x00000400) // device should be formatted with metadata whitening
#define     WMR_INIT_METADATA_WHITENING_CHECK (0x00000800) // device should be formatted with metadata whitening if it already isn't
#define     _WMR_INIT_ALLOW_ANY_EPOCH       (0x00001000) // driver will not fail security epoch check (deprecated)
#define     WMR_INIT_DISABLE_GC_IN_TRIM     (0x00002000) // disable empty GC in trim
#define     WMR_INIT_SET_INDEX_CACHE_SIZE   (0x00004000) // disable empty GC in trim
#define     WMR_INIT__USE_NEW_DUAL_CHANNEL_ADDRESS (0x00008000) // rdar://9159270
#define     WMR_INIT_OPEN_READONLY          (0x00010000) // Disable writes/erase during open rdar://9407663
#define     WMR_INIT_USE_KEEPOUT_AS_BORROW  (0x00020000)
#ifdef AND_SIMULATOR
#define     WMR_INIT_IGNORE_ERASE_GAP       (0x00040000)
#endif

// flags for external VFL calls
#define     AND_EXTERNAL_VFL_TEST_PARTITION (0x00000000)
#define     AND_EXTERNAL_VFL_FTL_PARTITION  (0x00000001)
#define     AND_EXTERNAL_VFL_FULL           (0x00000002)

// factory test conditions
#define     VFL_BONFIRE_UNTESTED_CODE       (0)
#define     VFL_BONFIRE_RMA_CODE            (306)

// FIL defines
typedef enum
{
    INIT_BBT_TOSHIBA_MLC = 1,
    INIT_BBT_SAMSUNG_MLC = 2,
    INIT_BBT_HYNIX_MLC = 3,
    INIT_BBT_SAMSUNG_SLC = 4,
    INIT_BBT_INTEL_ONFI = 5,
    INIT_BBT_ONFI = 6,
    INIT_BBT_MICRON_ONFI = 7,
    INIT_BBT_SAMSUNG_MLC_8K = 8,
    INIT_BBT_SANDISK_MLC = 9,
    INIT_BBT_PPN = 10,
    INIT_BBT_BOGUS = 0x0FFFFFFF, // Force 4-byte enum field
} CheckInitialBadType;

// format definitions
#define VS_FMT_UNKNOWN                      0x0000
#define VS_FMT_SIMPLE                       0x0001
#define VS_FMT_TOSHIBA_TWO_DISTRICT         0x0010
#define VS_FMT_TOSHIBA_TWO_DISTRICT_EXT     0x0011
#define VS_FMT_2BANKS_MSB                   0x0012
#define VS_FMT_4BANKS_LSB_MSB               0x0013
#define VS_FMT_2BANKS_LSB                   0x0014
#define VS_FMT_TOSHIBA_TWO_DIE              0x0015
#define VS_FMT_4BANKS_LSB_MSB_NOPWR2        0x0016

// command definitions
#define VS_CMD_UNKNOWN                      0x0000
#define VS_CMD_SIMPLE                       0x0001
#define VS_CMD_SAMSUNG_2P                   0x0010
#define VS_CMD_ONFI_2D                      0x0011
#define VS_CMD_ONFI_2P                      0x0012
#define VS_CMD_ONFI_2D_CACHE                0x0013
#define VS_CMD_ONFI_2P_CACHE                0x0014
#define VS_CMD_TOSHIBA_2P                   0x0015
#define VS_CMD_TOSHIBA_2P_CACHE             0x0016
#define VS_CMD_TOSHIBA_2D                   0x0017

typedef UInt32 VendorSpecificType;

#define VS_TYPE(cmd, fmt)                   (UInt32)(((UInt32)(cmd) << 16) | ((UInt32)(fmt) & 0x0000FFFF))
#define VS_GET_CMD(vs)                      (((UInt32)(vs) >> 16))
#define VS_GET_FMT(vs)                      (((UInt32)(vs) & 0x0000FFFF))

#define FIL_VS_UNKNOWN                      VS_TYPE(VS_CMD_UNKNOWN, VS_FMT_UNKNOWN)
#define FIL_VS_SIMPLE                       VS_TYPE(VS_CMD_SIMPLE, VS_FMT_SIMPLE)
// this is to make the current tables work...
#define FIL_VS_HYNIX_2P                     VS_TYPE(VS_CMD_SAMSUNG_2P, VS_FMT_2BANKS_LSB)
#define FIL_VS_ONFI_2D                      VS_TYPE(VS_CMD_ONFI_2D, VS_FMT_2BANKS_MSB)
#define FIL_VS_ONFI_2P                      VS_TYPE(VS_CMD_ONFI_2P, VS_FMT_2BANKS_LSB)
#define FIL_VS_ONFI_2P_2D                   VS_TYPE(VS_CMD_ONFI_2P, VS_FMT_4BANKS_LSB_MSB)
#define FIL_VS_ONFI_2D_CACHE                VS_TYPE(VS_CMD_ONFI_2D_CACHE, VS_FMT_2BANKS_MSB)
#define FIL_VS_ONFI_2P_CACHE                VS_TYPE(VS_CMD_ONFI_2P_CACHE, VS_FMT_2BANKS_LSB)
#define FIL_VS_ONFI_2P_2D_CACHE             VS_TYPE(VS_CMD_ONFI_2P_CACHE, VS_FMT_4BANKS_LSB_MSB)
#define FIL_VS_SAMSUNG_2D                   VS_TYPE(VS_CMD_SIMPLE, VS_FMT_2BANKS_MSB)
#define FIL_VS_SAMSUNG_2P_2D                VS_TYPE(VS_CMD_SIMPLE, VS_FMT_4BANKS_LSB_MSB)
#define FIL_VS_SAMSUNG_2P_2D_EXT            VS_TYPE(VS_CMD_SIMPLE, VS_FMT_4BANKS_LSB_MSB_NOPWR2)
#define FIL_VS_TOSHIBA_2P                   VS_TYPE(VS_CMD_TOSHIBA_2P, VS_FMT_TOSHIBA_TWO_DISTRICT)
#define FIL_VS_TOSHIBA_2P_EXT               VS_TYPE(VS_CMD_TOSHIBA_2P, VS_FMT_TOSHIBA_TWO_DISTRICT_EXT)
#define FIL_VS_TOSHIBA_2P_CACHE             VS_TYPE(VS_CMD_TOSHIBA_2P_CACHE, VS_FMT_TOSHIBA_TWO_DISTRICT)
#define FIL_VS_TOSHIBA_2P_EXT_CACHE         VS_TYPE(VS_CMD_TOSHIBA_2P_CACHE, VS_FMT_TOSHIBA_TWO_DISTRICT_EXT)
#define FIL_VS_TOSHIBA_2D                   VS_TYPE(VS_CMD_TOSHIBA_2D, VS_FMT_TOSHIBA_TWO_DIE)

//CE status report definitions
#define CE_STATUS_UNINITIALIZED                 ((UInt16)~0)
#define PAGE_STATUS_UNINITIALIZED               ((UInt32)~0)

//Enumeration for blocks that have failed erase
#define BLOCK_STATUS_UNINITIALIZED              ((UInt32)0xFFFFFFFF)

// =============================================================================
// Power Mode
// To be used with AND_FUNCTION_SET_POWER_MODE & AND_STRUCT_FIL_POWER_MODE
#define NAND_POWER_MODE_UNKNOWN    ((UInt32) 0)
#define NAND_POWER_MODE_FULL_SPEED ((UInt32) 1)
#define NAND_POWER_MODE_SINGLE_CE  ((UInt32) 2)
#define NAND_POWER_MODE_STANDBY    ((UInt32) 3)


#define NAND_DATA_REGION_SYSCFG       ((UInt32)1)
#define NAND_DATA_REGION_DIAG_CONTROL ((UInt32)0)

// Special Blcok Types - PPN VFL / FPart
// Bit allocations:
// [0-7]  - code (uniquue per special block type - valid range is 0x00 - 0xEF
//          leaving the top to VFL Context
// [8-13] - handler (how the special block shold be procesed - handler can be either supported 
//          by FPart or other layers. handler handled by other layers can not be persistent.
// [14] - persistent flag (1 - persistent, 0 - not persistent)
// [15] - special bit flag (1 - special block, 0 - block is allocated to virtual block exported by vfl)

#define AND_SB_BIT_SPEICAL_FLAG     (1 << 15) // if this bit is set the block is 
                                               // NOT allocated to VBN
                                               // It is either a special block or 
                                               // VFL context block

#define AND_SB_BIT_PERSISTENT_FLAG  (1 << 14) // persistent type (need to survive loosing the signature)

#define andSBGetHandler(_x)            ((UInt16)(((_x) & 0x3F00) >> 8))
#define andSBSetHandler(_x)            ((UInt16)(((_x) & 0x3F) << 8))

#define AND_SB_HANDLER_SINGLE_VERSION  (1)
#define AND_SB_HANDLER_MULTI_VERSION   (2)
#define AND_SB_HANDLER_NAND_BOOT       (0x30)
#define AND_SB_HANDLER_VFL_CXT         (0x3F)


#define andSBGetCode(_x)               ((UInt16)((_x) & 0x00FF))
#define andSBSetCode(_x)               ((UInt16)((_x) & 0x00FF))


#define AND_SB_CODE_DRIVER_SIGNATURE   (0x01)
#define AND_SB_CODE_BLOCK_ZERO         (0x02)
#define AND_SB_CODE_NAND_BOOT          (0x03)
#define AND_SB_CODE_DIAGNOSTICS        (0x04)
#define AND_SB_CODE_UNIQUE_INFO        (0x05)
#define AND_SB_CODE_KEEPOUT            (0x06)

#define AND_SB_TYPE_DRIVER_SIGNATURE (AND_SB_BIT_SPEICAL_FLAG | AND_SB_BIT_PERSISTENT_FLAG | andSBSetHandler(AND_SB_HANDLER_SINGLE_VERSION) | andSBSetCode(AND_SB_CODE_DRIVER_SIGNATURE))
#define AND_SB_TYPE_BLOCK_ZERO       (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_NAND_BOOT) | andSBSetCode(AND_SB_CODE_BLOCK_ZERO))
#define AND_SB_TYPE_NAND_BOOT        (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_NAND_BOOT) | andSBSetCode(AND_SB_CODE_NAND_BOOT))
#define AND_SB_TYPE_DIAGNOSTICS      (AND_SB_BIT_SPEICAL_FLAG | AND_SB_BIT_PERSISTENT_FLAG | andSBSetHandler(AND_SB_HANDLER_SINGLE_VERSION) | andSBSetCode(AND_SB_CODE_DIAGNOSTICS))
#define AND_SB_TYPE_UNIQUE_INFO      (AND_SB_BIT_SPEICAL_FLAG | AND_SB_BIT_PERSISTENT_FLAG | andSBSetHandler(AND_SB_HANDLER_SINGLE_VERSION) | andSBSetCode(AND_SB_CODE_UNIQUE_INFO))
#define AND_SB_TYPE_KEEPOUT_BOOT     (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_NAND_BOOT) | andSBSetCode(AND_SB_CODE_KEEPOUT))

#define NAND_TYPE_RAW (0)
#define NAND_TYPE_PPN (1)

#define AND_SPARE_TYPE_REGION_FTL      (0x00) // allowing FTL to use 5 bits for types
#define AND_SPARE_TYPE_REGION_VFL      (0x20) // allowing VFL to use 4 bits for types
#define AND_SPARE_TYPE_REGION_FPART    (0x30) // allowing FPART to use 4 bits for types
#define AND_SPARE_TYPE_REGION_BFN      (0x40) // allowing BFN to use 4 bits for types

// Get address special cases definitions
#define AND_GET_ADDRESS_SPECIAL        (0xFFFFFF80)
#define AND_GET_ADDRESS_TRIMMED        (AND_GET_ADDRESS_SPECIAL + 1)
#define AND_GET_ADDRESS_OUT_OF_RANGE   (AND_GET_ADDRESS_SPECIAL + 2)
#define AND_GET_ADDRESS_CACHED_WRITE   (AND_GET_ADDRESS_SPECIAL + 3)
#define AND_GET_ADDRESS_UNINITIALIZED  (0xFFFFFFFF)

#define AND_L2V_POOL_SIZE_UNSPECIFIED  (0XFFFFFFFF)

#endif /* _AND_TYPES_H_ */

