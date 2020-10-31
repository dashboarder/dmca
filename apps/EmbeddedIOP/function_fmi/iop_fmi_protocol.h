/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _IOP_FMI_PROTOCOL_H_
#define _IOP_FMI_PROTOCOL_H_

#include <sys/types.h>


#define kIOPFMI_MAX_NUM_OF_BUSES          (2)
#define kIOPFMI_MAX_NUM_OF_BANKS_PER_BUS  (8)
#define kIOPFMI_MAX_NUM_OF_CES_PER_BUS    (8)
#define kIOPFMI_MAX_NUM_OF_ENABLES        (kIOPFMI_MAX_NUM_OF_CES_PER_BUS * kIOPFMI_MAX_NUM_OF_BUSES)
#define kIOPFMI_MAX_NUM_OF_BANKS          (kIOPFMI_MAX_NUM_OF_ENABLES * kIOPFMI_MAX_NUM_OF_BUSES)
#define kIOPFMI_MAX_NUM_OF_CHANNELS       (kIOPFMI_MAX_NUM_OF_BUSES)
#define kIOPFMI_BYTES_PER_META            (10)
#define kIOPFMI_BYTES_PER_CHIPID          (5)

// define the maximum number of pages that the controller is willing
// to support per I/O command; Mike says that it is generally safe to
// assume a practical upper bounds of 1 MB bursts; so, for worst case
// of 2KiB pages, this translates into 512 pages per multi-I/O command
#define kIOPFMI_MAX_IO_PAGES 512


/*
 * Command size is (somewhat) tunable.
 *
 * The principal consideration here is the maximum scatter/gather list size
 * this permits.
 */
#define kIOPFMI_COMMAND_SIZE              (512)


typedef UInt32 IOPFMI_opcode_t;

#define kIOPFMI_OPCODE_UNKNOWN            ((IOPFMI_opcode_t) 0)

#define kIOPFMI_OPCODE_SET_CONFIG         ((IOPFMI_opcode_t) 1)
#define kIOPFMI_OPCODE_RESET_EVERYTHING   ((IOPFMI_opcode_t) 2)
#define kIOPFMI_OPCODE_ERASE_SINGLE       ((IOPFMI_opcode_t) 3)
#define kIOPFMI_OPCODE_READ_SINGLE        ((IOPFMI_opcode_t) 4)
#define kIOPFMI_OPCODE_READ_RAW           ((IOPFMI_opcode_t) 5)
#define kIOPFMI_OPCODE_WRITE_SINGLE       ((IOPFMI_opcode_t) 6)
#define kIOPFMI_OPCODE_WRITE_RAW          ((IOPFMI_opcode_t) 7)
#define kIOPFMI_OPCODE_READ_MULTIPLE      ((IOPFMI_opcode_t) 8)
#define kIOPFMI_OPCODE_WRITE_MULTIPLE     ((IOPFMI_opcode_t) 9)
#define kIOPFMI_OPCODE_WRITE_BOOTPAGE     ((IOPFMI_opcode_t) 10)
#define kIOPFMI_OPCODE_READ_BOOTPAGE      ((IOPFMI_opcode_t) 11)
#define kIOPFMI_OPCODE_ERASE_MULTIPLE     ((IOPFMI_opcode_t) 12)
#define kIOPFMI_OPCODE_READ_CAU_BBT       ((IOPFMI_opcode_t) 13)
#define kIOPFMI_OPCODE_ERASE_SCATTERED    ((IOPFMI_opcode_t) 14)
#define kIOPFMI_OPCODE_UPDATE_FIRMWARE    ((IOPFMI_opcode_t) 15)
#define kIOPFMI_OPCODE_PPN_READ           ((IOPFMI_opcode_t) 16)
#define kIOPFMI_OPCODE_PPN_ERASE          ((IOPFMI_opcode_t) 17)
#define kIOPFMI_OPCODE_PPN_WRITE          ((IOPFMI_opcode_t) 18)
#define kIOPFMI_OPCODE_PPN_SET_POWER      ((IOPFMI_opcode_t) 19)
#define kIOPFMI_OPCODE_READ_CHIP_IDS      ((IOPFMI_opcode_t) 20)
#define kIOPFMI_OPCODE_GET_FAILURE_INFO   ((IOPFMI_opcode_t) 21)
#define kIOPFMI_OPCODE_GET_CONTROLLER_INFO ((IOPFMI_opcode_t) 22)
#define kIOPFMI_OPCODE_GET_DIE_INFO        ((IOPFMI_opcode_t) 23)
#define kIOPFMI_OPCODE_POST_RESET_OPER     ((IOPFMI_opcode_t) 24)
#define kIOPFMI_OPCODE_GET_TEMPERATURE     ((IOPFMI_opcode_t) 25)
#define kIOPFMI_OPCODE_SET_FEATURES        ((IOPFMI_opcode_t) 26)


typedef UInt32 IOPFMI_flags_t;

#define kIOPFMI_FLAGS_NONE                ((IOPFMI_flags_t) 0)
#define kIOPFMI_FLAGS_BLANK_CHECK         ((IOPFMI_flags_t) (1<<0))
#define kIOPFMI_FLAGS_WITH_DMA_SGL        ((IOPFMI_flags_t) (1<<1))
#define kIOPFMI_FLAGS_USE_AES             ((IOPFMI_flags_t) (1<<2))
#define kIOPFMI_FLAGS_HOMOGENIZE          ((IOPFMI_flags_t) (1<<3))


typedef UInt32 IOPFMI_status_t;

#define kIOPFMI_STATUS_UNKNOWN            ((IOPFMI_status_t) 0)

#define kIOPFMI_STATUS_SUCCESS            ((IOPFMI_status_t) 1)
#define kIOPFMI_STATUS_BLANK              ((IOPFMI_status_t) 2)

#define kIOPFMI_STATUS_FAILURE            ((IOPFMI_status_t) 0x80000000)
#define kIOPFMI_STATUS_DEVICE_ERROR       ((IOPFMI_status_t) 0x80000001)
#define kIOPFMI_STATUS_DEVICE_TIMEOUT     ((IOPFMI_status_t) 0x80000002)
#define kIOPFMI_STATUS_DMA_TIMEOUT        ((IOPFMI_status_t) 0x80000003)
#define kIOPFMI_STATUS_PARAM_INVALID      ((IOPFMI_status_t) 0x80000004)
#define kIOPFMI_STATUS_UNIMPLEMENTED      ((IOPFMI_status_t) 0x80000005)
#define kIOPFMI_STATUS_IOP_NOT_READY      ((IOPFMI_status_t) 0x80000006)

#define kIOPFMI_STATUS_FAILED_RESET_ALL   ((IOPFMI_status_t) 0x80000011)
#define kIOPFMI_STATUS_FAILED_READ_ID     ((IOPFMI_status_t) 0x80000012)
#define kIOPFMI_STATUS_FAILED_ERASE_BLOCK ((IOPFMI_status_t) 0x80000013)
#define kIOPFMI_STATUS_FAILED_READ_PAGE   ((IOPFMI_status_t) 0x80000014)
#define kIOPFMI_STATUS_FAILED_WRITE_PAGE  ((IOPFMI_status_t) 0x80000015)
#define kIOPFMI_STATUS_FAILED_READ_RAW    ((IOPFMI_status_t) 0x80000016)
#define kIOPFMI_STATUS_FAILED_WRITE_RAW   ((IOPFMI_status_t) 0x80000017)
#define kIOPFMI_STATUS_FAILED_READ_MULTI  ((IOPFMI_status_t) 0x80000018)
#define kIOPFMI_STATUS_FAILED_WRITE_MULTI ((IOPFMI_status_t) 0x80000019)
#define kIOPFMI_STATUS_FAILED_WRITE_BOOTPAGE ((IOPFMI_status_t) 0x8000001A)
#define kIOPFMI_STATUS_FAILED_READ_BOOTPAGE  ((IOPFMI_status_t) 0x8000001B)

#define kIOPFMI_STATUS_FMC_DONE_TIMEOUT     ((IOPFMI_status_t) 0x8000001C)
#define kIOPFMI_STATUS_READY_BUSY_TIMEOUT     ((IOPFMI_status_t) 0x8000001D)
#define kIOPFMI_STATUS_ECC_CLEANUP     ((IOPFMI_status_t) 0x8000001E)
#define kIOPFMI_STATUS_ECC_DONE_TIMEOUT     ((IOPFMI_status_t) 0x8000001F)
#define kIOPFMI_STATUS_PPN_GENERAL_ERROR    ((IOPFMI_status_t) 0x80000020)
/**
 * Special status -- status code from NAND is in lower 16-bits.
 */
#define kIOPFMI_STATUS_PGM_ERROR     ((IOPFMI_status_t) 0x80200000)
#define kIOPFMI_STATUS_PGM_ERROR_MASK 0xffff0000
#define kIOPFMI_STATUS_ERASE_ERROR     ((IOPFMI_status_t) 0x80210000)
#define kIOPFMI_STATUS_ERASE_ERROR_MASK 0xffff0000
#define kIOPFMI_STATUS_DMA_DONE_TIMEOUT     ((IOPFMI_status_t) 0x80000022)
#define kIOPFMI_STATUS_NOT_ALL_CLEAN     ((IOPFMI_status_t) 0x80000023)
#define kIOPFMI_STATUS_AT_LEAST_ONE_UECC     ((IOPFMI_status_t) 0x80000024)
#define kIOPFMI_STATUS_FUSED                 ((IOPFMI_status_t) 0x80000025)

/**
 * Vendor-specific codes ... (passed in via vendorProtocol 
 * field) 
 */
#define kVS_NONE       0
#define kVS_HYNIX_2P      1
#define kVS_TOSHIBA_2P    2
#define kVS_MICRON_2P     3
#define kVS_SAMSUNG_2D    4
#define kVS_SAMSUNG_2P_2D    5

typedef UInt8 IOPFMI_chipid_t[kIOPFMI_BYTES_PER_CHIPID];


typedef UInt32 IOPFMI_correction_t;

/* this must match <drivers/dma.h>::struct dma_segment */
struct _IOPFMI_dma_segment {
    u_int32_t   paddr;
    u_int32_t   length;
};
typedef struct _IOPFMI_dma_segment IOPFMI_dma_segment;

#define kIOPFMI_AES_KEY_TYPE_USER128 (1)
#define kIOPFMI_AES_KEY_TYPE_USER192 (2)
#define kIOPFMI_AES_KEY_TYPE_USER256 (3)
#define kIOPFMI_AES_KEY_TYPE_UID0    (4)
#define kIOPFMI_AES_KEY_TYPE_GID0    (5)
#define kIOPFMI_AES_KEY_TYPE_GID1    (6)


typedef UInt32 IOPFMI_state_t;

#define kIOPFMI_STATE_NONE                ((IOPFMI_state_t) 0)
#define kIOPFMI_STATE_WAKING_UP           ((IOPFMI_state_t) (1UL << 0))
#define kIOPFMI_STATE_POWER_CHANGED       ((IOPFMI_state_t) (1UL << 1))

struct _IOPFMI
{
    IOPFMI_opcode_t  opcode;
    IOPFMI_flags_t   flags;
    IOPFMI_status_t  status;
    UInt32           context;
    UInt32           bus;
    IOPFMI_state_t   state;
};
typedef struct _IOPFMI IOPFMI;


struct _IOPFMI_FailureDetails
{
    /**
     * The operation may have had more than 
     * kIOPFMI_MAX_NUM_OF_ENABLES operations -- so we tell them how 
     * many were executed so that the caller can then figure out 
     * exactly where in a large list of things we were at when 
     * things went wrong. 
     */
    UInt32  wNumCE_Executed;
    IOPFMI_status_t wOverallOperationFailure;
    UInt32 wSingleCEStatus;
    UInt32 wFirstFailingCE;
};
typedef struct _IOPFMI_FailureDetails IOPFMI_FailureDetails;

struct _IOPFMI_PpnStatus
{
    UInt8  operation_status[kIOPFMI_MAX_IO_PAGES];
    UInt8  program_failed_pages[kIOPFMI_MAX_IO_PAGES];
    UInt8  program_ignored_pages[kIOPFMI_MAX_IO_PAGES];
    UInt8  program_retired_pages[kIOPFMI_MAX_IO_PAGES];
};
typedef struct _IOPFMI_PpnStatus IOPFMI_PpnStatus;


struct _IOPFMI_ResetEverything
{
    IOPFMI  iopfmi;
};
typedef struct _IOPFMI_ResetEverything IOPFMI_ResetEverything;

struct _IOPFMI_PostResetOperations
{
    IOPFMI  iopfmi;
};
typedef struct _IOPFMI_PostResetOperations IOPFMI_PostResetOperations;

struct _IOPFMI_EraseSingle
{
    IOPFMI  iopfmi;

    UInt16  ce;
    UInt32  block_number;
    IOPFMI_FailureDetails failure_details;
};
typedef struct _IOPFMI_EraseSingle IOPFMI_EraseSingle;


struct _IOPFMI_EraseMultiple
{
    IOPFMI  iopfmi;

    UInt32  number_of_elements;
    UInt16  ce[ kIOPFMI_MAX_NUM_OF_ENABLES ];
    UInt32  block_number[ kIOPFMI_MAX_NUM_OF_ENABLES ];
    IOPFMI_FailureDetails failure_details;
};
typedef struct _IOPFMI_EraseMultiple IOPFMI_EraseMultiple;

struct _IOPFMI_IOSingle
{
    IOPFMI  iopfmi;

    UInt16  ce;
    UInt32  page_number;
    UInt32  data_address;
    UInt32  meta_address;

    // if non-zero, is array of one byte per sector in page
    UInt32  correction_address;
    
    UInt32  aes_iv_array;           // Array of IOPFMI_aes_iv entries
    UInt32  aes_num_chains;
    UInt32  aes_chain_size;
    UInt32  aes_key_type;
    UInt8   aes_key_bytes[32];
};
typedef struct _IOPFMI_IOSingle IOPFMI_IOSingle;


struct _IOPFMI_IOMultiple
{
    IOPFMI  iopfmi;

    UInt32  page_count;

    UInt32  chip_enable_array;
    UInt32  page_number_array;
    UInt32  data_segments_array;    // Array of IOPFMI_dma_segment entries
    UInt32  data_segment_array_length_in_bytes;
    UInt32  meta_segments_array;    // Array of IOPFMI_dma_segment entries
    UInt32  meta_segment_array_length_in_bytes;

    // if non-zero, is array of one byte per sector in page
    UInt32  corrections_array;
    
    UInt32  aes_iv_array;           // Array of IOPFMI_aes_iv entries
    UInt32  aes_num_chains;
    UInt32  aes_chain_size;
    UInt32  aes_key_type;
    UInt8   aes_key_bytes[32];

    UInt32  vendorProtocol;

    IOPFMI_FailureDetails   failure_details;
};
typedef struct _IOPFMI_IOMultiple IOPFMI_IOMultiple;

struct _IOPFMI_IOPpn
{
    IOPFMI  iopfmi;

    UInt32  page_count;
    UInt32  data_segments_array;
    UInt32  data_segments_array_length;
    UInt32  meta_segments_array;
    UInt32  meta_segments_array_length;
    UInt32  ppn_fil_command;
    UInt32  ppn_fil_size;
    UInt32  aes_iv_array;   // Array of IOPFMI_aes_iv entries
    UInt32  aes_num_chains;
    UInt32  aes_chain_size;
    UInt32  aes_key_type;
    UInt8   aes_key_bytes[32];
    UInt32  geb_ce;
    UInt8   overall_status;
};
typedef struct _IOPFMI_IOPpn IOPFMI_IOPpn;

struct _IOPFMI_IORaw
{
    IOPFMI  iopfmi;

    UInt16  ce;
    UInt32  page_number;
    UInt32  buffer_address;
};
typedef struct _IOPFMI_IORaw IOPFMI_IORaw;

struct _IOPFMI_IOBootPage
{
    IOPFMI  iopfmi;

    UInt16  ce;
    UInt32  page_number;
    UInt32  buffer_address;

    // if non-zero, is array of one byte per sector in page
    UInt32  corrections_array;    
};
typedef struct _IOPFMI_IOBootPage IOPFMI_IOBootPage;

struct _IOPFMI_ReadCauBbt
{
    IOPFMI  iopfmi;

    UInt16  ce;
    UInt32  cau;
    UInt32  buffer_address;
};
typedef struct _IOPFMI_ReadCauBbt IOPFMI_ReadCauBbt;

struct _IOPFMI_UpdateFirmware
{
    IOPFMI  iopfmi;

    UInt32  sgl;
    UInt32  sgl_length_in_bytes;
    UInt32  fw_size;
};
typedef struct _IOPFMI_UpdateFirmware IOPFMI_UpdateFirmware;

struct _IOPFMI_SetConfig
{
    IOPFMI  iopfmi;

    UInt32  fmi;
    UInt32  num_of_ce;
    UInt32  valid_ces;
    UInt32  pages_per_block;
    UInt32  sectors_per_page;
    UInt32  bytes_per_page;
    UInt32  bytes_per_spare;
    UInt32  logical_page_size;
    UInt32  blocks_per_ce;
    UInt32  read_sample_cycles;
    UInt32  read_setup_cycles;
    UInt32  read_hold_cycles;
    UInt32  write_setup_cycles;
    UInt32  write_hold_cycles;
    UInt32  dqs_half_cycles;
    UInt32  ce_hold_cycles;
    UInt32  ce_setup_cycles;
    UInt32  adl_cycles;
    UInt32  whr_cycles;
    UInt32  read_pre_cycles;
    UInt32  read_post_cycles;
    UInt32  write_pre_cycles;
    UInt32  write_post_cycles;
    UInt32  enable_diff_DQS;
    UInt32  enable_diff_RE;
    UInt32  enable_VREF;
    UInt32  retire_on_invalid_refresh;
    UInt32  reg_dqs_delay;
    UInt32  ppn;
    UInt32  ppn_version;
    UInt32  toggle_system;
    UInt32  toggle;
    UInt32  wMaxOutstandingCEWriteOperations;
    UInt32  valid_bytes_per_meta;
    UInt32  total_bytes_per_meta;
    UInt32  ppn_debug_flags;
    UInt32  ppn_debug_flags_valid;
    UInt32  ppn_vs_debug_flags;
    UInt32  ppn_vs_debug_flags_valid;
    UInt32  ppn_allow_saving_debug_data;
    UInt32  clock_speed_khz;
};
typedef struct _IOPFMI_SetConfig IOPFMI_SetConfig;

struct _IOPFMI_ReadChipIDs
{
    IOPFMI  iopfmi;
    UInt32  chip_id_buffer;
};
typedef struct _IOPFMI_ReadChipIDs IOPFMI_ReadChipIDs;

// another copy lives in H2fmi_ppn.h (changes should be made to both places)
#define PPN_FEATURE__POWER_STATE__LOW_POWER   (0x1)
#define PPN_FEATURE__POWER_STATE__ASYNC       (0x2)
#define PPN_FEATURE__POWER_STATE__STANDBY     (0x4)
#define PPN_FEATURE__POWER_STATE__DDR         (0xA)

// Power State transition unique identifiers and accessors
// another copy lives in H2fmi_ppn.h (changes should be made to both places)
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

struct _IOPFMI_SetPower
{
    IOPFMI  iopfmi;
    UInt32  power_state_trans;
};
typedef struct _IOPFMI_SetPower IOPFMI_SetPower;

struct _IOPFMI_GetFailureInfo
{
    IOPFMI iopfmi;
    UInt32 ce;
    UInt16 type;
    UInt32 rma_buffer_length;
    UInt32 sgl;
    UInt32 sgl_length;
};
typedef struct _IOPFMI_GetFailureInfo IOPFMI_GetFailureInfo;

#define kIOPFMI_PPN_FW_VERSION_LENGTH             16
#define kIOPFMI_PPN_PACKAGE_ASSEMBLY_CODE_LENGTH  16
#define kIOPFMI_PPN_CONTROLLER_UNIQUE_ID_LENGTH   16
#define kIOPFMI_PPN_CONTROLLER_HW_ID_LENGTH       16
#define kIOPFMI_PPN_MANUFACTURER_ID_LENGTH        8
#define kIOPFMI_PPN_NAND_MARKETING_NAME_LENGTH    10

struct _IOPFMI_GetControllerInfo
{
    IOPFMI iopfmi;
    UInt32 ce;

    UInt8  fw_version[kIOPFMI_PPN_FW_VERSION_LENGTH];
    UInt8  package_assembly_code[kIOPFMI_PPN_PACKAGE_ASSEMBLY_CODE_LENGTH];
    UInt8  controller_unique_id[kIOPFMI_PPN_CONTROLLER_UNIQUE_ID_LENGTH];
    UInt8  controller_hw_id[kIOPFMI_PPN_CONTROLLER_HW_ID_LENGTH];
    UInt8  manufacturer_id[kIOPFMI_PPN_MANUFACTURER_ID_LENGTH];
    UInt8  marketing_name[kIOPFMI_PPN_NAND_MARKETING_NAME_LENGTH];

    UInt32 caus;
    UInt32 cau_bits;
    UInt32 blocks_per_cau;
    UInt32 block_bits;
    UInt32 pages_per_block_mlc;
    UInt32 pages_per_block_slc;
    UInt32 page_address_bits;
    UInt32 bits_per_cell_bits;
    UInt32 default_bits_per_cell;
    UInt32 page_size;
    UInt32 dies;
    UInt32 tRC;
    UInt32 tREA;
    UInt32 tREH;
    UInt32 tRHOH;
    UInt32 tRHZ;
    UInt32 tRLOH;
    UInt32 tRP;
    UInt32 tWC;
    UInt32 tWH;
    UInt32 tWP;
    UInt32 read_queue_size;
    UInt32 program_queue_size;
    UInt32 erase_queue_size;
    UInt32 prep_function_buffer_size;
    UInt32 tRST;
    UInt32 tPURST;
    UInt32 tSCE;
    UInt32 tCERDY;
};
typedef struct _IOPFMI_GetControllerInfo IOPFMI_GetControllerInfo;

struct _IOPFMI_GetTemperature
{
    IOPFMI iopfmi;
    Int16 temperature_celsius;
};
typedef struct _IOPFMI_GetTemperature IOPFMI_GetTemperature;

#define kIOPFMI_PPN_DIE_UNIQUE_ID_LENGTH    16
#define kIOPFMI_PPN_DIE_CHIP_ID_LENGTH      8

struct _IOPFMI_GetDieInfo
{
    IOPFMI iopfmi;
    UInt32 ce;
    UInt32 die;

    UInt8  unique_id[kIOPFMI_PPN_DIE_UNIQUE_ID_LENGTH];
    UInt8  chip_id[kIOPFMI_PPN_DIE_CHIP_ID_LENGTH];
};
typedef struct _IOPFMI_GetDieInfo IOPFMI_GetDieInfo;

struct _IOPFMI_SetFeatures
{
    IOPFMI iopfmi;

    UInt32 list;
    UInt32 size;
};
typedef struct _IOPFMI_SetFeatures IOPFMI_SetFeatures;

union _IOPFMI_Command
{
    IOPFMI                   iopfmi;

    IOPFMI_SetConfig         set_config;
    IOPFMI_ResetEverything   reset_everything;
    IOPFMI_PostResetOperations post_reset_oper;
    IOPFMI_EraseSingle       erase_single;
    IOPFMI_EraseMultiple     erase_multiple;
    IOPFMI_IOSingle          io_single;
    IOPFMI_IOMultiple        io_multiple;
    IOPFMI_IORaw             io_raw;
    IOPFMI_IOBootPage        io_bootpage;
    IOPFMI_ReadCauBbt        read_cau_bbt;
    IOPFMI_UpdateFirmware    update_firmware;
    IOPFMI_IOPpn             io_ppn;
    IOPFMI_ReadChipIDs       read_chip_ids;
    IOPFMI_SetPower          set_power;
    IOPFMI_GetFailureInfo    get_failure_info;
    IOPFMI_GetControllerInfo get_controller_info;
    IOPFMI_GetTemperature    get_temperature;
    IOPFMI_GetDieInfo        get_die_info;
    IOPFMI_SetFeatures       set_features;

    UInt8                    _pad[kIOPFMI_COMMAND_SIZE];
};
typedef union _IOPFMI_Command IOPFMI_Command;


#endif // _IOP_FMI_PROTOCOL_H_
