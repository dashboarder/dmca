// *****************************************************************************
//
// File: debug.c
//
// Copyright (C) 2010-2012, 2014 Apple Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
// *****************************************************************************

#include <libDER/DER_Keys.h>
#include <libDER/DER_Decode.h>
#include <libDER/asn1Types.h>
#include "WMROAM.h"
#include <FIL.h>
#include <FILTypes.h>
#include "H2fmi_private.h"
#include "H2fmi.h"
#include "H2fmi_ppn.h"

#include <platform/memmap.h>
#include <sys/menu.h>
#include <lib/env.h>
#include <platform/clocks.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <lib/random.h>
#include <drivers/power.h>
#include <drivers/usb/usb_debug.h>

#if SUPPORT_PPN

extern UInt32 *g_pScratchBuf;

extern h2fmi_t g_fmi0;
extern h2fmi_t g_fmi1;

static int do_vthsweep(int argc, struct cmd_arg *argv);
static int do_ppnfw(int argc, struct cmd_arg *argv);
static int do_ppnver(int argc, struct cmd_arg *argv);
static int do_ppnrma(int argc, struct cmd_arg *args);
static int do_ppnscratchpad(int argc, struct cmd_arg *argv);
#if SUPPORT_TOGGLE_NAND
static int do_nand_dqs_delay(int argc, struct cmd_arg *args);
#endif // SUPPORT_TOGGLE_NAND

MENU_COMMAND_DEVELOPMENT(ppnfw, do_ppnfw, "Update PPN controller firmware", NULL);
MENU_COMMAND_DEVELOPMENT(ppnver, do_ppnver, "Get PPN controller firmware version", NULL);
MENU_COMMAND_DEVELOPMENT(ppnrma, do_ppnrma, "Get PPN RMA data for a particular page", NULL);
MENU_COMMAND_DEVELOPMENT(ppnscratchpad, do_ppnscratchpad, "Perform a PPN Scratch Pad test", NULL);
MENU_COMMAND_DEVELOPMENT(vthsweep, do_vthsweep, "Performs Vth sweep on a block of a given CE and dumps the data to a file", NULL);

#define VTHSWEEP_ARGC        (4)
#define VENDOR_CODE__MICRON  (0x2C)
#define VENDOR_CODE__HYNIX   (0xAD)
#define VENDOR_CODE__SANDISK (0x45)
#define DMA_TRANSFER_LIMIT   (16 * 1024 * 1024)
#define SCRATCHPAD_LEN       (2 << 10) // there is currently a 2KB limit with Toshiba: rdar://problem/10134035
#define DUMPLEN              (8)
#define PREFILL              (0xa5)
static UInt8 scratchpad_buf_get[SCRATCHPAD_LEN];
static UInt8 scratchpad_buf_set[SCRATCHPAD_LEN];

#if SUPPORT_TOGGLE_NAND
MENU_COMMAND_DEVELOPMENT(nand_dqs_delay, do_nand_dqs_delay, "NAND DQS read delay test (see \"nand_dqs_delay help\")", NULL);

#if SUPPORT_NAND_DLL
static void dll_lock_sweep(UInt32 vdd_min, UInt32 vdd_max, UInt32 vdd_step){
    UInt32 vdd;
    UInt32 lock;
    char pbuf[16];

    printf("DLL lock sweep:\n");
    printf("SOC voltage: ");
    for (vdd = vdd_min; vdd <= vdd_max; vdd += vdd_step)
    {
        snprintf(pbuf, sizeof(pbuf), "%u mV", vdd);
        printf("%-7s  ", pbuf);
    }
    printf("\nDLL lock:    ");
    for (vdd = vdd_min; vdd <= vdd_max; vdd += vdd_step)
    {
        power_set_soc_voltage(vdd, true);
        WMR_DLL_CLOCK_GATE(TRUE32);
        h2fmi_dll_wr(NAND_DLL_CONTROL,   NAND_DLL_CONTROL__REFERENCE(0));
        WMR_DLL_CLOCK_GATE(FALSE32);
        h2fmiTrainDLL(&lock);
        printf("%-7u  ", lock);
    }
    printf("\n");
    power_set_soc_voltage(platform_get_base_soc_voltage(), true);
}
#endif // SUPPORT_NAND_DLL

static int do_nand_dqs_delay(int argc, struct cmd_arg *args) 
{
    h2fmi_ce_t ce;
    UInt8 status;
    UInt32 *errors;
    Int32 e_min;
    Int32 e_max;
    Int32 e0_min;
    Int32 e0_max;
    Int32 i;
    UInt32 ei = 0;
    UInt32 di;
    UInt32 iter;
    UInt32 dqsd;
    UInt32 vdd;
    UInt32 fq ;
    UInt32 fmi_mask = 3;
    UInt32 freq_mask;
    UInt32 vdd_min;
    UInt32 vdd_max;
    UInt32 vdd_step = 50;
    UInt32 dqsd_min = 8;
    UInt32 dqsd_max = 511;
    UInt32 fmi_freq;
    UInt32 rd_freq;
    UInt32 tHALF_CYCLE_ori;
    UInt32 def_dqsd;
    UInt32 timing_size;
    UInt32 errors_size;
    UInt32 num_freq = 0;
    UInt32 num_ce = 0;
    UInt32 num_iter = 5;
    UInt32 num_vdd;
    UInt32 num_dqsd;
    Int32 num_arg;
    BOOL32 vdd_ok;
    NandTimingParams timing;
    
    if (!g_fmi0.is_toggle || !g_fmi1.is_toggle)
    {
        printf("Not in toggle mode.\n");
        return -1;
    }

    memset(&timing, 0xFF, sizeof(timing));
    timing_size = sizeof(timing);
    FIL_GetStruct(AND_STRUCT_FIL_GET_TIMINGS, &timing, &timing_size);
    freq_mask = (1 << timing.ddr_tHALFCYCLE);

    vdd_min = chipid_get_soc_voltage(CHIPID_SOC_VOLTAGE_LOW);
    vdd_max = chipid_get_soc_voltage(CHIPID_SOC_VOLTAGE_HIGH);
    
    for (i = 1; i < argc; i += num_arg + 1){
        if (!strcmp(args[i].str, "iter") && (argc-i > (num_arg = 1)))
        {
            if (args[i+1].u == 0)
            {
                printf("Invalid number of iterations.\n");
                return -1;
            }
            num_iter = args[i+1].u;
        }
        else if (!strcmp(args[i].str, "vdd") && (argc-i > (num_arg = 3)))
        {
            vdd_ok = ((power_set_soc_voltage(args[i+1].u, true) == 0) && (power_set_soc_voltage(args[i+2].u, true) == 0));
            power_set_soc_voltage(platform_get_base_soc_voltage(), true);
            if (!vdd_ok || (args[i+1].u > args[i+2].u))
            {
                printf("Invalid vdd range.\n");
                return -1;
            }
            vdd_min = args[i+1].u;
            vdd_max = args[i+2].u;
            vdd_step = (args[i+3].u == 0) ? 1 : args[i+3].u;
        }
        else if (!strcmp(args[i].str, "freq") && (argc-i > (num_arg = 1)))
        {
            if ((args[i+1].u < 1) || (args[i+1].u > 0xFFFF))
            {
                printf("Invalid 16-bit bitmask of read frequencies to test.\n");
                return -1;
            }
            freq_mask = args[i+1].u;
        }
        else if (!strcmp(args[i].str, "fmi") && (argc-i > (num_arg = 1)))
        {
            if ((args[i+1].u < 1) || (args[i+1].u > 3))
            {
                printf("Invalid 2-bit bitmask of FMI channels to use.\n");
                return -1;
            }
            fmi_mask = args[i+1].u;
        }
        else if (!strcmp(args[i].str, "dqsdel") && (argc-i > (num_arg = 2)))
        {
            if ((args[i+1].u < dqsd_min) || (args[i+2].u > dqsd_max) || (args[i+1].u > args[i+2].u))
            {
                printf("Invalid DQS delay range (must be between %u and %u).\n", dqsd_min, dqsd_max);
                return -1;
            }
            dqsd_min = args[i+1].u;
            dqsd_max = args[i+2].u;
        }
        else
        {
            printf("Usage: nand_dqs_delay [iter <num_iter>] [vdd <vdd_min_mv> <vdd_max_mv> <vdd_step_mv>] ");
            printf("[freq <read_freq_mask>] [fmi <fmi_mask>] [dqsdel <dqs_delay_min> <dqs_delay_max>]\n");
            return -1;
        }
    }
    
    for(ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        if (g_fmi0.valid_ces & (1 << ce))
        {
            if(FIL_SUCCESS != h2fmi_ppn_set_features(&g_fmi0, ce, PPN_FEATURE__SCRATCH_PAD, (UInt8 *) scratchpad_buf_set, SCRATCHPAD_LEN, FALSE32, NULL))
            {
                printf("Unable to access scratch pad.\n");
                return -1;
            }
        }
    }

    num_vdd = (vdd_max - vdd_min) / vdd_step + 1;
    num_dqsd = dqsd_max - dqsd_min + 1;
    for(fq = 0; fq < 16; fq++)
    {
        if (freq_mask & (1 << fq)) 
        {
            num_freq++;
        }
    }
    for(ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        if (((fmi_mask & 1) && (g_fmi0.valid_ces & (1 << ce))) ||
            ((fmi_mask & 2) && (g_fmi1.valid_ces & (1 << ce))))
        {
            num_ce++;
        }
    }
    errors_size = num_ce * num_freq * num_vdd * num_dqsd;
    errors = calloc(errors_size, sizeof(errors[0]));

    fmi_freq = clock_get_frequency(CLK_FMI); 
    def_dqsd = FMC_DQS_TIMING_CTRL__DQS_READ_DELAY_CTRL_GET(g_fmi0.dqs_ctrl);
    printf("NAND DQS read delay test\n");
    printf("Data per iteration: %u bytes\n", SCRATCHPAD_LEN);
    printf("DQS delay test range: %u - %u\n", dqsd_min, dqsd_max);
    printf("FMI clock frequency: %u.%06u MHz\n", fmi_freq / 1000000, fmi_freq % 1000000);
    printf("Current (default) SOC voltage: %u mV\n", platform_get_base_soc_voltage());
    #if SUPPORT_NAND_DLL
        dll_lock_sweep(vdd_min, vdd_max, vdd_step);
    #else
        printf("Current (default) DQS read delay value: %u\n", def_dqsd);
        printf("Using DLL: no\n");
    #endif // SUPPORT_NAND_DLL

    printf("Generating random data ");
    for (i = 0; i < SCRATCHPAD_LEN; i += 1000){
        random_get_bytes(&scratchpad_buf_set[i], WMR_MIN(1000, SCRATCHPAD_LEN - i));
        printf(".");
    }
    printf("\nInterface  Freq [Mhz]  Vdd [mV]  Iter  Zero error range\n");
    printf("Bytes per test setting,%u\n\n", num_iter * SCRATCHPAD_LEN);
    printf(",Byte errors\nDQS Delay");
    
    tHALF_CYCLE_ori = timing.ddr_tHALFCYCLE;
    
    for(ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        h2fmi_t* fmi;
        UInt32 dqs_ctrl_ori;

        if ((fmi_mask & 1) && (g_fmi0.valid_ces & (1 << ce)))       
        {
            fmi = &g_fmi0;
        }
        else if ((fmi_mask & 2) && (g_fmi1.valid_ces & (1 << ce)))  
        {
            fmi = &g_fmi1;
        }
        else 
        {
            continue;
        }
        
        h2fmi_ppn_set_features(fmi, ce, PPN_FEATURE__SCRATCH_PAD, (UInt8 *) scratchpad_buf_set, SCRATCHPAD_LEN, FALSE32, NULL);
        
        dqs_ctrl_ori = fmi->dqs_ctrl;

        for(fq = 0; fq < 16; fq++)
        {
            if (!(freq_mask & (1 << fq))) continue;

            timing.ddr_tHALFCYCLE = fq;
            FIL_SetStruct(AND_STRUCT_FIL_SET_TIMINGS, &timing, timing_size);
            rd_freq = fmi_freq / (timing.ddr_tHALFCYCLE + 1) / 2;
            
            for (vdd = vdd_min; vdd <= vdd_max; vdd += vdd_step) 
            {
                printf(",\"FMI%d:CE%d, %u.%06uMhz, %umV\"", fmi->bus_id, ce, rd_freq / 1000000, rd_freq % 1000000, vdd);
                power_set_soc_voltage(vdd, true);
                for (iter = 0; iter < num_iter; iter++) 
                {
                    e_min = 0;
                    e_max = -1;
                    e0_min = 0;
                    e0_max = -1;
                    for (di = 0; di < num_dqsd; di++)
                    {
                        dqsd = dqsd_min + di;
                        fmi->dqs_ctrl = FMC_DQS_TIMING_CTRL__DQS_READ_DELAY_CTRL(dqsd);
                        h2fmi_wr(fmi, FMC_DQS_TIMING_CTRL, fmi->dqs_ctrl); 
                        
                        h2fmi_ppn_get_feature(fmi, ce, PPN_FEATURE__SCRATCH_PAD, (UInt8 *) scratchpad_buf_get, SCRATCHPAD_LEN, &status);
                        
                        WMR_ASSERT((ei+di) < errors_size);
                        for (i = 0; i < SCRATCHPAD_LEN; i++)
                        {
                            if (scratchpad_buf_set[i] != scratchpad_buf_get[i]) 
                            {
                                errors[ei+di]++;
                            }
                        }
                        
                        if (!errors[ei+di]) 
                        {
                            e0_max = dqsd;
                        }
                        if (!errors[ei+di] && (di == 0 || errors[ei+di-1]))
                        {
                            e0_min = dqsd;
                        }
                        if ((e0_max-e0_min) > (e_max-e_min)) 
                        {
                            e_max = e0_max; 
                            e_min = e0_min;
                        }
                    }
                }
                printf("\n");
                ei += num_dqsd;
            }
        }

        fmi->dqs_ctrl = dqs_ctrl_ori;
        h2fmi_wr(fmi, FMC_DQS_TIMING_CTRL, fmi->dqs_ctrl);
    }
    
    printf("\n");
    for (di = 0; di < num_dqsd; di++)
    {
        printf("%u", di + dqsd_min);
        for (ei = 0; ei < errors_size; ei += num_dqsd)
        {
            printf(",%u", errors[ei+di]);
        }
        printf("\n");
    }
    
    power_set_soc_voltage(platform_get_base_soc_voltage(), true);
    timing.ddr_tHALFCYCLE = tHALF_CYCLE_ori;
    FIL_SetStruct(AND_STRUCT_FIL_SET_TIMINGS, &timing, timing_size);
    free(errors);
    
    return 0;
}
#endif // SUPPORT_TOGGLE_NAND

static bool find_blobs(void *src_buffer, UInt32 src_length, void **fw, UInt32 *fw_len, void **fwa, UInt32 *fwa_len)
{
    DERReturn drtn;
    DERSequence rdn;
    DERItem seq, top = { src_buffer, src_length };
    DERDecodedInfo topDecode, keyC, valC;

    if (DR_Success != DERDecodeItem(&top, &topDecode)) {
        printf("couldn't crack top level\n");
        return false;
    }

    if ((ASN1_CONSTRUCTED|ASN1_APPLICATION) != ((ASN1_CONSTRUCTED|ASN1_APPLICATION) & topDecode.tag)) {
        printf("top should be cons app\n");
        return false;
    }

    seq.length = topDecode.content.length;
    seq.data = topDecode.content.data;

    if (DR_Success != DERDecodeSeqContentInit(&seq, &rdn)) {
        printf("could not initialize DER decode\n");
        return false;
    }

    while ((drtn = DERDecodeSeqNext(&rdn, &keyC)) == DR_Success) {
        // Got key UTF8-str, check
        if (ASN1_UTF8_STRING != keyC.tag) {
            printf("key invalid: %lld\n", keyC.tag);
            return false;
        }

        // Get value Octet-str
        if (DR_Success != ((drtn = DERDecodeSeqNext(&rdn, &valC)))) {
            printf("no value found for key %lld\n", keyC.tag);
            return false;
        }

        // Check value tag
        if (ASN1_OCTET_STRING != valC.tag) {
            printf("value invalid: %lld\n", valC.tag);
            return false;
        }

        // Decode
        if ((11 == keyC.content.length) && (0 == strncmp((char*)keyC.content.data, "ppn-fw-args", 11))) {
            *fwa = valC.content.data;
            *fwa_len = valC.content.length;
        } else if ((6 == keyC.content.length) && (0 == strncmp((char*)keyC.content.data, "ppn-fw", 6))) {
            *fw = valC.content.data;
            *fw_len = valC.content.length;
        } else {
            printf("unknown key\n");
            return false;
        }
    }

    return true;
}

static int do_vthsweep(int argc, struct cmd_arg *args)
{
    // investigate if there's an easy way to limit which ppn fw can run this cmd
    // <rdar://problem/11853291> Limit Vth sweep cmd to firmware that support it

    UInt8 virtual_ce;
    UInt32 blk;
    UInt32 numPerfPagesThatFit = DEFAULT_RAMDISK_SIZE / PPN_PERFECT_PAGE_SIZE;
    struct dma_segment sgl;
    h2fmi_ppn_failure_info_t failInfo;
    UInt32 pagesRead = 0;
    UInt32 totalPagesToRead;
    UInt32 startPage;
    UInt32 bytesToRead;
    UInt16 gebType;
    Int32 ret;
    UInt8 mfgID[H2FMI_NAND_ID_SIZE];
    int thisFuncRet = -1;
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 blocks_per_cau = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
    const UInt32 block_bits = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_BLOCK_ADDRESS);
    const UInt32 block_shift = 0;
    const UInt32 block_mask = (1UL << block_bits) - 1;
    const UInt32 caus_per_ce = fil->GetDeviceInfo(AND_DEVINFO_CAUS_PER_CE);
    const UInt32 cau_bits = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_CAU_ADDRESS);
    const UInt32 cau_shift = block_bits + block_shift;
    const UInt32 cau_mask = (1UL << cau_bits) - 1;
    const UInt32 slc_bits = 1;
    const UInt32 slc_shift = cau_bits + cau_shift;
    const UInt32 slc_mask = (1UL << slc_bits) - 1;
    const UInt32 invalid_shift = slc_bits + slc_shift;

    if (argc != VTHSWEEP_ARGC)
    {
        printf("Usage: vthsweep <virtual_ce> <blk> <file_path>\n");
        return -1;
    }

    if ((virtual_ce = args[1].u) >= g_fmi0.num_of_ce + g_fmi1.num_of_ce)
    {
        printf("Invalid ce = %d. ce < %d\n", virtual_ce, g_fmi0.num_of_ce + g_fmi1.num_of_ce);
        return -1;
    }

    blk = args[2].u;
    if ((0 != (blk >> invalid_shift)) ||
        (blocks_per_cau <= ((blk >> block_shift) & block_mask)) ||
        (caus_per_ce <= ((blk >> cau_shift) & cau_mask)))
    {
        printf("Invalid address: block %u (max %u). cau %u (max %u), slc %u, reserved %u\n",
               ((blk >> block_shift) & block_mask), blocks_per_cau - 1,
               ((blk >> cau_shift) & cau_mask), caus_per_ce - 1,
               ((blk >> slc_shift) & slc_mask),
               blk >> invalid_shift);
        return -1;
    }

#if SUPPORT_TOGGLE_NAND
    if (g_fmi0.is_toggle_system)
    {
        transitionWorldFromDDR(PPN_PS_TRANS_DDR_TO_LOW_POWER);
    }
    else
    {
#endif
        h2fmi_ppn_all_channel_power_state_transition(PPN_PS_TRANS_ASYNC_TO_LOW_POWER);
#if SUPPORT_TOGGLE_NAND
    }
#endif

    ret = h2fmiPpnGetManufacturerId(virtual_ce, mfgID);
    if (ret != FIL_SUCCESS)
    {
        printf("Failed to read Mfg ID! ret = 0x%x\n", ret);
        goto finish;
    }
    
    if ((mfgID[0] != VENDOR_CODE__HYNIX) && (mfgID[0] != VENDOR_CODE__SANDISK) && (mfgID[0] != VENDOR_CODE__MICRON))
    {
        printf("Vendor 0x%x not supported!\n", mfgID[0]);
        goto finish;
    }

    if ((ret = h2fmiVthSweepSetup(virtual_ce, blk, &failInfo, &bytesToRead)) != FIL_SUCCESS)
    {
        printf("h2fmiVthSweepSetup failed! ret = 0x%x, bytesToRead = 0x%x\n", ret, bytesToRead);
        goto finish;
    }
    totalPagesToRead = FAIL_INFO_GET_PAGE_COUNT(g_fmi0.ppn->bytes_per_row_address, failInfo);
    startPage = FAIL_INFO_GET_START_PAGE(g_fmi0.ppn->bytes_per_row_address, failInfo);
    gebType = FAIL_INFO_GET_TYPE(g_fmi0.ppn->bytes_per_row_address, failInfo);

    WMR_ASSERT(gebType == PPN_GEB_TYPE__EMB_SWEEP_GEB_TYPE);
    WMR_ASSERT(bytesToRead == totalPagesToRead * PPN_PERFECT_PAGE_SIZE);

    sgl.paddr = (UInt32)env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);

    printf("\033[1;31m"); // change font color to red & bold
    while (pagesRead < totalPagesToRead)
    {
        UInt32 currentPagesToRead = WMR_MIN(WMR_MIN(numPerfPagesThatFit, totalPagesToRead - pagesRead),  DMA_TRANSFER_LIMIT / PPN_PERFECT_PAGE_SIZE);
        sgl.length = currentPagesToRead * PPN_PERFECT_PAGE_SIZE;

        printf("Status: %d%%, Pages: %d/%d", pagesRead * 100 / totalPagesToRead, pagesRead, totalPagesToRead);
        if ((ret = h2fmiDmaDebugData(virtual_ce, startPage + pagesRead, currentPagesToRead, &sgl, FALSE32)) != FIL_SUCCESS)
        {
            printf("h2fmiDmaDebugData failed ret = 0x%x\n", ret);
            goto finish;
        }
        pagesRead += currentPagesToRead;

        if ((ret = usb_send_data_to_file(args[3].str, sgl.length, sgl.paddr, 0)))
        {
            printf("usb_cmd_put failed ret = 0x%x\n", ret);
            goto finish;
        }
        printf("\033[0E\033[2K"); // go to start of line and erase the line respectively
    }
    printf("Status: %d%%, Pages: %d/%d\n", pagesRead * 100 / totalPagesToRead, pagesRead, totalPagesToRead);
    printf("\033[0m"); // change font color back to default
    thisFuncRet = 0;

finish:

    // not going back to normal/DDR mode because the nand needs to be reset
    h2fmi_ppn_recover_nand();

    return thisFuncRet;
}

static int do_ppnfw(int argc, struct cmd_arg *args)
{
    addr_t             buf_ptr;
    size_t             buf_len;
    UInt32             virtualCe;
    Int32              status;
    void              *fw_buffer = NULL;
    UInt32             fw_length;
    void              *fwa_buffer = NULL;
    UInt32             fwa_length;
    int                ret = 0;

    if (argc > 3)
    {
        return -1;
    }

    buf_ptr = (argc > 1) ? args[1].u : env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    buf_len = (argc > 2) ? args[2].u : env_get_uint("filesize", 0);

    if (!security_allow_memory((void*)buf_ptr, buf_len))
    {
        return -2;
    }

    fw_buffer = NULL;
    fw_length = 0;
    fwa_buffer = NULL;
    fwa_length = 0;
    find_blobs((void*)buf_ptr, buf_len, &fw_buffer, &fw_length, &fwa_buffer, &fwa_length);
    if ((NULL == fw_buffer) && (NULL == fwa_buffer))
    {
        printf("couldn't find both fw and fwa payloads\n");
        return -3;
    }

#if SUPPORT_TOGGLE_NAND
    if (g_fmi0.is_toggle_system)
    {
        transitionWorldFromDDR(PPN_FEATURE__POWER_STATE__LOW_POWER);
    }
#endif

    // reset device to ensure clean initial state (and ensure PPN 1.0 device is in LP mode)
    h2fmi_nand_reset_all(&g_fmi0);
    h2fmi_nand_reset_all(&g_fmi1);
    h2fmi_ppn_post_rst_pre_pwrstate_operations(&g_fmi0);
    h2fmi_ppn_post_rst_pre_pwrstate_operations(&g_fmi1);

    for (virtualCe = 0; virtualCe < H2FMI_MAX_CE_TOTAL; virtualCe++)
    {
        printf("Updating virtual CE %d\n", virtualCe);

        status = h2fmiPpnUpdateFw(virtualCe, fw_buffer, fw_length, fwa_buffer, fwa_length);
        if (status == FIL_SUCCESS)
        {
            printf("Firmware successfully updated on virtual CE %d\n",
                   virtualCe);
        }
        else if (status == FIL_UNSUPPORTED_ERROR)
        {
            printf("Virtual CE %d doesn't support firmware updates; skipping\n",
                   virtualCe);
        }
        else
        {
            printf("PPN Firmware Update failed\n");
            ret = -10;
            goto done;
        }

    }

done:

    // issue FF reset after the firmware update to allow the device to transition to the new firmware
    // on monodie parts running invalid fw, fmi1 may not be valid <rdar://problem/11286520>
    // a reboot is required to sync up the system with the new firmware
    printf("System running in low power mode, reboot required after firmware update\n");
    h2fmi_nand_reset_all(&g_fmi0);
    h2fmi_nand_reset_all(&g_fmi1);

    return ret;
}

static int do_ppnver(int argc, struct cmd_arg *argv)
{
    UInt32   virtualCe;
    UInt8    buffer[NAND_DEV_PARAM_LEN_PPN];

    if (!g_fmi0.is_ppn)
    {
        WMR_PRINT(ERROR, "Target is not a PPN device\n");
        return -1;
    }

    for (virtualCe = 0; virtualCe < H2FMI_MAX_CE_TOTAL; virtualCe++)
    {
        if (FIL_SUCCESS == h2fmiPpnGetFirmwareVersion(virtualCe, buffer))
        {
            printf("PPN Version for virtual CE %d: %16.16s\n", virtualCe, buffer);
        }
    }
    return 0;
}

static void rma_usage(void)
{
    printf("Usage: \n");
    printf("       ppnrma get <virtual ce> [<page>]\n");
    printf("       ppnrma delete <virtual ce>\n");
    printf("       ppnrma configure <virtual ce> <flags>\n");
    printf("       ppnrma recover\n");
}

static int do_ppnrma(int argc, struct cmd_arg *args)
{
    h2fmi_t                 *fmi;
    UInt32                   virtualCe;
    UInt32                   ce;
    UInt32                   page;
    h2fmi_ppn_failure_info_t failureInfo;
    UInt32                   failureLength;
    void                    *buffer;
    struct dma_segment       sgl;

    if ((argc != 2) && (argc != 3) && (argc != 4))
    {
        rma_usage();
        return -1;
    }

    virtualCe = args[2].u;
    page      = argc > 3 ? args[3].u : 0;

    fmi = h2fmiTranslateVirtualCEToBus(virtualCe);
    ce  = h2fmiTranslateVirtualCEToCe(virtualCe);
    
    if (!strcmp(args[1].str, "get"))
    {
        buffer = (void *)env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    
        if (argc == 4)
        {
            // Force Debug Data for a particular page
            h2fmi_ppn_force_geb_address(fmi, ce, (PageAddressType)page);
        }

        // Get general error info and additional data length
        if ( h2fmi_ppn_get_general_error_info(fmi, ce, &failureInfo, &failureLength, NULL) &&
             (failureLength > 0))
        {
            printf("Reading %d bytes of Debug Data to %p\n", failureLength, buffer);
        
            sgl.paddr  = (UInt32)buffer;
            sgl.length = failureLength;
            WMR_PREPARE_READ_BUFFER(buffer, failureLength);
            // Get full buffer
            h2fmi_ppn_get_general_error_info(fmi, ce, &failureInfo, &failureLength, &sgl);
            WMR_COMPLETE_READ_BUFFER(buffer, failureLength);
            printf("Debug Data buffer extracted.  Save to host by running 'usb put <filename> %d'\n", failureLength);
        }
    }
    else if (!strcmp(args[1].str, "delete"))
    {
        UInt32 feature_data = PPN_DELETE_DEBUG_DATA__ENABLE;

        if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi,
                                                  ce,
                                                  PPN_FEATURE__DELETE_DEBUG_DATA,
                                                  (UInt8*) &feature_data,
                                                  PPN_FEATURE_LENGTH_DEFAULT,
                                                  FALSE32,
                                                  NULL))
        {
            printf("delete debug data failed\n");
            return -1;
        }
        else
        {
            printf("Debug data deleted\n");
        }
    }
    else if (!strcmp(args[1].str, "configure"))
    {
        if (argc < 4)
        {
            rma_usage();
            return -1;
        }
        fmi->ppn->allow_saving_debug_data = TRUE32;
        fmi->ppn->debug_flags_valid = TRUE32;
        fmi->ppn->debug_flags = args[3].u;
        fmi->ppn->vs_debug_flags_valid = FALSE32;

        if (FIL_SUCCESS != h2fmi_ppn_configure_debug_data(fmi, ce))
        {
            printf("configuration failed\n");
            return -1;
        }
    }
    else if (!strcmp(args[1].str, "recover"))
    {
        h2fmi_ppn_recover_nand();
    }
    else
    {
        rma_usage();
        return -1;
    }

    return 0;
}

static int do_ppnscratchpad(int argc, struct cmd_arg *argv)
{
    int ret = 0;
    int argi = 1;
    uint32_t len = (argi < argc ?         argv[argi++].u : SCRATCHPAD_LEN);
    void  *src = (argi < argc ? (void *)argv[argi++].u : NULL);

    uint32_t ce;

    if (SCRATCHPAD_LEN < len)
    {
        printf("truncating length (%u) to maximum (%u)\n", len, SCRATCHPAD_LEN);
        len = SCRATCHPAD_LEN;
    }

    if (NULL != src)
    {
        memcpy(scratchpad_buf_set, src, len);
    }
    else
    {
        random_get_bytes(scratchpad_buf_set, len);
    }

    for(ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        h2fmi_t* fmi;
        Int32 status;
        UInt32 i;

        if (0 != (g_fmi0.valid_ces & (1 << ce)))
        {
            fmi = &g_fmi0;
        }
        else if (0 != (g_fmi1.valid_ces & (1 << ce)))
        {
            fmi = &g_fmi1;
        }
        else
        {
            continue;
        }

        status = h2fmi_ppn_set_features(fmi, ce, PPN_FEATURE__SCRATCH_PAD, scratchpad_buf_set, len, FALSE32, NULL);
        if (FIL_SUCCESS != status)
        {
            printf("scratch pad write returned %i on bus %u, ce %u\n", status, fmi->bus_id, ce);
            ret = -1;
            continue;
        }

        memset(scratchpad_buf_get, PREFILL, len);
        if (!h2fmi_ppn_get_feature(fmi, ce, PPN_FEATURE__SCRATCH_PAD, scratchpad_buf_get, len, NULL))
        {
            printf("scratch pad read returned %i on bus %u, ce %u\n", status, fmi->bus_id, ce);
            ret = -1;
            continue;
        }

        for (i = 0; i < len; i++)
        {
            if (scratchpad_buf_set[i] != scratchpad_buf_get[i])
            {
                UInt32 j, k;

                j = ROUNDDOWNTO(i, DUMPLEN);
                printf("Expected @ 0x%08x:", j);
                for (k = 0 ; DUMPLEN > k ; k++)
                {
                    printf(" %02x", scratchpad_buf_set[j++]);
                }
                printf("\n");

                j = ROUNDDOWNTO(i, DUMPLEN);
                printf("Actual   @ 0x%08x:", j);
                for (k = 0 ; DUMPLEN > k ; k++)
                {
                    printf(" %02x", scratchpad_buf_get[j++]);
                }
                printf("\n");

                i = ROUNDUPTO(i, DUMPLEN);

                ret = -1;
            }
        }
    }

    return ret;
}

#endif //SUPPORT_PPN

