// *****************************************************************************
//
// File: H2fmi_iop.h
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

#ifndef _H2FMI_IOP_H
#define _H2FMI_IOP_H

// TODO: fix this by moving referenced header into common include directory
#include "../../../apps/EmbeddedIOP/function_fmi/iop_fmi_protocol.h"

#include "H2fmi_private.h"
bool h2fmi_setup_aes(h2fmi_t* fmi, IOPFMI* cmd, hfmi_aes_iv* aes_iv_array, UInt32 aes_chain_size, UInt32 aes_key_type, UInt32* aes_key, UInt32* seeds);
void h2fmi_iop_set_config(h2fmi_t* fmi, IOPFMI_SetConfig* config);
void h2fmi_iop_reset_everything(h2fmi_t* fmi, IOPFMI_ResetEverything* cmd);
void h2fmi_iop_read_chip_ids(h2fmi_t* fmi, IOPFMI_ReadChipIDs* cmd);
void h2fmi_iop_erase_single(h2fmi_t* fmi, IOPFMI_EraseSingle* cmd);
void h2fmi_iop_erase_multiple(h2fmi_t* fmi, IOPFMI_EraseMultiple* cmd);
void h2fmi_iop_read_single(h2fmi_t* fmi, IOPFMI_IOSingle* cmd);
void h2fmi_iop_write_single(h2fmi_t* fmi, IOPFMI_IOSingle* cmd);
void h2fmi_iop_read_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd);
void h2fmi_iop_write_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd);
void h2fmi_iop_read_multiple(h2fmi_t* fmi, IOPFMI_IOMultiple* cmd);
void h2fmi_iop_write_multiple(h2fmi_t* fmi, IOPFMI_IOMultiple* cmd);
void h2fmi_iop_write_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd);
void h2fmi_iop_read_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd);
void h2fmi_iop_sleep(h2fmi_t* fmi);
void h2fmi_iop_wake(h2fmi_t* fmi);

void h2fmi_ppn_iop_reset_everything(h2fmi_t *fmi, IOPFMI_ResetEverything* cmd);
void h2fmi_ppn_iop_erase_multiple(h2fmi_t *fmi, IOPFMI_IOPpn* cmd);
void h2fmi_ppn_iop_erase_single(h2fmi_t *fmi, IOPFMI_EraseSingle *cmd);
void h2fmi_ppn_iop_read_single(h2fmi_t *fmi, IOPFMI_IOSingle* cmd);
void h2fmi_ppn_iop_read_multiple(h2fmi_t *fmi, IOPFMI_IOPpn* cmd);
void h2fmi_ppn_iop_write_single(h2fmi_t *fmi, IOPFMI_IOSingle* cmd);
void h2fmi_ppn_iop_write_multiple(h2fmi_t *fmi, IOPFMI_IOPpn* cmd);
void h2fmi_ppn_iop_read_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd);
void h2fmi_ppn_iop_write_raw(h2fmi_t* fmi, IOPFMI_IORaw* cmd);
void h2fmi_ppn_iop_read_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd);
void h2fmi_ppn_iop_write_bootpage(h2fmi_t* fmi, IOPFMI_IOBootPage* cmd);
void h2fmi_ppn_iop_read_cau_bbt(h2fmi_t* fmi, IOPFMI_IOPpn* cmd);
void h2fmi_ppn_iop_update_firmware(h2fmi_t* fmi, IOPFMI_UpdateFirmware* cmd);
void h2fmi_ppn_iop_post_rst_pre_pwrstate_operations(h2fmi_t* fmi, IOPFMI_PostResetOperations *cmd);
void h2fmi_ppn_iop_set_power(h2fmi_t* fmi, IOPFMI_SetPower* cmd);
void h2fmi_ppn_iop_get_failure_info(h2fmi_t* fmi, IOPFMI_GetFailureInfo* cmd);
void h2fmi_ppn_iop_get_controller_info(h2fmi_t* fmi, IOPFMI_GetControllerInfo* cmd);
void h2fmi_ppn_iop_get_temperature(h2fmi_t *fmi, IOPFMI_GetTemperature* cmd);
void h2fmi_ppn_iop_get_die_info(h2fmi_t* fmi, IOPFMI_GetDieInfo* cmd);
void h2fmi_ppn_iop_set_feature_list(h2fmi_t *fmi, IOPFMI_SetFeatures* cmd);
void h2fmi_ppn_iop_power_state_changed(h2fmi_t* fmi);

void dump_fmi_state(h2fmi_t *pFMI, uint32_t index, BOOL32 withHWRegs, BOOL32 withECC);

#endif // _H2FMI_IOP_H
