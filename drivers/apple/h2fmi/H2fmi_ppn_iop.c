#include "H2fmi_iop.h"
#include "H2fmi_private.h"
#include "H2fmi_ppn.h"

#include <platform/memmap.h>
#include <platform.h>
#include <drivers/dma.h>
#include <drivers/aes.h>
#include <debug.h>

#include "PPN_FIL.h"

#if SUPPORT_PPN

void h2fmi_ppn_iop_reset_everything(h2fmi_t* fmi, IOPFMI_ResetEverything* cmd)
{
    h2fmi_restoreFmiRegs(fmi);

    h2fmi_iop_reset_everything(fmi, cmd);
    fmi->ppn->general_error = FALSE32;
}

void h2fmi_ppn_iop_erase_multiple(h2fmi_t *fmi, IOPFMI_IOPpn* cmd)
{
    PPNCommandStruct *ppnCommand;

    ppnCommand = (PPNCommandStruct *)mem_static_map_cached(cmd->ppn_fil_command);

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    WMR_ASSERT(ppnCommand != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(ppnCommand, cmd->ppn_fil_size);

    h2fmi_ppn_erase_blocks(fmi, ppnCommand);

    WMR_PREPARE_WRITE_BUFFER(ppnCommand, cmd->ppn_fil_size);
    cmd->geb_ce = fmi->ppn->general_error_ce;
    //WMR_PRINT(ALWAYS, "Erase Multiple: status 0x%02x\n", ppnCommand->page_status_summary);
}

static PPNCommandStruct singlePpnCommands[kIOPFMI_MAX_NUM_OF_CHANNELS];

void h2fmi_ppn_iop_erase_single(h2fmi_t *fmi, IOPFMI_EraseSingle* cmd)
{
    PPNCommandStruct   *ppnCommand;
    const UInt32        pageAddr = cmd->block_number << fmi->ppn->device_params.page_address_bits;
    Int32               status;

    // We shouldn't use this command going forward - it's a legacy command supported
    // for boot-from-NAND code only.

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    ppnCommand = &singlePpnCommands[fmi->bus_id];

    WMR_MEMSET(ppnCommand, 0, sizeof(PPNCommandStruct));

    ppnCommand->context.device_info = NULL;
    ppnCommand->context.handle      = NULL;
    ppnCommand->context.bus_num     = fmi->bus_id;

    ppnCommand->command                         = PPN_COMMAND_ERASE;
    ppnCommand->options                         = 0;
    ppnCommand->entry[0].ceIdx                  = 0;
    ppnCommand->entry[0].addr.row               = pageAddr;
    ppnCommand->mem_index[0].offset             = 0;
    ppnCommand->mem_index[0].idx                = 0;
    ppnCommand->entry[0].lbas                   = 0;
    ppnCommand->ceInfo[0].pages                 = 1;
    ppnCommand->ceInfo[0].ce                    = cmd->ce;
    ppnCommand->num_pages                       = 1;

    status = h2fmi_ppn_erase_blocks(fmi, ppnCommand);
    if (status != kIOPFMI_STATUS_SUCCESS)
    {
        cmd->iopfmi.status = status;
    }
    else if (ppnCommand->page_status_summary == 0x40)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_ERASE_ERROR;
    }
}

void h2fmi_ppn_iop_read_single(h2fmi_t *fmi, IOPFMI_IOSingle* cmd)
{
    PPNCommandStruct   *ppnCommand;
    const UInt32        pageAddr = cmd->page_number;
    UInt8              *dataBuf = (UInt8 *)mem_static_map_physical(cmd->data_address);
    UInt8              *metaBuf = (UInt8 *)mem_static_map_physical(cmd->meta_address);
    struct dma_segment  dataSegment;
    struct dma_segment  metaSegment;

    WMR_ASSERT(dataBuf != MAP_FAILED);
    WMR_ASSERT(metaBuf != MAP_FAILED);

    // We shouldn't use this command going forward - it's a legacy command supported
    // for boot-from-NAND code only.

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    fmi->current_aes_cxt = NULL;

    ppnCommand = &singlePpnCommands[fmi->bus_id];

    WMR_MEMSET(ppnCommand, 0, sizeof(PPNCommandStruct));

    ppnCommand->context.device_info            = NULL;
    ppnCommand->context.handle                 = NULL;
    ppnCommand->context.bus_num                = fmi->bus_id;

    ppnCommand->command                        = PPN_COMMAND_READ;
    ppnCommand->options                        = 0;

    ppnCommand->entry[0].ceIdx                 = 0;
    ppnCommand->entry[0].addr.length           = fmi->bytes_per_page + fmi->valid_bytes_per_meta;
    ppnCommand->entry[0].addr.column           = 0;
    ppnCommand->entry[0].addr.row              = pageAddr;
    ppnCommand->mem_index[0].offset            = 0;
    ppnCommand->mem_index[0].idx               = 0;
    ppnCommand->entry[0].lbas                  = fmi->bytes_per_page / fmi->logical_page_size;
    ppnCommand->ceInfo[0].pages                = 1;
    ppnCommand->ceInfo[0].ce                   = cmd->ce;
    ppnCommand->num_pages                      = 1;
    ppnCommand->lbas                           = fmi->bytes_per_page / fmi->logical_page_size;

    dataSegment.paddr = (UInt32)dataBuf;
    dataSegment.length = fmi->bytes_per_page;
    metaSegment.paddr = (UInt32)metaBuf;
    metaSegment.length = ppnCommand->lbas * fmi->valid_bytes_per_meta;

#if FMISS_ENABLED
    cmd->iopfmi.status = fmiss_ppn_read_multi(fmi, ppnCommand, &dataSegment, &metaSegment);
#else
    cmd->iopfmi.status = h2fmi_ppn_read_multi(fmi, ppnCommand, &dataSegment, &metaSegment);
#endif
}

void h2fmi_ppn_iop_read_multiple(h2fmi_t *fmi, IOPFMI_IOPpn* cmd)
{
    PPNCommandStruct *ppnCommand;
    UInt32           *data_segments_array;
    UInt32           *meta_segments_array;
    hfmi_aes_iv      *aes_iv_array = NULL;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    ppnCommand          = (PPNCommandStruct *)mem_static_map_cached(cmd->ppn_fil_command);
    data_segments_array = (UInt32 *)mem_static_map_cached(cmd->data_segments_array);
    meta_segments_array = (UInt32 *)mem_static_map_cached(cmd->meta_segments_array);

    WMR_ASSERT(ppnCommand != MAP_FAILED);
    WMR_ASSERT(data_segments_array != MAP_FAILED);
    WMR_ASSERT(meta_segments_array != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(ppnCommand, cmd->ppn_fil_size);
    WMR_PREPARE_READ_BUFFER(data_segments_array, cmd->data_segments_array_length);
    WMR_PREPARE_READ_BUFFER(meta_segments_array, cmd->meta_segments_array_length);

    if (cmd->aes_iv_array)
    {
        aes_iv_array = (hfmi_aes_iv *)mem_static_map_cached(cmd->aes_iv_array);
        WMR_ASSERT(aes_iv_array != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(aes_iv_array, cmd->aes_num_chains * sizeof(hfmi_aes_iv));
    }
    h2fmi_setup_aes(fmi, &cmd->iopfmi, aes_iv_array, cmd->aes_chain_size, cmd->aes_key_type, (UInt32 *)cmd->aes_key_bytes, NULL);

#if FMISS_ENABLED
    cmd->iopfmi.status = fmiss_ppn_read_multi(fmi,
                                              ppnCommand,
                                              (void *)data_segments_array,
                                              (void *)meta_segments_array);
#else
    cmd->iopfmi.status = h2fmi_ppn_read_multi(fmi,
                                              ppnCommand,
                                              (void *)data_segments_array,
                                              (void *)meta_segments_array);
#endif
    cmd->geb_ce = fmi->ppn->general_error_ce;

    WMR_PREPARE_WRITE_BUFFER(ppnCommand, cmd->ppn_fil_size);
}


void h2fmi_ppn_iop_write_single(h2fmi_t *fmi, IOPFMI_IOSingle* cmd)
{
    PPNCommandStruct   *ppnCommand;
    const UInt32        pageAddr = cmd->page_number;
    UInt8              *dataBuf = (UInt8 *)mem_static_map_physical(cmd->data_address);
    UInt8              *metaBuf = (UInt8 *)mem_static_map_physical(cmd->meta_address);
    struct dma_segment  dataSegment;
    struct dma_segment  metaSegment;

    WMR_ASSERT(dataBuf != MAP_FAILED);
    WMR_ASSERT(metaBuf != MAP_FAILED);

    // We shouldn't use this command going forward - it's a legacy command supported
    // for boot-from-NAND code only.

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    fmi->current_aes_cxt = NULL;

    ppnCommand = &singlePpnCommands[fmi->bus_id];

    WMR_MEMSET(ppnCommand, 0, sizeof(PPNCommandStruct));

    ppnCommand->context.device_info             = NULL;
    ppnCommand->context.handle                  = NULL;
    ppnCommand->context.bus_num                 = fmi->bus_id;
    ppnCommand->command                         = PPN_COMMAND_PROGRAM;
    ppnCommand->options                         = 0;
    ppnCommand->entry[0].ceIdx                  = 0;
    ppnCommand->entry[0].addr.row               = pageAddr;
    ppnCommand->mem_index[0].offset             = 0;
    ppnCommand->mem_index[0].idx                = 0;
    ppnCommand->entry[0].lbas                   = fmi->bytes_per_page / fmi->logical_page_size;
    ppnCommand->ceInfo[0].pages                 = 1;
    ppnCommand->ceInfo[0].ce                    = cmd->ce;
    ppnCommand->num_pages                       = 1;
    ppnCommand->lbas                            = fmi->bytes_per_page / fmi->logical_page_size;
    dataSegment.paddr = (UInt32)dataBuf;
    dataSegment.length = fmi->bytes_per_page;
    metaSegment.paddr = (UInt32)metaBuf;
    metaSegment.length = ppnCommand->lbas * fmi->valid_bytes_per_meta;

#if FMISS_ENABLED
    cmd->iopfmi.status = fmiss_ppn_write_multi(fmi, ppnCommand, &dataSegment, &metaSegment);
#else
    cmd->iopfmi.status = h2fmi_ppn_write_multi(fmi, ppnCommand, &dataSegment, &metaSegment);
#endif
}

void h2fmi_ppn_iop_write_multiple(h2fmi_t *fmi, IOPFMI_IOPpn* cmd)
{
    PPNCommandStruct *ppnCommand;
    UInt32           *data_segments_array;
    UInt32           *meta_segments_array;
    hfmi_aes_iv     *aes_iv_array = NULL;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    ppnCommand = (PPNCommandStruct *)mem_static_map_cached(cmd->ppn_fil_command);
    data_segments_array = (UInt32 *)mem_static_map_cached(cmd->data_segments_array);
    meta_segments_array = (UInt32 *)mem_static_map_cached(cmd->meta_segments_array);

    WMR_ASSERT(ppnCommand != MAP_FAILED);
    WMR_ASSERT(data_segments_array != MAP_FAILED);
    WMR_ASSERT(meta_segments_array != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(ppnCommand, cmd->ppn_fil_size);
    WMR_PREPARE_READ_BUFFER(data_segments_array, cmd->data_segments_array_length);
    WMR_PREPARE_READ_BUFFER(meta_segments_array, cmd->meta_segments_array_length);

    if (cmd->aes_iv_array)
    {
        aes_iv_array = (hfmi_aes_iv *)mem_static_map_cached(cmd->aes_iv_array);
        WMR_ASSERT(aes_iv_array != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(aes_iv_array, cmd->aes_num_chains * sizeof(hfmi_aes_iv));
    }
    h2fmi_setup_aes(fmi, &cmd->iopfmi, aes_iv_array, cmd->aes_chain_size, cmd->aes_key_type, (UInt32 *)cmd->aes_key_bytes, NULL);

#if FMISS_ENABLED
    cmd->iopfmi.status = fmiss_ppn_write_multi(fmi,
                                               ppnCommand,
                                               (void *)data_segments_array,
                                               (void *)meta_segments_array);
#else
    cmd->iopfmi.status = h2fmi_ppn_write_multi(fmi,
                                               ppnCommand,
                                               (void *)data_segments_array,
                                               (void *)meta_segments_array);
#endif
    cmd->geb_ce = fmi->ppn->general_error_ce;
    WMR_PREPARE_WRITE_BUFFER(ppnCommand, cmd->ppn_fil_size);
}

void h2fmi_ppn_iop_read_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd)
{
    UInt8 *page_buf = (UInt8 *)cmd->buffer_address;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    WMR_MEMSET(page_buf, 0xFF, fmi->bytes_per_page + fmi->bytes_per_spare);
    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_ppn_iop_write_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd)
{
    panic("Write Raw means nothing on PPN");
}

void h2fmi_ppn_iop_read_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd)
{
    const BOOL32 gather_corrections = (0 != cmd->corrections_array);
    UInt8 *corrections_array = (gather_corrections
                                ? (UInt8 *)mem_static_map_cached(cmd->corrections_array)
                                : NULL);
    UInt8 *page_buf = (UInt8 *)mem_static_map_cached(cmd->buffer_address);

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    WMR_ASSERT(corrections_array != MAP_FAILED);
    fmi->current_aes_cxt = NULL;

    cmd->iopfmi.status = h2fmi_ppn_read_bootpage(fmi,
                                                 cmd->ce,
                                                 cmd->page_number,
                                                 page_buf,
                                                 corrections_array);

    WMR_PREPARE_WRITE_BUFFER(page_buf, H2FMI_BOOT_BYTES_PER_PAGE);

    if (gather_corrections)
    {
        WMR_PREPARE_WRITE_BUFFER(corrections_array,
                                 fmi->sectors_per_page * sizeof(IOPFMI_correction_t));
    }
}

void h2fmi_ppn_iop_write_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd)
{
    UInt8 *page_buf;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    page_buf = (UInt8 *)mem_static_map_physical(cmd->buffer_address);
    WMR_ASSERT(page_buf != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(page_buf, H2FMI_BOOT_BYTES_PER_PAGE);

    fmi->current_aes_cxt = NULL;

    cmd->iopfmi.status = h2fmi_ppn_write_bootpage(fmi,
                                                  cmd->ce,
                                                  cmd->page_number,
                                                  page_buf);
}

void h2fmi_ppn_iop_read_cau_bbt(h2fmi_t *fmi, IOPFMI_IOPpn* cmd)
{
    PPNCommandStruct   *ppnCommand;
    struct dma_segment *sgl;
    UInt8              *buf;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    ppnCommand = (PPNCommandStruct *)mem_static_map_cached(cmd->ppn_fil_command);
    sgl = (struct dma_segment *)mem_static_map_cached(cmd->data_segments_array);
    WMR_ASSERT(ppnCommand != MAP_FAILED);
    WMR_ASSERT(sgl != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(ppnCommand, cmd->ppn_fil_size);
    WMR_PREPARE_READ_BUFFER(sgl, cmd->data_segments_array_length);

    buf = (UInt8 *)mem_static_map_cached(sgl[0].paddr);
    WMR_ASSERT(buf != MAP_FAILED);

    cmd->iopfmi.status = h2fmi_ppn_read_cau_bbt(fmi, ppnCommand, buf);
    cmd->geb_ce = fmi->ppn->general_error_ce;

    WMR_PREPARE_WRITE_BUFFER(buf, fmi->ppn->device_params.blocks_per_cau / 8);
    WMR_PREPARE_WRITE_BUFFER(ppnCommand, cmd->ppn_fil_size);
}

void h2fmi_ppn_iop_get_temperature(h2fmi_t *fmi, IOPFMI_GetTemperature* cmd)
{
    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

#if SUPPORT_TOGGLE_NAND
    if (h2fmi_ppn15_get_temperature(fmi, 0, &(cmd->temperature_celsius)))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILURE;
    }
#else
    cmd->iopfmi.status = kIOPFMI_STATUS_FAILURE;
#endif
}

void h2fmi_ppn_iop_get_controller_info(h2fmi_t *fmi, IOPFMI_GetControllerInfo* cmd)
{
    h2fmi_ppn_device_params_t *params = &fmi->ppn->device_params;
    BOOL32 result;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    h2fmi_ppn_get_fw_version(fmi, cmd->ce, cmd->fw_version);
    h2fmi_ppn_get_package_assembly_code(fmi, cmd->ce, cmd->package_assembly_code);
    h2fmi_ppn_get_controller_unique_id(fmi, cmd->ce, cmd->controller_unique_id);
    h2fmi_ppn_get_controller_hw_id(fmi, cmd->ce, cmd->controller_hw_id);
    h2fmi_ppn_get_manufacturer_id(fmi, cmd->ce, cmd->manufacturer_id);
    result = h2fmi_ppn_get_device_params(fmi, cmd->ce, params);
    WMR_ASSERT(result);

    if (fmi->ppn->spec_version >= PPN_VERSION_1_5_0)
    {
        h2fmi_ppn_get_marketing_name(fmi, cmd->ce, cmd->marketing_name);
    }

    cmd->caus                      = params->caus_per_channel;
    cmd->cau_bits                  = params->cau_bits;
    cmd->blocks_per_cau            = params->blocks_per_cau;
    cmd->block_bits                = params->block_bits;
    cmd->pages_per_block_mlc       = params->pages_per_block;
    cmd->pages_per_block_slc       = params->pages_per_block_slc;
    cmd->page_address_bits         = params->page_address_bits;
    cmd->bits_per_cell_bits        = params->address_bits_bits_per_cell;
    cmd->default_bits_per_cell     = params->default_bits_per_cell;
    cmd->page_size                 = params->page_size;
    cmd->dies                      = params->dies_per_channel;
    cmd->tRC                       = params->tRC;
    cmd->tREA                      = params->tREA;
    cmd->tREH                      = params->tREH;
    cmd->tRHOH                     = params->tRHOH;
    cmd->tRHZ                      = params->tRHZ;
    cmd->tRLOH                     = params->tRLOH;
    cmd->tRP                       = params->tRP;
    cmd->tWC                       = params->tWC;
    cmd->tWH                       = params->tWH;
    cmd->tWP                       = params->tWP;
    cmd->read_queue_size           = params->read_queue_size;
    cmd->program_queue_size        = params->program_queue_size;
    cmd->erase_queue_size          = params->erase_queue_size;
    cmd->prep_function_buffer_size = params->prep_function_buffer_size;
    cmd->tRST                      = params->tRST_ms;
    cmd->tPURST                    = params->tPURST_ms;
    cmd->tSCE                      = params->tSCE_ms;
    cmd->tCERDY                    = params->tCERDY_us;

    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_ppn_iop_get_die_info(h2fmi_t *fmi, IOPFMI_GetDieInfo *cmd)
{
    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    h2fmi_ppn_get_uid(fmi, cmd->ce, cmd->die, cmd->unique_id);
    h2fmi_ppn_get_die_chip_id(fmi, cmd->ce, cmd->die, cmd->chip_id);

    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_ppn_iop_update_firmware(h2fmi_t *fmi, IOPFMI_UpdateFirmware* cmd)
{
    UInt32    *sgl;
    UInt8      fwVerBuffer[NAND_DEV_PARAM_LEN_PPN];
    h2fmi_ce_t ce;

    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    sgl = (UInt32 *)mem_static_map_cached(cmd->sgl);
    WMR_ASSERT(sgl != MAP_FAILED);

    WMR_PREPARE_READ_BUFFER(sgl, cmd->sgl_length_in_bytes);

    for (ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        if (h2fmi_ppn_get_fw_version(fmi, ce, fwVerBuffer) == TRUE32)
        {
            cmd->iopfmi.status = h2fmi_ppn_fw_update(fmi, ce, (void *)sgl, cmd->fw_size, ppnFwTypeFw);
        }
    }
}

void h2fmi_ppn_iop_post_rst_pre_pwrstate_operations(h2fmi_t* fmi, IOPFMI_PostResetOperations *cmd)
{
    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    if (h2fmi_ppn_post_rst_pre_pwrstate_operations(fmi))
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
    }
    else
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_FAILURE;
    }
}

void h2fmi_ppn_iop_set_power(h2fmi_t *fmi, IOPFMI_SetPower* cmd)
{
    if (fmi->ppn->general_error)
    {
        cmd->iopfmi.status = kIOPFMI_STATUS_IOP_NOT_READY;
        return;
    }

    h2fmi_ppn_set_channel_power_state(fmi, cmd->power_state_trans);
    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_ppn_iop_get_failure_info(h2fmi_t *fmi, IOPFMI_GetFailureInfo* cmd)
{
    h2fmi_ppn_failure_info_t failureInfo;
    struct dma_segment       *sgl = NULL;

    WMR_ASSERT(fmi->ppn->general_error);

    if (cmd->sgl)
    {
        sgl = (struct dma_segment *)mem_static_map_cached(cmd->sgl);
        WMR_ASSERT(sgl != MAP_FAILED);
        WMR_PREPARE_READ_BUFFER(sgl, cmd->sgl_length);
    }

    h2fmi_ppn_get_general_error_info(fmi, cmd->ce, &failureInfo, &cmd->rma_buffer_length, sgl);

    if(fmi->ppn->bytes_per_row_address == PPN_1_5_ROW_ADDR_BYTES)
    {
        cmd->type = failureInfo.failure_info_1_5.type;
    }
    else
    {
        cmd->type = failureInfo.failure_info_1_0.type;
    }
    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_ppn_iop_set_feature_list(h2fmi_t *fmi, IOPFMI_SetFeatures* cmd)
{
    void *list = mem_static_map_cached(cmd->list);
    UInt32 size = cmd->size;

    h2fmi_ppn_set_feature_list(fmi, list, size / sizeof(ppn_feature_entry_t));

    cmd->iopfmi.status = kIOPFMI_STATUS_SUCCESS;
}

void h2fmi_ppn_iop_power_state_changed(h2fmi_t* fmi)
{
#if H2FMI_PPN_VERIFY_SET_FEATURES
    h2fmi_ppn_verify_feature_shadow(fmi);
#endif // H2FMI_PPN_VERIFY_SET_FEATURES
}

#endif //SUPPORT_PPN
