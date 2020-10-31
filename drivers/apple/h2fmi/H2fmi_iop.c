// *****************************************************************************
//
// File: H2fmi_iop.c
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2010 Apple, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "H2fmi_iop.h"
#include "H2fmi_private.h"
#include "H2fmi_ppn.h"
#include "fmiss.h"

#include <platform/soc/hwclocks.h>
#include <platform/soc/pmgr.h>
#include <platform/memmap.h>
#include <platform.h>
#include <drivers/dma.h>
#include <drivers/aes.h>
#include <debug.h>

// =============================================================================
// external global variable definitions
// =============================================================================

extern UInt8 nand_whitening_key[16];

// =============================================================================
// private function declarations
// =============================================================================


bool h2fmi_setup_aes(h2fmi_t* fmi, IOPFMI* cmd, hfmi_aes_iv* aes_iv_array, UInt32 aes_chain_size, UInt32 aes_key_type, UInt32* aes_key, UInt32* seeds)
{
    if (cmd->flags & (kIOPFMI_FLAGS_USE_AES | kIOPFMI_FLAGS_HOMOGENIZE))
    {
        switch (cmd->opcode)
        {
            case kIOPFMI_OPCODE_WRITE_SINGLE:
            case kIOPFMI_OPCODE_WRITE_MULTIPLE:
            case kIOPFMI_OPCODE_PPN_WRITE:
                fmi->aes_cxt.command = AES_CMD_ENC;
                break;
            case kIOPFMI_OPCODE_READ_SINGLE:
            case kIOPFMI_OPCODE_READ_MULTIPLE:
            case kIOPFMI_OPCODE_PPN_READ:
                fmi->aes_cxt.command = AES_CMD_DEC;
                break;
            default:
                ASSERT(false);
                return false;
                break;
        }
        if (cmd->flags & kIOPFMI_FLAGS_USE_AES)
        {
            fmi->current_aes_iv_array = (hfmi_aes_iv*)aes_iv_array;
            switch (aes_key_type)
            {
                case kIOPFMI_AES_KEY_TYPE_USER128:
                    fmi->aes_cxt.keying = AES_KEY_TYPE_USER | AES_KEY_SIZE_128;
                    fmi->aes_cxt.key = aes_key;
                    WMR_PRINT(CRYPTO, "Using HW key IV: 0x%08x 0x%08x 0x%08x 0x%08x\n",
                              aes_key[0], aes_key[1], aes_key[2], aes_key[3]);
                    break;
                case kIOPFMI_AES_KEY_TYPE_USER192:
                    fmi->aes_cxt.keying = AES_KEY_TYPE_USER | AES_KEY_SIZE_192;
                    fmi->aes_cxt.key = aes_key;
                    WMR_PRINT(CRYPTO, "Using HW key IV: 0x%08x 0x%08x 0x%08x 0x%08x 0X%08x 0x%08x\n",
                              aes_key[0], aes_key[1], aes_key[2], aes_key[3], aes_key[4], aes_key[5]);
                    break;
                case kIOPFMI_AES_KEY_TYPE_USER256:
                    fmi->aes_cxt.keying = AES_KEY_TYPE_USER | AES_KEY_SIZE_256;
                    fmi->aes_cxt.key = aes_key;
                    WMR_PRINT(CRYPTO, "Using HW key IV: 0x%08x 0x%08x 0x%08x 0x%08x 0X%08x 0x%08x 0X%08x 0x%08x\n",
                              aes_key[0], aes_key[1], aes_key[2], aes_key[3], aes_key[4], aes_key[5], aes_key[6], aes_key[7]);
                    break;
                case kIOPFMI_AES_KEY_TYPE_UID0:
                    fmi->aes_cxt.keying = AES_KEY_TYPE_UID0;
                    fmi->aes_cxt.key = NULL;
                    WMR_PRINT(CRYPTO, "Using HW UID0\n");
                    break;
                case kIOPFMI_AES_KEY_TYPE_GID0:
                    fmi->aes_cxt.keying = AES_KEY_TYPE_GID0;
                    fmi->aes_cxt.key = NULL;
                    WMR_PRINT(CRYPTO, "Using HW GID0\n");
                    break;
                case kIOPFMI_AES_KEY_TYPE_GID1:
                    fmi->aes_cxt.keying = AES_KEY_TYPE_GID1;
                    fmi->aes_cxt.key = NULL;
                    WMR_PRINT(CRYPTO, "Using HW GID1\n");
                    break;
                default:
                    ASSERT(false);
                    return false;
                    break;
            }
            fmi->aes_cxt.chunk_size = aes_chain_size;
        }
        else
        {
            fmi->aes_cxt.keying = (AES_KEY_TYPE_USER | AES_KEY_SIZE_128);
            fmi->aes_cxt.key = nand_whitening_key;
            fmi->current_aes_iv_array = NULL;
            fmi->aes_cxt.chunk_size = fmi->logical_page_size;
            WMR_PRINT(CRYPTO, "Using constant AES for whitening\n");
        }

        fmi->aes_cxt.chunk_size  = aes_chain_size;
        fmi->aes_cxt.iv_func = h2fmi_aes_iv;
        fmi->aes_cxt.iv_func_arg = fmi;
        fmi->iv_seed_array = seeds;

        fmi->current_aes_cxt = &fmi->aes_cxt;
    }
    else
    {
        //WMR_PRINT(CRYPTO, "Disabling AES\n");
        fmi->current_aes_cxt = NULL;
        fmi->current_aes_iv_array = NULL;
    }
    return true;
}

void h2fmi_restoreFmiRegs(h2fmi_t *fmi)
{
    // Restore any register state that may have been lost
    // due to power gating
#if FMI_VERSION > 2
    restoreTimingRegs(fmi);
    turn_on_fmc(fmi);
#endif

#if FMISS_ENABLED
    if (!fmi->is_ppn)
    {
        fmiss_raw_init_sequences(fmi);
    }
    else
    {
        fmiss_init_sequences(fmi);
    }
#endif
}

// =============================================================================
// implementation function declarations
// =============================================================================

void h2fmi_iop_set_config(h2fmi_t* fmi, IOPFMI_SetConfig* cmd)
{
    // record which interface is being used
    dprintf(DEBUG_SPEW, "configuring FMI%d\n", cmd->fmi);
    fmi->regs   = (cmd->fmi == 0) ? FMI0 : FMI1;
    fmi->bus_id = cmd->fmi;

    // record necessary device geometry information
    fmi->bytes_per_spare      = cmd->bytes_per_spare;
    fmi->num_of_ce            = cmd->num_of_ce;
    fmi->pages_per_block      = cmd->pages_per_block;
    fmi->blocks_per_ce        = cmd->blocks_per_ce;
    fmi->total_bytes_per_meta = cmd->total_bytes_per_meta;
    fmi->valid_bytes_per_meta = cmd->valid_bytes_per_meta;
    fmi->bytes_per_page       = cmd->bytes_per_page;
    fmi->sectors_per_page     = fmi->bytes_per_page / H2FMI_BYTES_PER_SECTOR;
    fmi->valid_ces            = cmd->valid_ces;
    fmi->is_ppn               = cmd->ppn;
    fmi->logical_page_size    = cmd->logical_page_size;
    fmi->clock_speed_khz      = cmd->clock_speed_khz;

    // TODO: decide on appropriate infrastructure in support of
    // vendor-specific superblock formats and command sequences
    fmi->banks_per_ce = 1;

    // calculate and store bus interface timing control
    WMR_PRINT(INF, "cmd->read_sample_cycles : %d\n", cmd->read_sample_cycles);
    WMR_PRINT(INF, "cmd->read_setup_cycles  : %d\n", cmd->read_setup_cycles);
    WMR_PRINT(INF, "cmd->read_hold_cycles   : %d\n", cmd->read_hold_cycles);
    WMR_PRINT(INF, "cmd->write_setup_cycles : %d\n", cmd->write_setup_cycles);
    WMR_PRINT(INF, "cmd->write_hold_cycles  : %d\n", cmd->write_hold_cycles);

    fmi->if_ctrl = (FMC_IF_CTRL__DCCYCLE(cmd->read_sample_cycles) |
                    FMC_IF_CTRL__REB_SETUP(cmd->read_setup_cycles) |
                    FMC_IF_CTRL__REB_HOLD(cmd->read_hold_cycles) |
                    FMC_IF_CTRL__WEB_SETUP(cmd->write_setup_cycles) |
                    FMC_IF_CTRL__WEB_HOLD(cmd->write_hold_cycles));
                    
    WMR_PRINT(INF, "cmd->retire_on_invalid_refresh  : %d\n", cmd->retire_on_invalid_refresh);
    fmi->retire_on_invalid_refresh = cmd->retire_on_invalid_refresh;

#if SUPPORT_TOGGLE_NAND
    fmi->is_toggle_system = cmd->toggle_system;
    fmi->is_toggle = cmd->toggle;

    if (fmi->is_toggle)
    {
        WMR_PRINT(INF, "cmd->ce_hold_cycles     : %d\n", cmd->ce_hold_cycles);
        WMR_PRINT(INF, "cmd->ce_setup_cycles    : %d\n", cmd->ce_setup_cycles);
        WMR_PRINT(INF, "cmd->adl_cycles         : %d\n", cmd->adl_cycles);
        WMR_PRINT(INF, "cmd->whr_cycles         : %d\n", cmd->whr_cycles);
        WMR_PRINT(INF, "cmd->read_pre_cycles    : %d\n", cmd->read_pre_cycles);
        WMR_PRINT(INF, "cmd->read_post_cycles   : %d\n", cmd->read_post_cycles);
        WMR_PRINT(INF, "cmd->write_pre_cycles   : %d\n", cmd->write_pre_cycles);
        WMR_PRINT(INF, "cmd->write_post_cycles  : %d\n", cmd->write_post_cycles);
        WMR_PRINT(INF, "cmd->enable_diff_DQS    : %d\n", cmd->enable_diff_DQS);
        WMR_PRINT(INF, "cmd->enable_diff_RE     : %d\n", cmd->enable_diff_RE);
        WMR_PRINT(INF, "cmd->enable_VREF        : %d\n", cmd->enable_VREF);

        fmi->dqs_ctrl = cmd->reg_dqs_delay;
        fmi->timing_ctrl_1 = (FMC_TOGGLE_CTRL_1_DDR_RD_PRE_TIME(cmd->read_pre_cycles) |
                              FMC_TOGGLE_CTRL_1_DDR_RD_POST_TIME(cmd->read_post_cycles) |
                              FMC_TOGGLE_CTRL_1_DDR_WR_PRE_TIME(cmd->write_pre_cycles) |
                              FMC_TOGGLE_CTRL_1_DDR_WR_POST_TIME(cmd->write_post_cycles));
        fmi->timing_ctrl_2 = (FMC_TOGGLE_CTRL_2_CE_SETUP_TIME(cmd->ce_setup_cycles) |
                              FMC_TOGGLE_CTRL_2_CE_HOLD_TIME(cmd->ce_hold_cycles) |
                              FMC_TOGGLE_CTRL_2_NAND_TIMING_ADL(cmd->adl_cycles) |
                              FMC_TOGGLE_CTRL_2_NAND_TIMING_WHR(cmd->whr_cycles));
        fmi->toggle_if_ctrl = (FMC_IF_CTRL__DCCYCLE(0) |
                               FMC_IF_CTRL__REB_SETUP(cmd->dqs_half_cycles) |
                               FMC_IF_CTRL__REB_HOLD(0) |
                               FMC_IF_CTRL__WEB_SETUP(cmd->write_setup_cycles) |
                               FMC_IF_CTRL__WEB_HOLD(cmd->write_hold_cycles));
        fmi->useDiffDQS     = cmd->enable_diff_DQS;
        fmi->useDiffRE      = cmd->enable_diff_RE;
        fmi->useVREF        = cmd->enable_VREF;
    }
#endif

    h2fmi_restoreFmiRegs(fmi);

    // always report config information
    WMR_PRINT(INF, "fmi->regs                 : %p\n", fmi->regs);
    WMR_PRINT(INF, "fmi->valid_ces            : %p\n", fmi->valid_ces);
    WMR_PRINT(INF, "fmi->num_of_ce            : %p\n", fmi->num_of_ce);
    WMR_PRINT(INF, "fmi->banks_per_ce         : %p\n", fmi->banks_per_ce);
    WMR_PRINT(INF, "fmi->pages_per_block      : %p\n", fmi->pages_per_block);
    WMR_PRINT(INF, "fmi->sectors_per_page     : %p\n", fmi->sectors_per_page);
    WMR_PRINT(INF, "fmi->bytes_per_spare      : %p\n", fmi->bytes_per_spare);
    WMR_PRINT(INF, "fmi->blocks_per_ce        : %p\n", fmi->blocks_per_ce);
    WMR_PRINT(INF, "fmi->valid_bytes_per_meta : %p\n", fmi->valid_bytes_per_meta);
    WMR_PRINT(INF, "fmi->total_bytes_per_meta : %p\n", fmi->total_bytes_per_meta);
    WMR_PRINT(INF, "fmi->if_ctrl              : %p\n", fmi->if_ctrl);
#if SUPPORT_TOGGLE_NAND
    if (fmi->is_toggle)
    {
        WMR_PRINT(INF, "fmi->dqs_ctrl             : %p\n", fmi->dqs_ctrl);
        WMR_PRINT(INF, "fmi->timing_ctrl_1        : %p\n", fmi->timing_ctrl_1);
        WMR_PRINT(INF, "fmi->timing_ctrl_2        : %p\n", fmi->timing_ctrl_2);
        WMR_PRINT(INF, "fmi->toggle_if_ctrl       : %p\n", fmi->toggle_if_ctrl);
        WMR_PRINT(INF, "fmi->is_toggle            : %p\n", fmi->is_toggle);
    }
#endif
    WMR_PRINT(INF, "fmi->bytes_per_page       : %p\n", fmi->bytes_per_page);
    WMR_PRINT(INF, "fmi->is_ppn               : %p\n", fmi->is_ppn);

    // these steps are only taken when fmi is first configured
    if (!fmi->initialized)
    {
        // Make certain the FMC is enabled upon init.
        turn_on_fmc(fmi);

        // Perform system-specific initialization.
        h2fmi_init_sys(fmi);

        // indicate that fmi is initialized
        fmi->initialized = TRUE32;

        fmi->stateMachine.currentMode = fmiNone;
        fmi->wMaxOutstandingCEWriteOperations = cmd->wMaxOutstandingCEWriteOperations;
    }

    fmi->activeCe = ~0;
    if (fmi->is_ppn && (!fmi->is_ppn_channel_init))
    {
#if SUPPORT_PPN
        if (!h2fmi_ppn_init_channel(fmi,
                                    cmd->ppn_debug_flags_valid,
                                    cmd->ppn_debug_flags,
                                    cmd->ppn_vs_debug_flags_valid,
                                    cmd->ppn_vs_debug_flags,
                                    cmd->ppn_allow_saving_debug_data,
                                    cmd->ppn_version))
        {
            WMR_PANIC("[FIL:ERR] Unable to initialize PPN device\n");
            cmd->iopfmi.status = kIOPFMI_STATUS_FAILURE;
            return;
        }
        fmi->is_ppn_channel_init = TRUE32;
#else
        WMR_PANIC("!SUPPORT_PPN");
#endif
    }

    // Cache page config registers
    if (fmi->is_ppn)
    {
#if SUPPORT_PPN
        fmi->fmi_config_reg    = h2fmi_ppn_calculate_fmi_config(fmi);

#if FMI_VERSION > 0
        fmi->fmi_data_size_reg = h2fmi_ppn_calculate_fmi_data_size(fmi);
#endif /* FMI_VERSION > 0*/
#else //SUPPORT_PPN
        WMR_PANIC("!SUPPORT_PPN");
#endif //SUPPORT_PPN
    }
    else
    {
        // Cache ECC configuration
        fmi->correctable_bits = h2fmi_calculate_ecc_bits(fmi);

        fmi->fmi_config_reg    = h2fmi_calculate_fmi_config(fmi);
#if FMI_VERSION > 0
        fmi->fmi_data_size_reg = h2fmi_calculate_fmi_data_size(fmi);
#endif /* FMI_VERSION > 0*/
    }
    // record success status in command structure
    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_iop_read_chip_ids(h2fmi_t* fmi, IOPFMI_ReadChipIDs* cmd)
{
    IOPFMI_chipid_t* ids_buf = (IOPFMI_chipid_t*)mem_static_map_cached(cmd->chip_id_buffer);
    IOPFMI_status_t  status  = kIOPFMI_STATUS_SUCCESS;
    UInt32           valid_ces;
    UInt32           ce;

    WMR_ASSERT(ids_buf != MAP_FAILED);

    valid_ces = fmi->valid_ces;
    fmi->valid_ces = 0xffff;

    for (ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        h2fmi_chipid_t* chipid = (h2fmi_chipid_t*)&ids_buf[ce];
        UInt8 *ptr = (UInt8 *)&ids_buf[ce];

        if (!h2fmi_nand_read_id(fmi, ce, chipid, CHIPID_ADDR))
        {
            status = kIOPFMI_STATUS_FAILED_READ_ID;
            break;
        }

        WMR_PRINT(INF, "Read Chip ID %02X %02X %02X %02X %02X %02X to %p\n",
                  ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], chipid);
    }
    fmi->valid_ces = valid_ces;
    cmd->iopfmi.status = status;

    WMR_PREPARE_WRITE_BUFFER(ids_buf, H2FMI_MAX_CE_TOTAL * sizeof(IOPFMI_chipid_t));
}


void h2fmi_iop_reset_everything(h2fmi_t* fmi, IOPFMI_ResetEverything* cmd)
{
    IOPFMI_status_t status   = kIOPFMI_STATUS_SUCCESS;
    BOOL32 ret = TRUE32;

    h2fmi_reset(fmi);

    h2fmi_init_sys(fmi);

    ret = h2fmi_nand_reset_all(fmi);
    if (!ret)
    {
        status = kIOPFMI_STATUS_FAILED_RESET_ALL;
    }
}

void h2fmi_iop_erase_multiple(h2fmi_t* fmi, IOPFMI_EraseMultiple* cmd)
{
    BOOL32 erase_failed = FALSE32;

    WMR_PRINT(ERASE, "IOP received Multi-Erase command %d elements\n", cmd->number_of_elements);

    if (!h2fmi_erase_blocks(fmi,
                            cmd->number_of_elements,
                            cmd->ce,
                            cmd->block_number,
                            &erase_failed))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILED_ERASE_BLOCK;
    }
    else if (erase_failed)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_DEVICE_ERROR;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }

    /**
     * Copy erase status back out ...
     */
    cmd->failure_details.wNumCE_Executed = fmi->failureDetails.wNumCE_Executed;
    cmd->failure_details.wFirstFailingCE = fmi->failureDetails.wFirstFailingCE;
}

void h2fmi_iop_erase_single(h2fmi_t* fmi, IOPFMI_EraseSingle* cmd)
{
    BOOL32 erase_failed = FALSE32;

    WMR_PRINT(ERASE, "IOP received Erase command CE %d Block %d\n", cmd->ce, cmd->block_number);

    fmi->failureDetails.wCEStatusArray = &fmi->failureDetails.wSingleCEStatus;
    fmi->failureDetails.wCEStatusArrayModulusMask = 0;

    if (!h2fmi_erase_blocks(fmi,
                           1,
                           &cmd->ce,
                           &cmd->block_number,
                           &erase_failed))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILED_ERASE_BLOCK;
    }
    else if (erase_failed)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_DEVICE_ERROR;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
    /**
     * Copy erase status back out ...
     */
    cmd->failure_details.wNumCE_Executed = fmi->failureDetails.wNumCE_Executed;
    cmd->failure_details.wSingleCEStatus = fmi->failureDetails.wSingleCEStatus;
}

void h2fmi_iop_read_single(h2fmi_t* fmi, IOPFMI_IOSingle* cmd)
{
    BOOL32 gather_corrections = (0 != cmd->correction_address);

    UInt8* correct_buf = (gather_corrections
                          ? (UInt8*)mem_static_map_cached(cmd->correction_address)
                          : NULL);

    UInt8* data_buf = (UInt8*)cmd->data_address;
    UInt8* meta_buf = (UInt8*)cmd->meta_address;
    hfmi_aes_iv* aes_iv_array = NULL;

    WMR_ASSERT(correct_buf != MAP_FAILED);

    WMR_PRINT(READ, "IOP received Read Single command CE %d Block %d\n", cmd->ce, cmd->page_number);

    if (cmd->aes_iv_array)
    {
        aes_iv_array = (hfmi_aes_iv*)mem_static_map_cached(cmd->aes_iv_array);
        WMR_ASSERT(aes_iv_array != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(aes_iv_array, cmd->aes_num_chains * sizeof(hfmi_aes_iv));
    }

    h2fmi_setup_aes(fmi, &cmd->iopfmi, aes_iv_array, cmd->aes_chain_size, cmd->aes_key_type, (UInt32*)cmd->aes_key_bytes, &cmd->page_number);

    cmd->iopfmi.status = h2fmi_read_page(fmi,
                                         cmd->ce,
                                         cmd->page_number,
                                         data_buf,
                                         meta_buf,
                                         correct_buf,
                                         NULL);
    
    // clean corrections buffer, if not NULL
    if (gather_corrections)
    {
        WMR_PREPARE_WRITE_BUFFER(correct_buf,
                                 fmi->sectors_per_page * sizeof(IOPFMI_correction_t));
    }
}

void h2fmi_iop_read_multiple(h2fmi_t* fmi, IOPFMI_IOMultiple* cmd)
{
    const UInt32 page_count   = cmd->page_count;

    BOOL32 gather_corrections = (0 != cmd->corrections_array);
    UInt8* corrections_array  = (gather_corrections
                                 ? (UInt8*)mem_static_map_cached(cmd->corrections_array)
                                 : NULL);

    h2fmi_ce_t* chip_enable_array = (h2fmi_ce_t*)mem_static_map_cached(cmd->chip_enable_array);
    UInt32* page_number_array = (UInt32*)mem_static_map_cached(cmd->page_number_array);
    hfmi_aes_iv* aes_iv_array = NULL;


    WMR_ASSERT(corrections_array != MAP_FAILED);
    WMR_ASSERT(chip_enable_array != MAP_FAILED);
    WMR_ASSERT(page_number_array != MAP_FAILED);

    if (cmd->aes_iv_array)
    {
        aes_iv_array = (hfmi_aes_iv*)mem_static_map_cached(cmd->aes_iv_array);
        WMR_ASSERT(aes_iv_array != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(aes_iv_array, cmd->aes_num_chains * sizeof(hfmi_aes_iv));
    }

    // Invalidate control arrays, ensuring that these values are
    // re-read from physical memory.
    WMR_PREPARE_READ_BUFFER(chip_enable_array, page_count * sizeof(UInt32));
    WMR_PREPARE_READ_BUFFER(page_number_array, page_count * sizeof(UInt32));

    UInt32* data_segments_array = (UInt32*)mem_static_map_cached(cmd->data_segments_array);
    UInt32* meta_segments_array = (UInt32*)mem_static_map_cached(cmd->meta_segments_array);
    WMR_ASSERT(data_segments_array != MAP_FAILED);
    WMR_ASSERT(meta_segments_array != MAP_FAILED);
    
    WMR_PREPARE_READ_BUFFER(data_segments_array, cmd->data_segment_array_length_in_bytes);
    WMR_PREPARE_READ_BUFFER(meta_segments_array, cmd->meta_segment_array_length_in_bytes);

    h2fmi_setup_aes(fmi, &cmd->iopfmi, aes_iv_array, cmd->aes_chain_size, cmd->aes_key_type, (UInt32*)cmd->aes_key_bytes, page_number_array);

    WMR_PRINT(READ, "IOP received Read Multiple command NumPages %d\n", page_count);

    // perform multi-read
    cmd->iopfmi.status = h2fmi_read_multi(fmi,
                                          page_count,
                                          chip_enable_array,
                                          page_number_array,
                                          (void*)data_segments_array,
                                          (void*)meta_segments_array,
                                          corrections_array,
                                          NULL);

    // clean corrections buffer, if not NULL
    if (gather_corrections)
    {
        WMR_PREPARE_WRITE_BUFFER(corrections_array,
                                 page_count * fmi->sectors_per_page * sizeof(IOPFMI_correction_t));
    }
    /**
     * Copy failure data.
     */
    cmd->failure_details.wNumCE_Executed = fmi->failureDetails.wNumCE_Executed;
    cmd->failure_details.wOverallOperationFailure = fmi->failureDetails.wOverallOperationFailure;
    cmd->failure_details.wFirstFailingCE = fmi->failureDetails.wFirstFailingCE;
#if (defined (WMR_DEBUG) && WMR_DEBUG)
    if ( (kIOPFMI_STATUS_SUCCESS != cmd->iopfmi.status) &&
         (kIOPFMI_STATUS_BLANK   != cmd->iopfmi.status) &&
         (kIOPFMI_STATUS_FUSED   != cmd->iopfmi.status))
    {
        unsigned k = fmi->failureDetails.wCEStatusArrayModulusMask;
        WMR_PRINT(ERROR, "IOP --> err 0x%08x, %d completed on %d\n",cmd->iopfmi.status, fmi->failureDetails.wNumCE_Executed,fmi->bus_id);
        while ( k>0 )
        {
            WMR_PRINT(ERROR, "Status[%d] = 0x%x\n",k,fmi->failureDetails.wCEStatusArray[k]);
            k--;
        }
    }
#endif
}

void h2fmi_iop_write_single(h2fmi_t* fmi, IOPFMI_IOSingle* cmd)
{
    UInt8* data_buf = (UInt8*)cmd->data_address;
    UInt8* meta_buf = (UInt8*)cmd->meta_address;
    BOOL32 write_failed = FALSE32;
    hfmi_aes_iv* aes_iv_array = NULL;

    if (cmd->aes_iv_array)
    {
        aes_iv_array = (hfmi_aes_iv*)mem_static_map_cached(cmd->aes_iv_array);
        WMR_ASSERT(aes_iv_array != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(aes_iv_array, cmd->aes_num_chains * sizeof(hfmi_aes_iv));
    }
    WMR_PRINT(WRITE, "IOP received Write Single command CE %d Block %d\n", cmd->ce, cmd->page_number);

    h2fmi_setup_aes(fmi, &cmd->iopfmi, aes_iv_array, cmd->aes_chain_size, cmd->aes_key_type, (UInt32*)cmd->aes_key_bytes, &cmd->page_number);

    if (!h2fmi_write_page(fmi,
                          cmd->ce,
                          cmd->page_number,
                          data_buf,
                          meta_buf,
                          &write_failed))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILED_WRITE_PAGE;
    }
    else if (write_failed)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_DEVICE_ERROR;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
}

void h2fmi_iop_write_multiple(h2fmi_t* fmi, IOPFMI_IOMultiple* cmd)
{
    const UInt32 page_count   = cmd->page_count;

    h2fmi_ce_t* chip_enable_array = (h2fmi_ce_t*)mem_static_map_cached(cmd->chip_enable_array);
    UInt32* page_number_array = (UInt32*)mem_static_map_cached(cmd->page_number_array);

    BOOL32 write_failed = FALSE32;
    hfmi_aes_iv* aes_iv_array = NULL;

    WMR_ASSERT(chip_enable_array != MAP_FAILED);
    WMR_ASSERT(page_number_array != MAP_FAILED);

    if (cmd->aes_iv_array)
    {
        aes_iv_array = (hfmi_aes_iv*)mem_static_map_cached(cmd->aes_iv_array);
        WMR_ASSERT(aes_iv_array != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(aes_iv_array, cmd->aes_num_chains * sizeof(hfmi_aes_iv));
    }
    // Invalidate control arrays, ensuring that these values are
    // re-read from physical memory.
    WMR_PREPARE_READ_BUFFER(chip_enable_array, page_count * sizeof(UInt32));
    WMR_PREPARE_READ_BUFFER(page_number_array, page_count * sizeof(UInt32));

    UInt32* data_segments_array = (UInt32*)mem_static_map_cached(cmd->data_segments_array);
    UInt32* meta_segments_array = (UInt32*)mem_static_map_cached(cmd->meta_segments_array);

    WMR_ASSERT(data_segments_array != MAP_FAILED);
    WMR_ASSERT(meta_segments_array != MAP_FAILED);
    WMR_PREPARE_READ_BUFFER(data_segments_array, cmd->data_segment_array_length_in_bytes);
    WMR_PREPARE_READ_BUFFER(meta_segments_array, cmd->meta_segment_array_length_in_bytes);

    WMR_PRINT(WRITE, "IOP received Write Multiple command NumPages %d\n", page_count);

    h2fmi_setup_aes(fmi, &cmd->iopfmi, aes_iv_array, cmd->aes_chain_size, cmd->aes_key_type, (UInt32*)cmd->aes_key_bytes, page_number_array);

    // perform multi-write
    if (!h2fmi_write_multi(fmi,
                           page_count,
                           chip_enable_array,
                           page_number_array,
                           (void*)data_segments_array,
                           (void*)meta_segments_array,
                           &write_failed,
                           cmd->vendorProtocol))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILED_WRITE_MULTI;
    }
    else if (write_failed)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_DEVICE_ERROR;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
    /**
     * Copy failure data.
     */
    cmd->failure_details.wNumCE_Executed = fmi->failureDetails.wNumCE_Executed;
    cmd->failure_details.wFirstFailingCE = fmi->failureDetails.wFirstFailingCE;
}

void h2fmi_iop_read_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd)
{
    UInt8* page_buf  = (UInt8*)(cmd->buffer_address);

    UInt8* data_buf  = page_buf + 0;
    UInt8* spare_buf = page_buf + fmi->bytes_per_page;

    fmi->current_aes_cxt = NULL;

    WMR_PRINT(READ, "IOP received Read Raw command CE %d Page %d\n", cmd->ce, cmd->page_number);

    if (!h2fmi_read_raw_page(fmi,
                             cmd->ce,
                             cmd->page_number,
                             data_buf,
                             spare_buf))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILED_READ_RAW;
    }
    else
    {
            cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
}

void h2fmi_iop_write_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd)
{
    const UInt32 data_bytes_per_page = (fmi->sectors_per_page * H2FMI_BYTES_PER_SECTOR);
    const UInt32 bytes_per_page = data_bytes_per_page + fmi->bytes_per_spare;
    UInt8* page_buf = (UInt8*)cmd->buffer_address;
    UInt8* data_buf = page_buf;

    // deal with cache coherency issues
    const UInt32 data_clean_size = bytes_per_page;

    fmi->current_aes_cxt = NULL;

    WMR_PRINT(WRITE, "IOP sending write raw ce %d page %d data_buf %08x size %d\n",
              cmd->ce,
              cmd->page_number,
              data_buf,
              bytes_per_page);

    // Invalidate data and meta buffers, ensuring their
    // contents are re-read from physical memory
    WMR_PREPARE_READ_BUFFER(data_buf, data_clean_size);

    cmd->iopfmi.status = h2fmi_write_raw_page(fmi,
                                              cmd->ce,
                                              cmd->page_number,
                                              data_buf);
}

void h2fmi_iop_read_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd)
{
    BOOL32 gather_corrections = (0 != cmd->corrections_array);

    UInt8* corrections_array = (gather_corrections
                                ? (UInt8*)mem_static_map_cached(cmd->corrections_array)
                                : NULL);

    UInt8* page_buf  = (UInt8*)cmd->buffer_address;

    WMR_ASSERT(corrections_array != MAP_FAILED);

    fmi->current_aes_cxt = NULL;

    WMR_PRINT(READ, "IOP received Read Boot Page command CE %d Page %d\n", cmd->ce, cmd->page_number);

    cmd->iopfmi.status = h2fmi_read_bootpage(fmi,
                                             cmd->ce,
                                             cmd->page_number,
                                             page_buf,
                                             corrections_array);

    // clean corrections buffer, if not NULL
    if (gather_corrections)
    {
        WMR_PREPARE_WRITE_BUFFER(corrections_array,
                                 fmi->sectors_per_page * sizeof(IOPFMI_correction_t));
    }
}

void h2fmi_iop_write_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd)
{
    UInt8* page_buf = (UInt8*)mem_static_map_cached(cmd->buffer_address);

    WMR_ASSERT(page_buf != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(page_buf, H2FMI_BOOT_BYTES_PER_PAGE);

    fmi->current_aes_cxt = NULL;

    WMR_PRINT(WRITE, "IOP received Write Boot Page command CE %d Page %d\n", cmd->ce, cmd->page_number);

    cmd->iopfmi.status = h2fmi_write_bootpage(fmi,
                                              cmd->ce,
                                              cmd->page_number,
                                              page_buf);
}



void h2fmi_iop_sleep(h2fmi_t* fmi)
{
    dprintf(DEBUG_CRITICAL, "sleeping FMI%d\n", h2fmi_bus_index(fmi));
}

void h2fmi_iop_wake(h2fmi_t* fmi)
{
    dprintf(DEBUG_CRITICAL, "waking FMI%d\n", h2fmi_bus_index(fmi));
}

void dump_fmi_state(h2fmi_t *pFMI, uint32_t index, BOOL32 withHWRegs, BOOL32 withECC)
{
    /**
     * Dump 'important' parts of state machine ...
     */
    printf("FMI[%u] - stateMachine{mode:%d, state: %d, CEsav: %d, page_idx: %d, page_count: %d, currentCE: %d, currentPage: 0x%x, fSuccessful: %d, lastCE: %d, needsPrepare: %d, cleanPages: %d, uecc_pages: %d, cmdStartTick: 0x%llx, wVendorType: %d}\n",
           index,
           (int)pFMI->stateMachine.currentMode,
           (int)pFMI->stateMachine.state.rd,
           pFMI->stateMachine.savedCurrentCE,
           pFMI->stateMachine.page_idx,
           pFMI->stateMachine.page_count,
           pFMI->stateMachine.currentCE,
           pFMI->stateMachine.currentPage,
           (int)pFMI->stateMachine.fSuccessful,
           pFMI->stateMachine.lastCE,
           pFMI->stateMachine.needs_prepare,
           pFMI->stateMachine.clean_pages,
           pFMI->stateMachine.uecc_pages,
           pFMI->stateMachine.wCmdStartTicks,
           pFMI->stateMachine.wVendorType);

    /**
     * Dump failure details.
     */
    printf("FMI[%u] - failureDetails{wNumCEexec:%d, hldoff: %d, outstd: %d/%d, overall: 0x%x, firstFailingCE: 0x%x, wSingleCEStatus: 0x%x}\n",
           index,
           pFMI->failureDetails.wNumCE_Executed,
           pFMI->stateMachine.wNumTimesHoldoffExecuted,
           pFMI->stateMachine.wOutstandingCEWriteOperations,
           pFMI->stateMachine.wMaxOutstandingCEWriteOperations,
           pFMI->failureDetails.wOverallOperationFailure,
           pFMI->failureDetails.wFirstFailingCE,
           pFMI->failureDetails.wSingleCEStatus
           );
           
    if (withHWRegs)
    {
        /**
         * Dump important FMI registers ...
         */
#if FMI_VERSION <=2
        printf("FMI[%u] - fmi{bus: %d, CFG: 0x%x, CTRL: 0x%x, STAT: 0x%x, INTPND: 0x%x, INT_EN: 0x%x, DBG0: 0x%x, DBG1: 0x%x, DBG2: 0x%x}\n",
               index,
               pFMI->bus_id,
               h2fmi_rd(pFMI,FMI_CONFIG),
               h2fmi_rd(pFMI,FMI_CONTROL),
               h2fmi_rd(pFMI,FMI_STATUS),
               h2fmi_rd(pFMI,FMI_INT_PEND),
               h2fmi_rd(pFMI,FMI_INT_EN),
               h2fmi_rd(pFMI,FMI_DEBUG0),
               h2fmi_rd(pFMI,FMI_DEBUG1),
               h2fmi_rd(pFMI,FMI_DEBUG2)
               );
#else
        printf("FMI[%u] - fmi{bus: %d, CFG: 0x%x, CTRL: 0x%x, STAT: 0x%x, INTPND: 0x%x, INT_EN: 0x%x, DBG0: 0x%x, DATSIZ: 0x%x}\n",
               index,
               pFMI->bus_id,
               h2fmi_rd(pFMI,FMI_CONFIG),
               h2fmi_rd(pFMI,FMI_CONTROL),
               h2fmi_rd(pFMI,FMI_STATUS),
               h2fmi_rd(pFMI,FMI_INT_PEND),
               h2fmi_rd(pFMI,FMI_INT_EN),
               h2fmi_rd(pFMI,FMI_DEBUG0),
               h2fmi_rd(pFMI,FMI_DATA_SIZE)
               );
#endif

#if ((FMI_VERSION > 0) && (FMI_VERSION <= 2))
        printf("FMI[%u] - fmi{DBG3: 0x%x, DATSIZ: 0x%x}\n",index,h2fmi_rd(pFMI,FMI_DEBUG3),h2fmi_rd(pFMI,FMI_DATA_SIZE));
#endif
        /**
         * Dump important FMC registers ...
         */
        printf("FMI[%u] - fmc{ON: 0x%x, IFCTRL: 0x%x, CECTRL: 0x%x, RWCTRL: 0x%x, CMD: 0x%x, ADDR0: 0x%x, ADDR1: 0x%x, ANUM: 0x%x, DNUM: 0x%x, FSTATUS: 0x%x, NSTATUS: 0x%x, RBB_CFG: 0x%x}\n",
               index,
               h2fmi_rd(pFMI,FMC_ON),
               h2fmi_rd(pFMI,FMC_IF_CTRL),
               h2fmi_rd(pFMI,FMC_CE_CTRL),
               h2fmi_rd(pFMI,FMC_RW_CTRL),
               h2fmi_rd(pFMI,FMC_CMD),
               h2fmi_rd(pFMI,FMC_ADDR0),
               h2fmi_rd(pFMI,FMC_ADDR1),
               h2fmi_rd(pFMI,FMC_ADDRNUM),
               h2fmi_rd(pFMI,FMC_DATANUM),
               h2fmi_rd(pFMI,FMC_STATUS),
               h2fmi_rd(pFMI,FMC_NAND_STATUS),
               h2fmi_rd(pFMI,FMC_RBB_CONFIG)
               );

#if FMI_VERSION > 2
        printf("FMI[%u] - fmc{DQST: 0x%x, NANDT1: 0x%x, NANDT2: 0x%x, NANDT3: 0x%x}\n",
               index,
               h2fmi_rd(pFMI,FMC_DQS_TIMING_CTRL),
               h2fmi_rd(pFMI,FMC_TOGGLE_CTRL_1),
               h2fmi_rd(pFMI,FMC_TOGGLE_CTRL_2),
               h2fmi_rd(pFMI,FMC_TOGGLE_CTRL_3)
               );
#endif // FMI_VERSION > 2
    
        // ECC might be clocked off for PPN
        if (withECC)
        {
            /**
            * Dump important ECC registers ...
            */
            printf("FMI[%u] - ecc{PND: 0x%x, MASK: 0x%x}\n",
                index,
                h2fmi_rd(pFMI,ECC_PND),
                h2fmi_rd(pFMI,ECC_MASK)
                );
        }
        else
        {
            printf("FMI[%u} - omitting ecc registers for ppn device\n", index);
        }

#if FMISS_DUMP_ENABLED
        // FMISS
        if (0 != (FMI_CONTROL__SEQUENCER_TIMEOUT_ENABLE & h2fmi_rd(pFMI, FMI_CONTROL)))
        {
            fmiss_dump(pFMI);
        }
#endif // FMISS_DUMP_ENABLED
    }
}
