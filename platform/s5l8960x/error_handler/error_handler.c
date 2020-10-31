/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amc/amc_regs.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <platform/int.h>
#include <sys.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/chipid.h>
#include <platform/error_handler.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/miu.h>

// Enables all errors (the remaining bits are perf counters)
#define AMC_IRERRDBG_INTEN_ALLERRORS                    ((1<<20) |\
                        (1<<19)|(1<<17)|(1<<16)|(1<<15)| \
                        (1<<14)|(1<<13))

// INTSTATUS and IERRDBG bit positions are different.
#define AMC_INTSTATUS_ALLERRORS \
                        ((1<<25)|(1<<24)|(1<<22)|(1<<21)|(1<<20)|(1<<19)|(1<<18))

#define CP_COM_NORM_MSK_CLR_ALL                         (0xFFFFFFFF)
#define CP_COM_NORM_MSK_ILLEGAL_COH_INTS                ((1<<0) | (1<<14))


static void amc_int_handler(void* arg);
static void cp_int_handler(void *arg); 


static void enable_cp_checks()
{
    install_int_handler(INT_COHERENCE_PNT_ERR, cp_int_handler, NULL); 
    if (chipid_get_chip_revision() < 0x10) {
        // Alcatraz A0 errata <rdar://problem/11535513> causes spurious illegal coherent access errors
        rCP_COM_INT_NORM_MASK_CLR = (CP_COM_NORM_MSK_CLR_ALL & ~CP_COM_NORM_MSK_ILLEGAL_COH_INTS); 
        rCP_COM_INT_NORM_MASK_SET = CP_COM_NORM_MSK_ILLEGAL_COH_INTS;
    }
    else
    {
        rCP_COM_INT_NORM_MASK_CLR = CP_COM_NORM_MSK_CLR_ALL;
    }
    unmask_int(INT_COHERENCE_PNT_ERR);
}
static void enable_amc_checks()
{
    install_int_handler(INT_MEM_CONTROLLER, amc_int_handler, NULL);
    rAMC_INTEN = AMC_IRERRDBG_INTEN_ALLERRORS;
    if(rAMC_INTSTATUS & AMC_INTSTATUS_ALLERRORS) {
        // AMC Interrupts, unlike the CP ones, don't seem to stick. If an error already exists
        // this early in bootup, we won't get an interrupt upon unmask.
        amc_int_handler(NULL);
    }
    rAMC_INTSTATUS = AMC_INTSTATUS_ALLERRORS;
    unmask_int(INT_MEM_CONTROLLER); 
}
void platform_enable_error_handler() {

    enable_amc_checks(); 
    enable_cp_checks(); 
}

static void amc_int_handler(void *arg)
{
    (void)arg;
    panic("Received unexpected AMC error. AMC_IERRDBG_INTSTATUS = 0x%x\n", rAMC_INTSTATUS);
}

static void cp_int_handler(void *arg) {
    (void)arg;
    panic("Received unexpected Coherency Point error. CP_COM_INT_NORM_REQUEST = 0x%x\n", rCP_COM_INT_NORM_REQUEST);
}
