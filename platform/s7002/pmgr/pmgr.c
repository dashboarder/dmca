/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/power.h>
#include <platform/timer.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/int.h>
#include <sys/boot.h>
#include <target.h>

#define PLL_VCO_TARGET(pllx)    ((2ULL * (pllx##_O) * (pllx##_M)) / (pllx##_P))
#define PLL_FREQ_TARGET(pllx)   (((pllx##_O) * (pllx##_M)) / (pllx##_P) / ((pllx##_S) + 1))

struct clock_source {
    uint32_t    src_clk;
    uint32_t    factor;
};

struct clock_config {
    volatile uint32_t	*clock_reg;     // CLK_CFG Register
    struct clock_source sources[8];    // List of sources
};

/* ******************************************************************************** */
#if APPLICATION_IBOOT

/* PLL0 @532.8MHz */
#define PLL0        0
#define PLL0_O      OSC_FREQ
#define PLL0_P      5
#define PLL0_M      111
#define PLL0_S      0
#define PLL0_V      PLL_VCO_TARGET(PLL0)
#define PLL0_T      PLL_FREQ_TARGET(PLL0)

static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
    0x80100000, // gpio             on  24MHz
    0x83100000, // mcu              on  533MHz, mcu_cfg_sel 0 (highest freq)
    0x81100000, // mcu_fixed        on  533MHz
    0x84100000,	// gfx              on  133.25MHz
    0x83100000, // mipi_dsi         on  533MHz
    0x83100000, // vid              on  22.21MHz
    0x83100000, // media            on  133.25MHz
    0x83100000, // disp             on  106.6MHz
    0x84100000, // sdio             on  88.8MHz
    0x83100000, // ans              on  106.6MHz 
    0x83100000, // pio              on  106.6MHz
    0x83100000,	// lio              on  106.6MHz
    0x83100000, // aue              on  106.6MHz
    0x83100000, // usb              on  53.3MHz
    0x83100000, // spi0             on  53.3MHz
    0x83100000, // spi1             on  53.3MHz
    0x83100000, // nco_ref0         on  266.5MHz
    0x83100000, // nco_ref1         on  266.5MHz
    0x80100000, // nco_alg0         on  24MHz
    0x80100000, // nco_alg1         on  24MHz
    0x83100000, // mca0_m           on  nco_ref0
    0x84100000  // mca1_m           on  nco_ref1
};

static const uint32_t spare_divs_active[PMGR_SPARE_CLK_CFG_COUNT] = {
    0x00000000, // s0               off
    0x00000000  // s1               off
};

#endif /* APPLICATION_IBOOT */
/* ******************************************************************************** */
#if APPLICATION_SECUREROM

/* PLL0 @266.4MHz */
#define PLL0        0
#define PLL0_O      OSC_FREQ
#define PLL0_P      10
#define PLL0_M      111
#define PLL0_S      0
#define PLL0_V      PLL_VCO_TARGET(PLL0)
#define PLL0_T      PLL_FREQ_TARGET(PLL0)

// We won't touch the clk gen's that aren't necessary during SecureROM.
static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
    0x80100000, // gpio             on  24MHz
    0x83100000, // mcu              on  266.4MHz, mcu_cfg_sel 0 (highest freq)
    0x81100000, // mcu_fixed        on  266.4MHz
    0x80100000,	// gfx              on  24MHz
    0x80100000, // mipi_dsi         on  24MHz
    0x80100000, // vid              on  24MHz
    0x80100000, // media            on  24MHz
    0x80100000,	// disp             on  24MHz
    0x80100000, // sdio             on  24MHz
    0x83100000, // ans              on  53.3MHz 
    0x83100000, // pio              on  53.3MHz
    0x80100000,	// lio              on  24MHz
    0x80100000, // aue              on  24MHz
    0x84100000, // usb              on  53.3MHz
    0x80100000, // spi0             on  24MHz
    0x80100000, // spi1             on  24MHz
    0x80100000, // nco_ref0         on  24MHz
    0x80100000, // nco_ref1         on  24MHz
    0x80100000, // nco_alg0         on  24MHz
    0x80100000, // nco_alg1         on  24MHz
    0x80100000, // mca0_m           on  24MHz
    0x80100000  // mca1_m           on  24MHz
};

static const uint32_t spare_divs_active[PMGR_SPARE_CLK_CFG_COUNT] = {
    0x80000001, // s0               on  266.4MHz 
    0x80000001  // s1               on  266.4MHz
};

#endif /* APPLICATION_SECUREROM */
/* ******************************************************************************** */
#define CLOCK_SOURCES_MAX   8

static const struct clock_config clk_configs[PMGR_CLK_COUNT] = {
    [PMGR_CLK_GPIO] = { 
        &rPMGR_GPIO_CLK_CFG, 
        { 
            {PMGR_CLK_OSC, 1}
        }
    },

    [PMGR_CLK_MCU] = { 
        &rPMGR_MCU_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 1},
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 3},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 5},
        }
    },

    [PMGR_CLK_MCU_FIXED] = { 
        &rPMGR_MCU_FIXED_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_PLL0, 1},
        }
    },

    [PMGR_CLK_GFX] = { 
        &rPMGR_GFX_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 3},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
        }
    },

    [PMGR_CLK_MIPI_DSI] = { 
        &rPMGR_MIPI_DSI_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 1},
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
        }
    },

    [PMGR_CLK_VID] = { 
        &rPMGR_VID_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 24},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 3},
            {PMGR_CLK_OSC, 4},
        }
    },

    [PMGR_CLK_MEDIA] = { 
        &rPMGR_MEDIA_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
        }
    },

    [PMGR_CLK_DISP] = { 
        &rPMGR_DISP_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
        }
    },

    [PMGR_CLK_SDIO] = { 
        &rPMGR_SDIO_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
        }
    },

    [PMGR_CLK_ANS] = { 
        &rPMGR_ANS_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
        }
    },

    [PMGR_CLK_PIO] = { 
        &rPMGR_PIO_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_OSC, 4},
        }
    },

    [PMGR_CLK_LIO] = { 
        &rPMGR_LIO_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_OSC, 4},            
        }
    },

    [PMGR_CLK_AUE] = { 
        &rPMGR_AUE_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 5},
            {PMGR_CLK_PLL0, 4},
        }
    },

    [PMGR_CLK_USB] = { 
        &rPMGR_USB_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 5},
        }
    },

    [PMGR_CLK_SPI0_N] = { 
        &rPMGR_SPI0_N_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 20},
        }
    },

    [PMGR_CLK_SPI1_N] = { 
        &rPMGR_SPI1_N_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 20},
        }
    },

    [PMGR_CLK_NCO_REF0] = { 
        &rPMGR_NCO_REF0_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
        }
    },

    [PMGR_CLK_NCO_REF1] = { 
        &rPMGR_NCO_REF1_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
        }
    },

    [PMGR_CLK_NCO_ALG0] = { 
        &rPMGR_NCO_ALG0_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 10},
        }
    },

    [PMGR_CLK_NCO_ALG1] = { 
        &rPMGR_NCO_ALG1_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 10},
        }
    },

    [PMGR_CLK_MCA0_M] = { 
        &rPMGR_MCA0_M_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_NOT_SUPPORTED, 1},
            {PMGR_CLK_NOT_SUPPORTED, 1},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 4},
        }
    },

    [PMGR_CLK_MCA1_M] = { 
        &rPMGR_MCA1_M_CLK_CFG,
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_NOT_SUPPORTED, 1},
            {PMGR_CLK_NOT_SUPPORTED, 1},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 4},
        }
    },

    [PMGR_CLK_S0] = { 
        &rPMGR_S0_CLK_CFG,
        {
            {PMGR_CLK_PLL0, 1},
        }
    },

    [PMGR_CLK_S1] = { 
        &rPMGR_S1_CLK_CFG,
        {
            {PMGR_CLK_PLL0, 1},        }
    },
    
};

/* ******************************************************************************** */

static void set_pll(uint32_t p, uint32_t m, uint32_t s);
static uint32_t get_pll_frequency();
static uint32_t get_spare_frequency(uint32_t cnt);

static void clocks_get_frequencies(void);
static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk);
static void clocks_quiesce_internal(void);

static void apply_pmgr_tunables();

static void update_memory_clk_config(uint32_t performance_level);

// current clock frequencies
static uint32_t clks[PMGR_CLK_COUNT + 1];

/* ******************************************************************************** */

static void set_pll(uint32_t p, uint32_t m, uint32_t s)
{
    rPMGR_PLL0_CTL = (PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s) | PMGR_PLL_LOAD);
    while ((rPMGR_PLL0_CTL & PMGR_PLL_PENDING) == 1);

    rPMGR_PLL0_EXT_BYPASS_CFG &= ~PMGR_PLL_EXT_BYPASS;
    while ((rPMGR_PLL0_EXT_BYPASS_CFG & PMGR_PLL_BYP_ENABLED) != 0);

    rPMGR_PLL0_CTL |= PMGR_PLL_ENABLE;
    while ((rPMGR_PLL0_CTL & PMGR_PLL_PENDING) == 1);
}

static uint32_t get_pll_frequency()
{
    uint32_t pllctl, bypcfg;
    uint64_t freq = 0;

    pllctl = rPMGR_PLL0_CTL;
    bypcfg = rPMGR_PLL0_EXT_BYPASS_CFG;

    // If PLL is not enabled, check for External Bypass
    if ((pllctl & PMGR_PLL_ENABLE) == 0) {
        if ((bypcfg & PMGR_PLL_EXT_BYPASS) == 0) 
            return 0;
        else 
            return OSC_FREQ;
    }

    freq = OSC_FREQ;
    freq *= ((pllctl >> PMGR_PLL_M_SHIFT) & PMGR_PLL_M_MASK);
    freq /= ((pllctl >> PMGR_PLL_P_SHIFT) & PMGR_PLL_P_MASK);
    freq /= (1 + ((pllctl >> PMGR_PLL_S_SHIFT) & PMGR_PLL_S_MASK));

#if DEBUG_BUILD
    if (freq> 0xFFFFFFFF)
        panic("Frequency value does not fit in uint32_t");
#endif

    return (uint32_t)freq;
}

static uint32_t get_spare_frequency(uint32_t spare)
{
    uint32_t reg_val, src_idx, src_clk, src_factor, div;
    volatile uint32_t *spare_clkcfg = clk_configs[PMGR_CLK_S0 + spare].clock_reg;

    reg_val = *spare_clkcfg;

    div = reg_val & 0x3F;

    if (((reg_val & PMGR_CLK_CFG_ENABLE) == 0) || div == 0)
          return 0;

    src_idx = 0; // Source index is always 0 because only spare source is PLL0
    src_clk = clk_configs[PMGR_CLK_S0 + spare].sources[src_idx].src_clk;
    src_factor = clk_configs[PMGR_CLK_S0 + spare].sources[src_idx].factor;

    return (clks[src_clk] / src_factor) / div;
}

static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk)
{
    volatile uint32_t *reg;
    uint32_t cnt, val, src_idx, src_clk, src_factor;

    if (start_clk < PMGR_CLK_GPIO || end_clk > PMGR_CLK_MCA1_M)
        return;

    for (cnt = start_clk; cnt <= end_clk; cnt++) {
        reg = clk_configs[cnt].clock_reg;
        val = *reg;

        if ((val & PMGR_CLK_CFG_ENABLE) == 0) {
             clks[cnt] = 0;
             continue;
        }

        src_idx = (val >> 24) & PMGR_CLK_CFG_SRC_SEL_MASK;
        src_clk = clk_configs[cnt].sources[src_idx].src_clk;
        src_factor = clk_configs[cnt].sources[src_idx].factor;
        clks[cnt] = clks[src_clk] / src_factor;
    }
}

static void clocks_get_frequencies(void)
{
#if SUPPORT_FPGA
    uint32_t cnt;
    uint32_t freq = OSC_FREQ;

    for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
    {
        clks[cnt] = freq;
    }
    
    clks[PMGR_CLK_MCU] = 10000000;
    clks[PMGR_CLK_MCU_FIXED]= 10000000;
    clks[PMGR_CLK_USB]	= 12000000;

#elif CONFIG_SIM
    uint32_t cnt;
    uint32_t freq = OSC_FREQ;

    for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
    clks[cnt] = freq;

#else
    uint32_t cnt;

    clks[PMGR_CLK_OSC] = OSC_FREQ;

    // Use get_pll_frerquency() to establish the frequency (unconfigured PLLs will bypass OSC)
    clks[PMGR_CLK_PLL0] = get_pll_frequency();

    // Use get_spare_frequencies() to establish the frequencies for spare clocks (unconfigured will be skipped)
    for (cnt = 0; cnt < PMGR_SPARE_CLK_CFG_COUNT; cnt++) 
    {
        clks[PMGR_CLK_S0 + cnt] = get_spare_frequency(cnt);
    }

    clocks_get_frequencies_range(PMGR_CLK_GPIO, PMGR_CLK_MCA1_M);
#endif
}

int clocks_init(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS))
    clocks_get_frequencies();
#endif /* (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)) */
    return 0;
}

static bool clk_mipi_req_on = false;
static bool clk_disp0_req_on = false;

void clock_gate(int device, bool enable)
{

    volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device);
    
    // Make sure we are within limits
    if (reg > PMGR_LAST_PS) 
        return;

    // Workaround for <rdar://problem/16051906> /  <rdar://problem/15883349>
    // Keep track of the pending requests for CLK_DISP0 and CLK_MIPI_DSI
    switch(device)
    {
        case CLK_DISP0:
            clk_disp0_req_on = enable;
            break;
        case CLK_MIPI_DSI:
            clk_mipi_req_on = enable;
            break;
    }

    if (!enable && ((device == CLK_DISP0) || (device == CLK_MIPI_DSI)))
    {
        // Workaround for <rdar://problem/16051906>
        // if requesting DISP0 or MIPI off, make sure both are requested off before effecting the change.
        if (clk_disp0_req_on || clk_mipi_req_on)
            return;

        // Otherwise clock_gate the other one before clock gating this one.
        int device2 = (device == CLK_DISP0) ? CLK_MIPI_DSI : CLK_DISP0;
        volatile uint32_t *reg2 = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device2);
        *reg2 &= ~PMGR_PS_RUN_MAX;
        while ((*reg2 & PMGR_PS_MANUAL_PS_MASK) != ((*reg2 >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
    }
    // Set the PS field to the requested level
    if (enable) 
        *reg |= PMGR_PS_RUN_MAX;
    else
        *reg &= ~PMGR_PS_RUN_MAX; // i.e. set PMGR_PS_POWER_OFF

    // Wait for the MANUAL_PS and ACTUAL_PS fields to be equal
    while ((*reg & PMGR_PS_MANUAL_PS_MASK) != ((*reg >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
    
    // Workaround for <rdar://problem/15883349>
    //  If device == MIPI_DSI | DISP, turn the enable the other one as well
    if (enable && ((device == CLK_DISP0) || (device == CLK_MIPI_DSI)))
    {
        int device2 = (device == CLK_DISP0) ? CLK_MIPI_DSI : CLK_DISP0;
        volatile uint32_t *reg2 = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device2);
        *reg2 |= PMGR_PS_RUN_MAX;
        while ((*reg2 & PMGR_PS_MANUAL_PS_MASK) != ((*reg2 >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
    }    
}

static void restore_clock_config_reset_state()
{
    volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
    uint32_t reg;

    // 2. Write reset value to ACG, CLK_DIVIDER_ACG_CFG, CLK_DIVIDER_ACG_CFG1, and PLL_ACG_CFG 
    rPMGR_MISC_ACG = 0;
    rPMGR_CLK_DIVIDER_ACG_CFG = 0;
    rPMGR_CLK_POWER_CONFIG = 0;

    // 5. Write reset value for all mux clock configs (excluding spares, mcu, mcu_fixed)
    reg = PMGR_CLK_NUM(GPIO);
    while (reg <= PMGR_CLK_NUM(MCA1_M)) {
	if ((reg == PMGR_CLK_NUM(MCU)) || (reg == PMGR_CLK_NUM(MCU_FIXED))) goto skip;
	clkcfgs[reg] = 0x80100000;
	while (clkcfgs[reg] & PMGR_CLK_CFG_PENDING);
skip:	
	reg++;
    }

    // 6. Write to MCU and MCU_FIXED to allow memory to run at 24MHz
    update_memory_clk_config(kPerformanceMemoryLow);

    // 7. Write PLL0_EXT_BYPASS_CFG.EXT_BYPASS to 1
    rPMGR_PLL0_EXT_BYPASS_CFG |= PMGR_PLL_EXT_BYPASS;
    while ((rPMGR_PLL0_EXT_BYPASS_CFG & PMGR_PLL_BYP_ENABLED) == 0);

    // 8. Write reset value to PLL0_CTL 
    rPMGR_PLL0_CTL = 0x00001010;

    // 10. Write reset value spares
    reg = PMGR_SPARE_CLK_NUM(S0);
    while (reg <= PMGR_SPARE_CLK_NUM(S1)) {
	clkcfgs[reg] = 0x80000001;
	while (clkcfgs[reg] & PMGR_CLK_CFG_PENDING);
	reg++;
    }
}

static void clocks_quiesce_internal(void)
{
    //clock_gate(PMGR_CPU, 1);
    clock_gate(CLK_AIC, 1);
    clock_gate(CLK_LIO, 1);
    clock_gate(CLK_GPIO, 1);
    clock_gate(CLK_MCU, 1);
    clock_gate(CLK_AMP, 1);
    clock_gate(CLK_PIOSYS, 1);
    clock_gate(CLK_SPU, 1);
    clock_gate(CLK_SPU_SGPIO, 1);
    clock_gate(CLK_AUE, 1);
    clock_gate(CLK_USB_M7, 1);
    clock_gate(CLK_AES0, 1);
    clock_gate(CLK_DOCKFIFO, 1);
    clock_gate(CLK_ANS, 1);

    // DEBUG_PS lives in SPU, enable it as well
    rSPU_PMGR_DEBUG_PS |= PMGR_PS_RUN_MAX;
    while ((rSPU_PMGR_DEBUG_PS & PMGR_PS_MANUAL_PS_MASK) != ((rSPU_PMGR_DEBUG_PS >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
    
    clock_gate(CLK_SPU_AKF, 0);    
    // <rdar://problem/17191216> SPU_SMB_PS needs to be clocked on in iBoot and/or not touched by iOS sleep/wake
    // SPU_*_PS should be on by default, without them in EDT the OS should then leave them alone.
    clock_gate(CLK_SPU_UART0, 1);
    clock_gate(CLK_SPU_UART1, 1);
    clock_gate(CLK_SPU_SMB, 1);

    clock_gate(CLK_DISP0, 0);
    clock_gate(CLK_MIPI_DSI, 0);
    clock_gate(CLK_MSR, 0);
    clock_gate(CLK_GFX, 0);
    clock_gate(CLK_SDIO, 0);
    clock_gate(CLK_MCA0, 0);
    clock_gate(CLK_MCA1, 0);
    clock_gate(CLK_SPI0, 0);
    clock_gate(CLK_SPI1, 0);
    clock_gate(CLK_DMATX, 0);
    clock_gate(CLK_DMARX, 0);
    clock_gate(CLK_UART0, 0);
    clock_gate(CLK_UART1, 0);
    clock_gate(CLK_UART2, 0);
    clock_gate(CLK_UART3, 0);
    clock_gate(CLK_UART4, 0);
    clock_gate(CLK_I2C0, 0);
    clock_gate(CLK_I2C1, 0);
    clock_gate(CLK_PWM0, 0);
        
    clock_gate(CLK_ETH, 0);
    clock_gate(CLK_VDEC, 0);

    restore_clock_config_reset_state();
}

static void apply_pmgr_tunables()
{
#define CLAMP_TIME_MASK         (((1 << 8) - 1) << 8)
#define CLK_EN_TIME_MASK        (((1 << 8) - 1) << 16)
#define SRAM_PG_EN_TIME_MASK    (((1 << 3) - 1) << 24)
#define SRAM_PG_DIS_TIME_MASK   (((1 << 3) - 1) << 27)
#define _PWRGATE_CFG0(rCFG0, CLAMP_TIME, CLK_EN_TIME, SRAM_PG_EN_TIME, SRAM_PG_DIS_TIME) \
    regTemp = rCFG0; \
    regTemp &= ~CLAMP_TIME_MASK; \
    regTemp |= (CLAMP_TIME << 8); \
    regTemp &= ~CLK_EN_TIME_MASK; \
    regTemp |= (CLK_EN_TIME << 16); \
    regTemp &= ~SRAM_PG_EN_TIME_MASK; \
    regTemp |= (SRAM_PG_EN_TIME << 24); \
    regTemp &= ~SRAM_PG_DIS_TIME_MASK; \
    regTemp |= (SRAM_PG_DIS_TIME << 27); \
    rCFG0 = regTemp;

#define RAMP_PRE_TIME_MASK      (((1 << 12) - 1) << 0)    
#define RAMP_ALL_TIME_MASK      (((1 << 12) - 1) << 16)
#define _PWRGATE_CFG1(rCFG1, RAMP_PRE_TIME, RAMP_ALL_TIME) \
    regTemp = rCFG1; \
    regTemp &= ~RAMP_PRE_TIME_MASK; \
    regTemp |= (RAMP_PRE_TIME << 0); \
    regTemp &= ~RAMP_ALL_TIME_MASK; \
    regTemp |= (RAMP_ALL_TIME << 16); \
    rCFG1 = regTemp;
    
#define RESET_DOWN_TIME_MASK    (0xFF << 0)
#define RESET_UP_TIME_MASK      (0xFF << 8)
#define RESET_OFF_TIME_MASK     (0xFF << 16)
#define _PWRGATE_CFG2(rCFG2, RESET_DOWN_TIME, RESET_UP_TIME, RESET_OFF_TIME) \
    regTemp = rCFG2; \
    regTemp &= ~RESET_DOWN_TIME_MASK; \
    regTemp |= (RESET_DOWN_TIME << 0); \
    regTemp &= ~RESET_UP_TIME_MASK; \
    regTemp |= (RESET_UP_TIME << 8); \
    regTemp &= ~RESET_OFF_TIME_MASK; \
    regTemp |= (RESET_OFF_TIME << 16); \
    rCFG2 = regTemp;
    
    uint32_t regTemp;
    
    _PWRGATE_CFG0(rPMGR_PWR_CPU_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_CPU_CFG1, 0x4, 0x6b);
    _PWRGATE_CFG2(rPMGR_PWR_CPU_CFG2, 0x8, 0x8, 0x8);
    
    _PWRGATE_CFG0(rPMGR_PWR_AMCPIO_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_AMCPIO_CFG1, 0x0, 0x1);
    _PWRGATE_CFG2(rPMGR_PWR_AMCPIO_CFG2, 0x28, 0x28, 0x28);

    _PWRGATE_CFG0(rPMGR_PWR_DISP_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_DISP_CFG1, 0x5, 0x66);
    _PWRGATE_CFG2(rPMGR_PWR_DISP_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_DISP_BE_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_DISP_BE_CFG1, 0x1, 0x1b);
    _PWRGATE_CFG2(rPMGR_PWR_DISP_BE_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_MSR_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_MSR_CFG1, 0x3, 0x3a);
    _PWRGATE_CFG2(rPMGR_PWR_MSR_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_ANS_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_ANS_CFG1, 0x2, 0x78);
    _PWRGATE_CFG2(rPMGR_PWR_ANS_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_GFX_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_GFX_CFG1, 0x3, 0x49);
    _PWRGATE_CFG2(rPMGR_PWR_GFX_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_SDIO_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_SDIO_CFG1, 0x2, 0x3a);
    _PWRGATE_CFG2(rPMGR_PWR_SDIO_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_LIO_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_LIO_CFG1, 0x1, 0x31);
    _PWRGATE_CFG2(rPMGR_PWR_LIO_CFG2, 0x8, 0xc, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_AUE_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_AUE_CFG1, 0x1, 0x1f);
    _PWRGATE_CFG2(rPMGR_PWR_AUE_CFG2, 0x8, 0x8, 0x8);

    _PWRGATE_CFG0(rPMGR_PWR_VDEC_CFG0, 0x3, 0x4, 0x4, 0x4);
    _PWRGATE_CFG1(rPMGR_PWR_VDEC_CFG1, 0x3, 0x30);
    _PWRGATE_CFG2(rPMGR_PWR_VDEC_CFG2, 0x8, 0x8, 0x8);

    rPMGR_MCU_ASYNC_RESET = (0x1 << 28) | (0x1 << 24) | (0x1 << 20) | (0x1 << 16) | (0x1 << 8) | (0x1 << 4) | (0x1 << 0);
    
    rPMGR_MISC_GFX_CTL = (0x1 << 0);
}

static void update_memory_clk_config(uint32_t performance_level)
{
	int32_t cfg_sel = -1;
        uint32_t src_index;
	volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	uint32_t mcu_clk_cfg_reg, mcu_fixed_clk_cfg_reg;
	uint8_t current_mcu_fixed_clk;

        switch(performance_level) {
            case kPerformanceMemoryLow:
                cfg_sel = 3;
                break;

            case kPerformanceMemoryFull:
                cfg_sel = 0;
                break;
        }

        if (cfg_sel == -1)
            panic("pmgr:cfg_sel not set correctly for configuring amc clock");

        src_index = (performance_level == kPerformanceMemoryLow) ? 0 : 3;

	mcu_clk_cfg_reg = clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)];
	mcu_fixed_clk_cfg_reg = clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU_FIXED)];

	mcu_clk_cfg_reg &= ~(0x7 << PMGR_CLK_CFG_SRC_SEL_SHIFT);
	mcu_clk_cfg_reg &= ~(0x3 << PMGR_CLK_CFG_CFG_SEL_SHIFT);
	mcu_clk_cfg_reg |= ((src_index & 0x7) << PMGR_CLK_CFG_SRC_SEL_SHIFT);
	mcu_clk_cfg_reg |= ((cfg_sel & 0x3) << PMGR_CLK_CFG_CFG_SEL_SHIFT);

	current_mcu_fixed_clk = (mcu_fixed_clk_cfg_reg >> PMGR_CLK_CFG_SRC_SEL_SHIFT) & 0x1;

	mcu_fixed_clk_cfg_reg &= ~(1 << PMGR_CLK_CFG_SRC_SEL_SHIFT);
	mcu_fixed_clk_cfg_reg |= ((src_index & 0x1) << PMGR_CLK_CFG_SRC_SEL_SHIFT);

	if (current_mcu_fixed_clk == 0) {
		clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU_FIXED)] = mcu_fixed_clk_cfg_reg;
		while ((clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU_FIXED)] >> PMGR_CLK_CFG_PENDING_SHIFT) & 0x1);
		clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)] = mcu_clk_cfg_reg;
		while ((clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)] >> PMGR_CLK_CFG_PENDING_SHIFT) & 0x1);
	}
	else {
		clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)] = mcu_clk_cfg_reg;
		while ((clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)] >> PMGR_CLK_CFG_PENDING_SHIFT) & 0x1);
		clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU_FIXED)] = mcu_fixed_clk_cfg_reg;
		while ((clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU_FIXED)] >> PMGR_CLK_CFG_PENDING_SHIFT) & 0x1);
	}
}

static void set_nco_clocks(void)
{
    // Enable this NCO with alg_ref0_clk and nco_ref0_clk.
    rPMGR_NCO_CLK_CFG(0) |= (1 << 31);
    rPMGR_NCO_CLK_CFG(1) |= (1 << 31);
}

/* 
 * clocks_set_default - called by SecureROM, LLB, iBSS main via
 * platform_init_setup_clocks, so the current state of the chip is
 * either POR, or whatever 'quiesce' did when leaving SecureROM.
 */
int clocks_set_default(void)
{
    uint32_t cnt;
    volatile uint32_t *spare_clkcfgs = PMGR_FIRST_SPARE_CLK_CFG;
    volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;

    clks[PMGR_CLK_OSC] = OSC_FREQ;

    // Change all clocks to something safe
    clocks_quiesce_internal();

#ifdef PLL0_T
    set_pll(PLL0_P, PLL0_M, PLL0_S);
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
    // Turn on NCO clocks before enabling MCA clocks.
    set_nco_clocks();
#endif

    // Set all spare clock divs to their active values
    for (cnt = 0; cnt < PMGR_SPARE_CLK_CFG_COUNT; cnt++) {
        spare_clkcfgs[cnt] = spare_divs_active[cnt];
        while ((spare_clkcfgs[cnt] & PMGR_CLK_CFG_PENDING) != 0);
    }

    // Set all but the spare clock divs to their active values
    for (cnt = 0; cnt < PMGR_CLK_CFG_COUNT; cnt++) {
        clkcfgs[cnt] = clk_divs_active[cnt];
        while ((clkcfgs[cnt] & PMGR_CLK_CFG_PENDING) != 0);
    }

    clocks_get_frequencies();

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
    apply_pmgr_tunables();
#endif
    return 0;
}

void clocks_quiesce(void)
{
    clocks_quiesce_internal();
}

uint32_t clocks_set_performance(uint32_t performance_level)
{
#if APPLICATION_IBOOT
    if (performance_level == kPerformanceMemoryLow || performance_level == kPerformanceMemoryFull) {
        update_memory_clk_config(performance_level);
        clocks_get_frequencies_range(PMGR_CLK_MCU_FIXED, PMGR_CLK_MCU);
    }
#endif

	return kPerformanceHigh;
}

void clock_get_frequencies(uint32_t *clocks, uint32_t count)
{
    uint32_t cnt = PMGR_CLK_COUNT;

    if (cnt > count) cnt = count;

    memcpy(clocks, clks, cnt * sizeof(uint32_t));
}

uint32_t clock_get_frequency(int clock)
{
    uint32_t freq = OSC_FREQ;

    switch (clock) {
        case CLK_NCLK:
        case CLK_FIXED:
        case CLK_TIMEBASE:
            freq = clks[PMGR_CLK_OSC];
            break;
        case CLK_ANS_LINK:
            freq = clks[PMGR_CLK_ANS];
            break;
        case CLK_BUS:
        case CLK_PCLK:
        case CLK_PERIPH:
            freq = clks[PMGR_CLK_LIO];
            break;
        case CLK_MEM:
            freq = clks[PMGR_CLK_MCU];
            break;            
        case CLK_CPU:
            freq = clks[PMGR_CLK_MCU];
            break;
        case CLK_MIPI:
            freq = clks[PMGR_CLK_MIPI_DSI];
            break;
        default:
        break;
    }

    return freq;
}

static void clock_update_frequency(uint32_t clk, uint32_t freq)
{
    uint32_t src_idx, src_clk, src_factor, reg;
    bool freq_supported = false;
    volatile uint32_t *clkcfg = clk_configs[clk].clock_reg;
    
    if (freq == 0)
    {
        return;
    }
    
    for (src_idx = 0; src_idx < CLOCK_SOURCES_MAX && clk_configs[clk].sources[src_idx].factor != 0; src_idx++)
    {
        src_clk = clk_configs[clk].sources[src_idx].src_clk;
        src_factor = clk_configs[clk].sources[src_idx].factor;
        
        // Round the requested frequency to closest MHz value and check if we have a match
        if ((freq / 1000000) == ((clks[src_clk] / src_factor) / 1000000))
        {
            freq_supported = true;
            break;
        }	
    }

    if (freq_supported) 
    {
        // Configure clock
        reg = *clkcfg;
        reg &= ~(PMGR_CLK_CFG_SRC_SEL_MASK << PMGR_CLK_CFG_SRC_SEL_SHIFT);
        reg |= (src_idx & PMGR_CLK_CFG_SRC_SEL_MASK) << PMGR_CLK_CFG_SRC_SEL_SHIFT;
        *clkcfg = reg;
        while (*clkcfg & PMGR_CLK_CFG_PENDING);
    }
}

void clock_set_frequency(int clock, uint32_t divider, uint32_t pll_p, uint32_t pll_m, uint32_t pll_s, uint32_t pll_t)
{
    uint32_t clk = PMGR_CLK_OSC;

	switch (clock) {

        case CLK_VCLK0:
            clk = PMGR_CLK_VID;
            break;

        default:
            break;
    }
    
    if (clk >= PMGR_CLK_SOURCE_FIRST && clk <= PMGR_CLK_SOURCE_LAST) {
        clock_update_frequency(clk, pll_t);
        clocks_get_frequencies_range(clk, clk);
	}
}

void clock_reset_device(int device)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device);

    switch (device) {
        case CLK_MCU:
        default:
            *reg |= PMGR_PS_RESET;
            spin(1);
            *reg &= ~PMGR_PS_RESET;
        break;
    }
}

void platform_system_reset(bool panic)
{
#if WITH_BOOT_STAGE
    if (!panic) boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

    wdt_system_reset();

    while (1);
}

void platform_reset(bool panic)
{
#if WITH_BOOT_STAGE
    if (!panic) boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

    wdt_chip_reset();

    while (1);
}

void platform_power_init(void)
{
}

void platform_power_spin(uint32_t usecs)
{
    extern void aic_spin(uint32_t usecs);
    aic_spin(usecs);
}

#if WITH_DEVICETREE
void pmgr_update_device_tree(DTNode *pmgr_node)
{
}
#endif

/*
 * Timer support
 */
#define DECR_MAX_COUNT	(INT32_MAX)

static void timer_deadline(void *arg);
static void (* timer_deadline_func)(void);

int timer_init(uint32_t timer)
{
    /* Allow only banked access to CPU's own timer */
    if (timer != 0) return -1;

    rPMGR_INTERVAL_TMR_CTL = (1 << 0) | (1 << 8);

    return 0;
}

void timer_stop_all(void)
{
    rPMGR_INTERVAL_TMR_CTL &= ~(1 << 0);
    rPMGR_INTERVAL_TMR_CTL = (1 << 8);
}

void timer_deadline_enter(uint64_t deadline, void (* func)(void))
{
    uint64_t	ticks;
    uint32_t	decr;

    timer_deadline_func = func;
    if (func) {
//      printf("installing deadline %p\n", func);
        
        /* convert absolute deadline to relative time */
        ticks = timer_get_ticks();
        if (deadline <= ticks) {
            // If the desired deadline would be 0 or negative, we ensure it is at least 1 tick
            // in order to fire the timer immediately and not wrap around.
            deadline = 1;
        } else {
            deadline -= ticks;
        }

        /* clamp the deadline to our maximum, which is about 178 seconds */
        decr = (deadline > DECR_MAX_COUNT) ? DECR_MAX_COUNT : deadline;
//      printf(" using decrementer count %u\n", decr);

        /* Reprogramme the desired count */
        rPMGR_INTERVAL_TMR = DECR_MAX_COUNT;
        rPMGR_INTERVAL_TMR_CTL = (1 << 0) | (1 << 8);
        rPMGR_INTERVAL_TMR = decr;
    
    } else {
        rPMGR_INTERVAL_TMR_CTL &= ~(1 << 0);
        rPMGR_INTERVAL_TMR_CTL = (1 << 8);
    }
}

void platform_fiq()
{
    /* clear FIQ and disable decrementer */
    rPMGR_INTERVAL_TMR_CTL &= ~(1 << 0);
    rPMGR_INTERVAL_TMR_CTL = (1 << 8);

    /* if we have a callback, invoke it now */
    if (timer_deadline_func) {
        timer_deadline_func();
    }
}

utime_t timer_ticks_to_usecs(uint64_t ticks)
{
#if (OSC_FREQ % 1000000) == 0
	return ticks / (OSC_FREQ / 1000000);
#else
#error "The code below has overflow issues. Make sure you really want to use it"
	return (ticks * 1000 * 1000) / OSC_FREQ;
#endif
}

uint64_t timer_usecs_to_ticks(utime_t usecs)
{
#if (OSC_FREQ % 1000000) == 0
	return usecs * (OSC_FREQ / 1000000);
#else
#error "The code below has overflow issues. Make sure you really want to use it"
	uint64_t timer_scale = ((uint64_t)(OSC_FREQ) << 20) / (1000 * 1000);
	return (ticks * timer_scale) >> 20;
#endif
}

uint32_t timer_get_tick_rate(void)
{
    return OSC_FREQ;
}

uint64_t timer_get_ticks(void)
{
    extern uint64_t aic_get_ticks(void);

    return (aic_get_ticks());
}
