/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* This file contains Tunables for t8002 */

#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwregbase.h>
#include <soc/module/address_map.h>

/**************************************** BEGIN CHIP_REVISION_B0 ****************************************/
#include <soc/t8002/b0/tunable/minipmgr.h>
#include <soc/t8002/b0/tunable/pmgr.h>
#include <soc/t8002/b0/tunable/cpu_fabric.h>
#include <soc/t8002/b0/tunable/nrt_fabric.h>
#include <soc/t8002/b0/tunable/uvdc.h>
#include <soc/t8002/b0/tunable/pio_wrap.h>

static const struct tunable_struct pmgr_tunable_t8002_b0[] = {
    PMS_PMGR_DEFAULT_TUNABLES_T8002_B0
};

static const struct tunable_struct minipmgr_tunable_t8002_b0[] = {
    AOP_MINIPMGR_DEFAULT_TUNABLES_T8002_B0
};

static const struct tunable_struct cpu_fabric_tunable_t8002_b0[] = {
    PIO_CPU_FABRIC_DEFAULT_TUNABLES_T8002_B0
};

static const struct tunable_struct nrt_fabric_tunable_t8002_b0[] = {
    PIO_NRT_FABRIC_DEFAULT_TUNABLES_T8002_B0
};

static const struct tunable_struct uvdc_tunable_t8002_b0[] = {
    PMS_UVDC_DEFAULT_TUNABLES_T8002_B0
};

static const struct tunable_struct pio_wrap_tunable_t8002_b0[] = {
    PIO_PIO_WRAP_DEFAULT_TUNABLES_T8002_B0
};
/***************************************** END CHIP_REVISION_B0 *****************************************/

/**************************************** BEGIN CHIP_REVISION_A0 ****************************************/
#include <soc/t8002/a0/tunable/minipmgr.h>
#include <soc/t8002/a0/tunable/pmgr.h>
#include <soc/t8002/a0/tunable/cpu_fabric.h>
#include <soc/t8002/a0/tunable/nrt_fabric.h>
#include <soc/t8002/a0/tunable/uvdc.h>
#include <soc/t8002/a0/tunable/pio_wrap.h>

static const struct tunable_struct pmgr_tunable_t8002_a0[] = {
    PMS_PMGR_DEFAULT_TUNABLES_T8002_A0
};

static const struct tunable_struct minipmgr_tunable_t8002_a0[] = {
    AOP_MINIPMGR_DEFAULT_TUNABLES_T8002_A0
};

static const struct tunable_struct cpu_fabric_tunable_t8002_a0[] = {
    PIO_CPU_FABRIC_DEFAULT_TUNABLES_T8002_A0
};

static const struct tunable_struct nrt_fabric_tunable_t8002_a0[] = {
    PIO_NRT_FABRIC_DEFAULT_TUNABLES_T8002_A0
};

static const struct tunable_struct uvdc_tunable_t8002_a0[] = {
    PMS_UVDC_DEFAULT_TUNABLES_T8002_A0
};

static const struct tunable_struct pio_wrap_tunable_t8002_a0[] = {
    PIO_PIO_WRAP_DEFAULT_TUNABLES_T8002_A0
};
/***************************************** END CHIP_REVISION_A0 *****************************************/

const struct tunable_chip_struct tunables_pmgr[] = {
    // B0
    {CHIP_REVISION_B0,         PMS_PMGR_PADDR_T8002_B0,         pmgr_tunable_t8002_b0,       NULL, true},
    {CHIP_REVISION_B0,         AOP_MINIPMGR_PADDR_T8002_B0,     minipmgr_tunable_t8002_b0,   NULL, false},
    {CHIP_REVISION_B0,         PIO_CPU_FABRIC_PADDR_T8002_B0,   cpu_fabric_tunable_t8002_b0, NULL, true},
    {CHIP_REVISION_B0,         PIO_NRT_FABRIC_PADDR_T8002_B0,   nrt_fabric_tunable_t8002_b0, NULL, true},
    {CHIP_REVISION_B0,         PMS_UVDC_PADDR_T8002_B0,         uvdc_tunable_t8002_b0,       NULL, true},
    {CHIP_REVISION_B0,         PIO_PIO_WRAP_PADDR_T8002_B0,     pio_wrap_tunable_t8002_b0,   NULL, true},

    // A0
    {CHIP_REVISION_A0,         PMS_PMGR_PADDR_T8002_A0,         pmgr_tunable_t8002_a0,       NULL, true},
    {CHIP_REVISION_A0,         AOP_MINIPMGR_PADDR_T8002_A0,     minipmgr_tunable_t8002_a0,   NULL, false},
    {CHIP_REVISION_A0,         PIO_CPU_FABRIC_PADDR_T8002_A0,   cpu_fabric_tunable_t8002_a0, NULL, true},
    {CHIP_REVISION_A0,         PIO_NRT_FABRIC_PADDR_T8002_A0,   nrt_fabric_tunable_t8002_a0, NULL, true},
    {CHIP_REVISION_A0,         PMS_UVDC_PADDR_T8002_A0,         uvdc_tunable_t8002_a0,       NULL, true},
    {CHIP_REVISION_A0,         PIO_PIO_WRAP_PADDR_T8002_A0,     pio_wrap_tunable_t8002_a0,   NULL, true},

    {-1, -1, NULL, NULL, false},
};

const struct tunable_filtered_chip_struct tunables_filtered_pmgr[] = {
    // B0
    // Applied in OS
    {CHIP_REVISION_B0, AOP_MINIPMGR_PADDR_T8002_B0 + MINIPMGR_MINI_CLKCFG_PROXY_OSC_CLK_CFG_OFFSET, AOP_MINIPMGR_PADDR_T8002_B0 + MINIPMGR_MINI_CLKCFG_XI0MUX_CLK_CFG_OFFSET, true, true},
    {CHIP_REVISION_B0, AOP_MINIPMGR_PADDR_T8002_B0 + MINIPMGR_MINI_MISC_CFG_ACG_OFFSET, AOP_MINIPMGR_PADDR_T8002_B0 + MINIPMGR_MINI_MISC_CFG_ACG_OFFSET, true, true},
    {CHIP_REVISION_B0, PMS_PMGR_PADDR_T8002_B0 + PMGR_MISC_CFG_ACG_OFFSET, PMS_PMGR_PADDR_T8002_B0 + PMGR_MISC_CFG_ACG_OFFSET, true, true},

    // A0
    // Applied in OS
    {CHIP_REVISION_A0, AOP_MINIPMGR_PADDR_T8002_A0 + MINIPMGR_MINI_CLKCFG_PROXY_OSC_CLK_CFG_OFFSET, AOP_MINIPMGR_PADDR_T8002_A0 + MINIPMGR_MINI_CLKCFG_XI0MUX_CLK_CFG_OFFSET, true, true},
    {CHIP_REVISION_A0, AOP_MINIPMGR_PADDR_T8002_A0 + MINIPMGR_MINI_MISC_CFG_ACG_OFFSET, AOP_MINIPMGR_PADDR_T8002_A0 + MINIPMGR_MINI_MISC_CFG_ACG_OFFSET, true, true},
    {CHIP_REVISION_A0, PMS_PMGR_PADDR_T8002_A0 + PMGR_MISC_CFG_ACG_OFFSET, PMS_PMGR_PADDR_T8002_A0 + PMGR_MISC_CFG_ACG_OFFSET, true, true},

    {-1, -1, -1, NULL, false},
};
