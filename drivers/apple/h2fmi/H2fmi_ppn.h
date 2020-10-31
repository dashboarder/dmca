// *****************************************************************************
//
// File: H2fmi_ppn.h
//
// Copyright (C) 2009 Apple Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************
#ifndef _H2FMI_PPN_H_
#define _H2FMI_PPN_H_

#include "H2fmi_private.h"
#include "H2fmi_timing.h"
#include "PPN_FIL.h"

#if defined(APPLICATION_IBOOT)
#include "nandid.h"
#elif WMR_BUILDING_EFI
#include "NandSpec.h"
#endif

#define H2FMI_PPN_VERIFY_SET_FEATURES (0)
#define PPN_MAX_FEATURE_COUNT         (32)

#define NAND_CMD__MULTIPAGE_READ                 ((UInt8)0x07)
#define NAND_CMD__MULTIPAGE_READ_LAST            ((UInt8)0x0A)
#define NAND_CMD__MULTIPAGE_READ_CONFIRM         ((UInt8)0x37)
#define NAND_CMD__MULTIPAGE_PROGRAM              ((UInt8)0x87)
#define NAND_CMD__MULTIPAGE_PROGRAM_LAST         ((UInt8)0x8A)
#define NAND_CMD__MULTIPAGE_PROGRAM_CONFIRM      ((UInt8)0x17)
#define NAND_CMD__MULTIPAGE_PREP                 ((UInt8)0xB0)
#define NAND_CMD__MULTIPAGE_PREP__CONFIRM        ((UInt8)0xB8)
#define NAND_CMD__MULTIBLOCK_ERASE               ((UInt8)0x67)
#define NAND_CMD__MULTIBLOCK_ERASE_LAST          ((UInt8)0x6A)
#define NAND_CMD__MULTIBLOCK_ERASE_CONFIRM       ((UInt8)0xD7)
#define NAND_CMD__LOW_POWER_READ_STATUS          ((UInt8)0x70)
#define NAND_CMD__GET_NEXT_OPERATION_STATUS      ((UInt8)0x77)
#define NAND_CMD__OPERATION_STATUS               ((UInt8)0x7D)
#define NAND_CMD__READ_SERIAL_OUTPUT             ((UInt8)0x7A)
#define NAND_CMD__CONTROLLER_STATUS              ((UInt8)0x79)
#define NAND_CMD__READ_DEVICE_PARAMETERS         ((UInt8)0x92)
#define NAND_CMD__READ_DEVICE_PARAMETERS_CONFIRM ((UInt8)0x97)
#define NAND_CMD__SET_FEATURES                   ((UInt8)0xEF)
#define NAND_CMD__GET_FEATURES                   ((UInt8)0xEE)
#define NAND_CMD__SET_GET_FEATURES_CONFIRM       ((UInt8)0xE7)
#define NAND_CMD__SET_DEBUG_DATA                 ((UInt8)0xE9)
#define NAND_CMD__GET_DEBUG_DATA                 ((UInt8)0xE8)
#define NAND_CMD__UPDATE_FW                      ((UInt8)0xED)
#define NAND_CMD__PPN_STATUS_CONFIRM             ((UInt8)0xED)
#define NAND_CMD__RESET                          ((UInt8)0xFF)

#define PPN_FEATURE__POWER_STATE                      ((UInt16)0x0180)
#define PPN_FEATURE__SCRATCH_PAD                      ((UInt16)0x0380)
#define PPN_FEATURE__ALLOW_SAVING_DEBUG_DATA          ((UInt16)0x0480)
#define PPN_FEATURE__IDLE_COUNTER                     ((UInt16)0x0580)
#define PPN_FEATURE__FW_UPDATE                        ((UInt16)0x1080)
#define PPN_FEATURE__FWA_UPDATE                       ((UInt16)0x1180)
#define PPN_FEATURE__DRIVE_STRENGTH                   ((UInt16)0x2080)
#define PPN_FEATURE__DEVICE_TIMING                    ((UInt16)0x2180)
#define PPN_FEATURE__PROGRAM_FAILED_PAGES             ((UInt16)0x3080)
#define PPN_FEATURE__PROGRAM_RETIRED_PAGES            ((UInt16)0x3180)
#define PPN_FEATURE__PROGRAM_IGNORED_PAGES            ((UInt16)0x3280)
#define PPN_FEATURE__CLEAR_PROGRAM_ERROR_LISTS        ((UInt16)0x3380)
#define PPN_FEATURE__DQS_COMPLEMENT_ENABLE            ((UInt16)0x4080)
#define PPN_FEATURE__RE_COMPLEMENT_ENABLE             ((UInt16)0x4180)
#define PPN_FEATURE__VREF_ENABLE                      ((UInt16)0x4280)
#define PPN_FEATURE__NUM_DIE_PROGRAM_PARALLEL         ((UInt16)0x5080)
#define PPN_FEATURE__NUM_DIE_READ_PARALLEL            ((UInt16)0x5180)
#define PPN_FEATURE__NUM_DIE_ERASE_PARALLEL           ((UInt16)0x5280)
#define PPN_FEATURE__PHYSICAL_PAGE_SIZE               ((UInt16)0x8080)
#define PPN_FEATURE__ECC_CHUNK_SIZE                   ((UInt16)0x8180)
#define PPN_FEATURE__ECC_CODE_SIZE                    ((UInt16)0x8280)
#define PPN_FEATURE__FW_VERSION                       ((UInt16)0x9080)
#define PPN_FEATURE__PACKAGE_ASSEMBLY_CODE            ((UInt16)0xA080)
#define PPN_FEATURE__CONTROLLER_UNIQUE_ID             ((UInt16)0xA180)
#define PPN_FEATURE__CONTROLLER_HW_ID                 ((UInt16)0xA181)
#define PPN_FEATURE__NAND_DIE_UNIQUE_ID               ((UInt16)0x0082)
#define PPN_FEATURE__NAND_DIE_CHIP_ID                 ((UInt16)0x0083)
#define PPN_FEATURE__BAD_BLOCK_BITMAP_ARRAY           ((UInt16)0x0084)
#define PPN_FEATURE__NAND_MARKETING_NAME              ((UInt16)0x0085)
#define PPN_FEATURE__ENABLE_BITFLIPS_DATA_COLLECTION  ((UInt16)0x0090)
#define PPN_FEATURE__ENABLE_HEALTH_MONITORING         ((UInt16)0x1090)
#define PPN_FEATURE__GET_HEALTH_MONTIORING_GRADES     ((UInt16)0x1190)
#define PPN_FEATURE__GET_DEVICE_TEMPERATURE           ((UInt16)0x2090)
#define PPN_FEATURE__SET_DEBUG_DATA_GENERIC_CONFIG    ((UInt16)0x00A0)
#define PPN_FEATURE__SET_DEBUG_DATA_VENDOR_SPECIFIC   ((UInt16)0x01A0)
#define PPN_FEATURE__DELETE_DEBUG_DATA                ((UInt16)0x08A0)

// another copy lives in iop_fmi_protocol.h (changes should be made to both places)
#define PPN_FEATURE__POWER_STATE__LOW_POWER   (0x1)
#define PPN_FEATURE__POWER_STATE__ASYNC       (0x2)
#define PPN_FEATURE__POWER_STATE__STANDBY     (0x4)
#define PPN_FEATURE__POWER_STATE__DDR         (0xA)

// Power State transition unique identifiers and accessors
// another copy lives in iop_fmi_protocol.h (changes should be made to both places)
#define PPN_PS_TRANS_LOW_POWER_TO_ASYNC      PS_TRANS_SET(PPN_FEATURE__POWER_STATE__LOW_POWER, PPN_FEATURE__POWER_STATE__ASYNC)
#define PPN_PS_TRANS_STDBY_TO_ASYNC          PS_TRANS_SET(PPN_FEATURE__POWER_STATE__STDBY, PPN_FEATURE__POWER_STATE__ASYNC)
#define PPN_PS_TRANS_ASYNC_TO_LOW_POWER      PS_TRANS_SET(PPN_FEATURE__POWER_STATE__ASYNC, PPN_FEATURE__POWER_STATE__LOW_POWER)
#define PPN_PS_TRANS_DDR_TO_LOW_POWER        PS_TRANS_SET(PPN_FEATURE__POWER_STATE__DDR, PPN_FEATURE__POWER_STATE__LOW_POWER)
#define PPN_PS_TRANS_ASYNC_TO_STDBY          PS_TRANS_SET(PPN_FEATURE__POWER_STATE__ASYNC, PPN_FEATURE__POWER_STATE__STDBY)
#define PPN_PS_TRANS_DDR_TO_STDBY            PS_TRANS_SET(PPN_FEATURE__POWER_STATE__DDR, PPN_FEATURE__POWER_STATE__STDBY)
#define PPN_PS_TRANS_LOW_POWER_TO_DDR        PS_TRANS_SET(PPN_FEATURE__POWER_STATE__LOW_POWER, PPN_FEATURE__POWER_STATE__DDR)
#define PPN_PS_TRANS_STDBY_TO_DDR            PS_TRANS_SET(PPN_FEATURE__POWER_STATE__STDBY, PPN_FEATURE__POWER_STATE__DDR)
#define PS_TRANS_SET(from, to) ((((UInt32)(from)) << 16) | ((UInt32)(to)))
#define PS_TRANS_GET_FROM(ps)  ((ps) >> 16)
#define PS_TRANS_GET_TO(ps)    ((ps) & 0xFFFF)

#define PPN_FEATURE_LENGTH_ENABLE_BITFLIPS_DATA_COLLECTION  4
#define PPN_FEATURE_LENGTH_DIE_CHIP_ID                      8
#define PPN_FEATURE_LENGTH_TEMPERATURE                      2
#define PPN_FEATURE_LENGTH_DEFAULT                          4

#define PPN_FEATURE__ALLOW_SAVING_DEBUG_DATA_ENABLE   (1)
#define PPN_FEATURE__ALLOW_SAVING_DEBUG_DATA_DISABLE  (0)
#define PPN_FEATURE__DEBUG_DATA_CONFIGURATION_GEB     (1 << 0)
#define PPN_FEATURE__DEBUG_DATA_CONFIGURATION_READ    (1 << 1)
#define PPN_FEATURE__DEBUG_DATA_CONFIGURATION_PROGRAM (1 << 2)
#define PPN_FEATURE__DEBUG_DATA_CONFIGURATION_ERASE   (1 << 3)

#define PPN_OPERATION_STATUS__ERROR         (1 << 0)
#define PPN_OPERATION_STATUS__REFRESH       (1 << 1)
#define PPN_OPERATION_STATUS__RETIRE        (1 << 2)
#define PPN_OPERATION_STATUS__CLEAN         (1 << 3)
#define PPN_OPERATION_STATUS__GENERAL_ERROR (1 << 4)
#define PPN_OPERATION_STATUS__READY         (1 << 6)

#define PPN_FEAT_OPERATION_STATUS__ERROR               (1 << 0)
#define PPN_FEAT_OPERATION_STATUS__FW_UPDATE_ERROR     (1 << 1)
#define PPN_FEAT_OPERATION_STATUS__NOT_SUPPORTED       (1 << 2)
#define PPN_FEAT_OPERATION_STATUS__NOT_SUPPORTED_BY_CH (1 << 3)
#define PPN_FEAT_OPERATION_STATUS__GENERAL_ERROR       (1 << 4)
#define PPN_FEAT_OPERATION_STATUS__READY               (1 << 6)

#define PPN_FEAT_OPERATION_STATUS__SUCCESS             (PPN_FEAT_OPERATION_STATUS__READY)
#define PPN_FEAT_OPERATION_STATUS__FAILURE             (PPN_FEAT_OPERATION_STATUS__READY | PPN_FEAT_OPERATION_STATUS__ERROR)
#define PPN_FEAT_OPERATION_STATUS__FW_UPDATE_FAILURE   (PPN_FEAT_OPERATION_STATUS__READY | PPN_FEAT_OPERATION_STATUS__FW_UPDATE_ERROR | PPN_FEAT_OPERATION_STATUS__ERROR)
#define PPN_FEAT_OPERATION_STATUS__UNSUPPORTED         (PPN_FEAT_OPERATION_STATUS__READY | PPN_FEAT_OPERATION_STATUS__ERROR | PPN_FEAT_OPERATION_STATUS__NOT_SUPPORTED)
#define PPN_FEAT_OPERATION_STATUS__UNSUPPORTED_BY_CH   (PPN_FEAT_OPERATION_STATUS__READY | PPN_FEAT_OPERATION_STATUS__ERROR | PPN_FEAT_OPERATION_STATUS__NOT_SUPPORTED_BY_CH)
#define PPN_FEAT_OPERATION_STATUS__SUCCESS_WITH_GEB    (PPN_FEAT_OPERATION_STATUS__READY | PPN_FEAT_OPERATION_STATUS__GENERAL_ERROR)
#define PPN_FEAT_OPERATION_STATUS__FAILURE_WITH_GEB    (PPN_FEAT_OPERATION_STATUS__READY | PPN_FEAT_OPERATION_STATUS__ERROR | PPN_FEAT_OPERATION_STATUS__GENERAL_ERROR)

#define PPN_CONTROLLER_STATUS__PENDING_ERRORS             (1 << 0)
#define PPN_CONTROLLER_STATUS__READY_FOR_OPERATIONS       (1 << 1)
#define PPN_CONTROLLER_STATUS__OPERATION_STATUS_AVAILABLE (1 << 2)
#define PPN_CONTROLLER_STATUS__GENERAL_ERROR              (1 << 4)
#define PPN_CONTROLLER_STATUS__QUEUE_EMPTY                (1 << 5)
#define PPN_CONTROLLER_STATUS__READY                      (1 << 6)

#define PPN_LOW_POWER_STATUS__FAIL              (1 << 0)
#define PPN_LOW_POWER_STATUS__READY             (1 << 6)
#define PPN_LOW_POWER_STATUS__WRITE_PROTECT_BAR (1 << 7)

#define PPN_GET_DEBUG_DATA__GET_FAILURE_TYPE      0xFFFFFFul
#define PPN_tGDD_1_5_US                           (20*1000)
#define PPN_tGDD_1_0_US                           (10*1000)

#define PPN_SET_DEBUG_DATA__FORCE_ADDRESS         (0xF0FFFF)
#define PPN_SET_DEBUG_DATA__EMB_SWEEP_ADDRESS     (0x540000)

#define PPN_GEB_TYPE__EMB_SWEEP_GEB_TYPE          (0x07C0)

#define PPN_DELETE_DEBUG_DATA__ENABLE             (1)

#define PPN_ERROR_LIST_SIZE    (512)
#define PPN_DEVICE_ID_LEN      (6)
#define PPN_NAND_MARKETING_NAME_LENGTH (10)

#define PPN_PERFECT_PAGE_SIZE  (4112)
#define PPN_PERFECT_FMI_SECTORS_PER_PERFECT_PAGE ((UInt32)(PPN_PERFECT_PAGE_SIZE / H2FMI_BYTES_PER_SECTOR)) // rounds down
#define PPN_PERFECT_FMI_SECTOR_SIZE              (PPN_PERFECT_PAGE_SIZE / PPN_PERFECT_FMI_SECTORS_PER_PERFECT_PAGE)

#define PPN_TIMING_MIN_TRHW_NS (100)
#define PPN_TIMING_MIN_TWHR_NS (60)

#define PPN_FW_START_SIGNATURE (0x72617453)
#define PPN_FW_END_SIGNATURE   (0x00646E45)

#define PPN_1_5_ROW_ADDR_BYTES (4)
#define PPN_1_5_FAILURE_INFO_EXPCT_LEN (14)
#define PPN_1_0_FAILURE_INFO_EXPCT_LEN (12)

typedef enum
{
    ppnFwTypeFw,
    ppnFwTypeFwa
} h2fmi_ppn_fw_type;

//update PPN_1_0_FAILURE_INFO_EXPCT_LEN if this struct is updated
typedef struct _h2fmi_ppn_failure_info_ppn1_0_t
{
    UInt16   type;
    UInt8    startPage[3];
    UInt8    pageCount[3];
    UInt8    checksum;
    UInt8    reserved1;
    UInt8    reserved2;
    UInt8    reserved3;
} h2fmi_ppn_failure_info_ppn1_0_t;

//update PPN_1_5_FAILURE_INFO_EXPCT_LEN if this struct is updated
typedef struct _h2fmi_ppn_failure_info_ppn1_5_t
{
    UInt16   type;
    UInt8    startPage[4];
    UInt8    pageCount[3];
    UInt8    checksum;
    UInt8    reserved1;
    UInt8    reserved2;
    UInt8    reserved3;
} h2fmi_ppn_failure_info_ppn1_5_t;

typedef union _h2fmi_ppn_failure_info_t
{
    h2fmi_ppn_failure_info_ppn1_5_t failure_info_1_5;
    h2fmi_ppn_failure_info_ppn1_0_t failure_info_1_0;
} h2fmi_ppn_failure_info_t; 

#define FAIL_INFO_GET_START_PAGE(bytesPerRowAddr, failInfo)  (((bytesPerRowAddr) == PPN_1_5_ROW_ADDR_BYTES) ? \
                                                                  (((failInfo).failure_info_1_5.startPage[0]) | \
                                                                   ((failInfo).failure_info_1_5.startPage[1] << 8) | \
                                                                   ((failInfo).failure_info_1_5.startPage[2] << 16) | \
                                                                   ((failInfo).failure_info_1_5.startPage[3] << 24)) : \
                                                                  (((failInfo).failure_info_1_0.startPage[0]) |\
                                                                   ((failInfo).failure_info_1_0.startPage[1] << 8) |\
                                                                   ((failInfo).failure_info_1_0.startPage[2] << 16)))

#define FAIL_INFO_GET_PAGE_COUNT(bytesPerRowAddr, failInfo)  (((bytesPerRowAddr) == PPN_1_5_ROW_ADDR_BYTES) ? \
                                                       (((failInfo).failure_info_1_5.pageCount[0]) | \
                                                        ((failInfo).failure_info_1_5.pageCount[1] << 8) | \
                                                        ((failInfo).failure_info_1_5.pageCount[2] << 16)) : \
                                                       (((failInfo).failure_info_1_0.pageCount[0]) | \
                                                        ((failInfo).failure_info_1_0.pageCount[1] << 8) | \
                                                        ((failInfo).failure_info_1_0.pageCount[2] << 16)))

#define FAIL_INFO_GET_TYPE(bytesPerRowAddr, failInfo)  (((bytesPerRowAddr) == PPN_1_5_ROW_ADDR_BYTES) ? \
                                                        ((failInfo).failure_info_1_5.type) : \
                                                        ((failInfo).failure_info_1_0.type))

typedef enum
{
    powerStateOff,
    powerStateReadyForReset,
    powerStateLowPower,
    powerStateNormal,
    powerStateStandby
} PPN_POWER_STATE;


typedef struct _h2fmi_ppn_device_params_t
{
    char     header[16];
    UInt32   caus_per_channel;
    UInt32   cau_bits;
    UInt32   blocks_per_cau;
    UInt32   block_bits;
    UInt32   pages_per_block;
    UInt32   pages_per_block_slc;
    UInt32   page_address_bits;
    UInt32   address_bits_bits_per_cell;
    UInt32   default_bits_per_cell;
    UInt32   page_size;
    UInt32   dies_per_channel;
    UInt32   block_pairing_scheme;
    UInt32   bytes_per_row_address;
    UInt8    reserved[92];
    UInt8    tRC;
    UInt8    tREA;
    UInt8    tREH;
    UInt8    tRHOH;
    UInt8    tRHZ;
    UInt8    tRLOH;
    UInt8    tRP;
    UInt8    reserved2;
    UInt8    tWC;
    UInt8    tWH;
    UInt8    tWP;
    UInt8    reserved3[53];
    UInt32   read_queue_size;
    UInt32   program_queue_size;
    UInt32   erase_queue_size;
    UInt32   prep_function_buffer_size;
    UInt32   tRST_ms;
    UInt32   tPURST_ms;
    UInt32   tSCE_ms;
    UInt32   tCERDY_us;
    UInt8    reserved4[4];
    UInt32   cau_per_channel2;
    UInt32   dies_per_channel2;
    UInt8    reserved5[4];
    UInt8    ddr_tRC;
    UInt8    ddr_tREH;
    UInt8    ddr_tRP;
    UInt8    reserved6;
    UInt32   tDQSL_ps;
    UInt32   tDQSH_ps;
    UInt32   tDSC_ps;
    UInt32   tDQSRE_ps;
    UInt32   tDQSQ_ps;
    UInt32   tDVW_ps;
    UInt8    max_interface_speed;
    UInt8    reserved7[211];
} h2fmi_ppn_device_params_t;

typedef struct _h2fmi_ppn_t
{
    h2fmi_ppn_device_params_t  device_params;
    UInt32                     spec_version;
#if !defined(APPLICATION_EMBEDDEDIOP)
    NandGeometry               geometry;
    NandBoardSupport           boardSupport;
    NandPpn                    nandPpn;
#endif /* !APPLICATION_EMBEDDEDIOP */
    UInt8                      bytes_per_row_address;
    UInt8                      bytes_per_full_address;
    BOOL32                     general_error;
    UInt32                     general_error_ce;
    UInt32                     debug_flags;
    BOOL32                     debug_flags_valid;
    UInt32                     vs_debug_flags;
    BOOL32                     vs_debug_flags_valid;
    UInt32                     allow_saving_debug_data;
    PageAddressType           *prep_buffer[PPN_MAX_CES_PER_BUS];

#if FMI_ECC_DEBUG
    UInt32                     ecc_sectors_read;
#endif // FMI_ECC_DEBUG

} h2fmi_ppn_t;

typedef struct _ppn_feature_entry_t
{
    UInt32 version;
    UInt16 feature;
    UInt32 value;
    UInt32 bytes;
} ppn_feature_entry_t;

void h2fmiPpnPerformCommandList(PPNCommandStruct **commands, UInt32 num_commands);

Int32 h2fmiPpnReadNoEcc(UInt16 wVirtualCE,
                        UInt32 dwPpn,
                        UInt8 *pabDataBuf,
                        UInt8 *pabSpareBuf);

Int32 h2fmiPpnReadSequentialPages(UInt32 *padwPpn,
                                  UInt8  *pabDataBuf,
                                  UInt8  *pabMetaBuf,
                                  UInt16  wNumOfPagesToRead,
                                  UInt8  *pbMaxCorrectedBits,
                                  UInt8  *pdwaSectorStats,
                                  BOOL32  bDisableWhitening);

Int32 h2fmiPpnReadScatteredPages(UInt16 *pawCEs,
                                 UInt32 *padwPpn,
                                 UInt8  *pabDataBuf,
                                 UInt8  *pabMetaBuf,
                                 UInt16  wNumOfPagesToRead,
                                 UInt8  *pbMaxCorrectedBits,
                                 UInt8  *pdwaSectorStats,
                                 BOOL32  bDisableWhitening);

Int32 h2fmiPpnReadSinglePage(UInt16 wVirtualCE,
                             UInt32 dwPpn,
                             UInt8* pabDataBuf,
                             UInt8* pabMetaBuf,
                             UInt8* pbMaxCorrectedBits,
                             UInt8* pdwaSectorStats,
                             BOOL32 bDisableWhitening);

Int32 h2fmiPpnReadBootpage(UInt16 wVirtualCE,
                           UInt32 dwPpn,
                           UInt8 *pabData);

Int32 h2fmiPpnReadSinglePageMaxECC(UInt16 wVirtualCE,
                                   UInt32 dwPpn,
                                   UInt8 *pabDataBuf);

Int32 h2fmiPpnEraseSingleBlock(UInt16 wVirtualCE,
                               UInt32 wPbn);

Int32 h2fmiPpnWriteScatteredPages(UInt16 *pawBanks,
                                  UInt32 *padwPpn,
                                  UInt8  *pabDataBuf,
                                  UInt8  *pabMetaBuf,
                                  UInt16  wNumOfPagesToWrite,
                                  UInt16 *pawFailingCE,
                                  BOOL32  bDisableWhitening,
                                  UInt32 *pwFailingPageNum);

Int32 h2fmiPpnWriteSinglePage(UInt16 wVirtualCE,
                              UInt32 dwPpn,
                              UInt8 *pabDataBuf,
                              UInt8 *pabMetaBuf,
                              BOOL32 disableWhitening);

Int32 h2fmiPpnWriteSinglePageMaxECC(UInt16 wBank,
                                    UInt32 nPpn,
                                    UInt8 *pdBuf);

Int32 h2fmiPpnWriteBootpage(UInt16 wVirtualCE,
                            UInt32 dwPpn,
                            UInt8 *pabData);

Int32 h2fmiPpnReadCauBbt(UInt16 wCS, UInt32 wCAU, UInt8 *pData);

Int32 h2fmiPpnUpdateFw(UInt16 wVirtualCE, 
                       UInt8 *fw, 
                       UInt32 fw_length, 
                       UInt8 *fwa, 
                       UInt32 fwa_length);

Int32 h2fmiPpnGetFirmwareVersion(UInt16 virtualCE,
                                 UInt8 *buffer);
BOOL32 h2fmiPpnValidateFirmwareVersions(UInt8 *buffer, UInt32 ce_count);
Int32 h2fmiPpnGetControllerHwId(UInt16 virtualCE, UInt8 *buffer);
Int32 h2fmiPpnGetManufacturerId(UInt16 virtualCE, UInt8 *buffer);
BOOL32 h2fmiPpnValidateManufacturerIds(UInt8 *expected, UInt32 ce_count);
Int32 h2fmiPpnGetControllerInfo(UInt16 virtualCE, UInt8 *mfg_id, UInt8 *fw_version);
Int32 h2fmiVthSweepSetup(UInt8 virCE, UInt32 blk, h2fmi_ppn_failure_info_t *failInfo, UInt32 *totalDataToRead);
Int32 h2fmiDmaDebugData(UInt8 virCE, UInt32 page, UInt32 pageCount, struct dma_segment *sgl, BOOL32 waitFortGDD);

void   h2fmi_ppn_dma_debug_data_payload(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 page, UInt32 pageCount, struct dma_segment *sgl, BOOL32 waitFortGDD);
UInt32 h2fmi_ppn_get_general_error_info(h2fmi_t *fmi, h2fmi_ce_t ce, h2fmi_ppn_failure_info_t *buffer, UInt32 *dataLength, struct dma_segment *sgl);
Int32 h2fmi_ppn_configure_debug_data(h2fmi_t *fmi, h2fmi_ce_t ce);

#if SUPPORT_TOGGLE_NAND
BOOL32 h2fmi_ppn15_get_temperature(h2fmi_t *fmi, h2fmi_ce_t ce, Int16 *temperature);
BOOL32 h2fmi_ppn15_enable_optional_signals(h2fmi_t *fmi, h2fmi_ce_t ce);
#endif

Int32 h2fmi_ppn_write_bootpage(h2fmi_t      *fmi,
                               UInt16        ce,
                               UInt32        dwPpn,
                               const UInt8  *pabData);

Int32 h2fmi_ppn_read_multi(h2fmi_t *fmi,
                           PPNCommandStruct *ppnCommand,
                           struct dma_segment *data_segment_array,
                           struct dma_segment *meta_segment_array);

Int32 h2fmi_ppn_write_multi(h2fmi_t *fmi,
                            PPNCommandStruct *ppnCommand,
                            struct dma_segment *data_segment_array,
                            struct dma_segment *meta_segment_array);

Int32 h2fmi_ppn_read_bootpage(h2fmi_t   *fmi,
                              UInt16     ce,
                              UInt32     page,
                              UInt8     *data,
                              UInt8     *max_corrected);

Int32 h2fmi_ppn_erase_blocks(h2fmi_t  *fmi, PPNCommandStruct *ppnCommand);

Int32 h2fmi_ppn_read_cau_bbt(h2fmi_t *fmi,
                             PPNCommandStruct *ppnCommand,
                             UInt8   *buffer);

Int32 h2fmi_ppn_fw_update(h2fmi_t *fmi, h2fmi_ce_t ce, struct dma_segment *sgl, UInt32 fw_size, h2fmi_ppn_fw_type fw_type);
BOOL32 h2fmi_ppn_get_fw_version(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer);
BOOL32 h2fmi_ppn_get_package_assembly_code(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer);
BOOL32 h2fmi_ppn_get_controller_unique_id(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer);
BOOL32 h2fmi_ppn_get_controller_hw_id(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer);
BOOL32 h2fmi_ppn_get_controller_status(h2fmi_t *fmi, UInt8 *controller_status);
BOOL32 h2fmi_ppn_get_manufacturer_id(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer);
BOOL32 h2fmi_ppn_get_die_chip_id(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 die, UInt8 *buffer);
BOOL32 h2fmi_ppn_get_marketing_name(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer);
BOOL32 h2fmi_ppn_set_debug_data(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 type, UInt8 *buffer, UInt32 len);

void h2fmi_ppn_poweron(h2fmi_t *fmi);
void h2fmi_ppn_poweroff(h2fmi_t *fmi);
void h2fmi_ppn_set_power_low_power(h2fmi_t *fmi);

void h2fmi_ppn_set_power_standby(h2fmi_t *fmi);
BOOL32 h2fmi_ppn_init_channel(h2fmi_t *fmi, BOOL32 debug_flags_valid, UInt32 debug_flags, BOOL32 vs_debug_flags_valid, UInt32 vs_debug_flags, BOOL32 allow_saving_debug_data, UInt32 spec_version);
BOOL32 h2fmi_ppn_set_channel_power_state(h2fmi_t *fmi, UInt32 ps_tran);
BOOL32 h2fmi_ppn_post_rst_pre_pwrstate_operations(h2fmi_t* fmi);
BOOL32 h2fmi_ppn_get_device_params(h2fmi_t *fmi, h2fmi_ce_t ce, h2fmi_ppn_device_params_t *params);
BOOL32 h2fmi_ppn_get_next_operation_status(h2fmi_t *fmi, UInt8 *operation_status);
UInt32 h2fmi_ppn_get_device_info(UInt32 dwParamType);
UInt32 h2fmi_ppn_calculate_fmi_config(h2fmi_t *fmi);
UInt32 h2fmi_ppn_calculate_fmi_data_size(h2fmi_t *fmi);
UInt8 h2fmi_ppn_get_idle_counter(h2fmi_t *fmi, h2fmi_ce_t ce);
void  h2fmi_ppn_force_geb_address(h2fmi_t *fmi, h2fmi_ce_t ce, PageAddressType addr);
void h2fmi_ppn_set_feature_list(h2fmi_t *fmi, ppn_feature_entry_t *list, UInt32 size);

BOOL32 h2fmi_ppn_get_uid(h2fmi_t *fmi, UInt8 ce, UInt8 die, UInt8 *buf);

Int32 h2fmi_ppn_set_features(h2fmi_t *fmi, UInt32 ce, UInt16 feature, UInt8 *data, UInt32 len, BOOL32 optFeature, UInt8 *status);
BOOL32 h2fmi_ppn_get_feature(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 feature, UInt8 *buffer, UInt32 len, UInt8 *status);
void h2fmi_ppn_process_error_list(h2fmi_t *fmi, PPNCommandStruct *ppnCommand, UInt8 ceIndex, UInt32 *list, UInt8 status);

#if !defined(APPLICATION_EMBEDDEDIOP)
void h2fmi_ppn_fill_nandinfo(h2fmi_t *fmi, NandInfo *nandInfo, NandRequiredTimings *requiredTiming);
#endif /* !APPLICATIONS_EMBEDDEDIOP */

void fmiss_init_sequences(h2fmi_t *fmi);
Int32 fmiss_ppn_read_multi(h2fmi_t *fmi, PPNCommandStruct *ppnCommand, struct dma_segment *data_segment_array, struct dma_segment *meta_segment_array);
Int32 fmiss_ppn_write_multi(h2fmi_t *fmi, PPNCommandStruct *ppnCommand, struct dma_segment *data_segment_array, struct dma_segment *meta_segment_array);

#if H2FMI_PPN_VERIFY_SET_FEATURES
void h2fmi_ppn_verify_feature_shadow(h2fmi_t *fmi);
void h2fmi_ppn_reset_feature_shadow(h2fmi_t *fmi);
#endif // H2FMI_PPN_VERIFY_SET_FEATURES

#endif /* _H2FMI_PPN_H_ */
