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
#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/int.h>
#include <sys/boot.h>
#include <target.h>
#include <drivers/aic/aic.h>

extern const struct tunable_chip_struct tunables_pmgr[];
extern const struct tunable_filtered_chip_struct tunables_filtered_pmgr[];

#define PLL_VCO_TARGET(pllx)    ((2ULL * (pllx##_O) * (pllx##_M)) / (pllx##_P))
#define PLL_FREQ_TARGET(pllx)   (((pllx##_O) * (pllx##_M)) / (pllx##_P) / ((pllx##_S) + 1))

static uint32_t perf_level = kPerformanceMemoryLow;

#if APPLICATION_SECUREROM
#define SOC_PERF_STATE_ACTIVE kSOC_PERF_STATE_SECUREROM
#endif

#if APPLICATION_IBOOT
#define SOC_PERF_STATE_ACTIVE kSOC_PERF_STATE_VMIN
#endif

struct clock_source {
    uint32_t    src_clk;
    uint32_t    factor;
};

struct clock_config {
    volatile uint32_t	*clock_reg;     // CLK_CFG Register
    struct clock_source sources[8];    // List of sources
};

struct clock_config_active {
    uint32_t    clk;
    uint32_t    clock_reg_val;
};

struct device_config {
    volatile uint32_t   *ps_reg; // PS Register
};

/* ******************************************************************************** */
#if APPLICATION_IBOOT

/* LPO @192MHz */
#define LPPLL_T     LPPLL_FREQ

/* PLL0 @1065.6MHz */
#define PLL0            0
#define PLL0_P          5
#define PLL0_M          111
#define PLL0_S          0
#define PLL0_T          PLL_FREQ_TARGET(PLL0)
#define PLL0_VCO_OUT    true

/* PLL1 @480MHz */
#define PLL1            1
#define PLL1_P          1
#define PLL1_M          40
#define PLL1_S          1
#define PLL1_T          PLL_FREQ_TARGET(PLL1)
#define PLL1_VCO_OUT    false

static const struct clock_config_active clk_configs_active[] = {
    {PMGR_CLK_AOP,                  0x81100000}, // minipmgr_aop                 on  96MHz
    {PMGR_CLK_UART0,                0x83100000}, // minipmgr_uart0               on  24MHz
    {PMGR_CLK_UART1,                0x83100000}, // minipmgr_uart1               on  24MHz
    {PMGR_CLK_UART2,                0x83100000}, // minipmgr_uart2               on  24MHz
    {PMGR_CLK_SENSE_X2,             0x81100000}, // minipmgr_sense_x2            on  48MHz
    {PMGR_CLK_DETECT,               0x81100000}, // minipmgr_detect              on  96MHz
    {PMGR_CLK_I2CM,                 0x81100000}, // minipmgr_i2cm                on  24MHz
    {PMGR_CLK_I2CM1,                0x81100000}, // minipmgr_i2cm1               on  24MHz
    {PMGR_CLK_PROXY_FABRIC,         0x81100000}, // minipmgr_proxy_fabric        on  96MHz
    {PMGR_CLK_PROXY_MCU_REF,        0x81100000}, // minipmgr_proxy_mcu_ref       on  96MHz
    {PMGR_CLK_GPIO,                 0x80100000}, // gpio                         on  24MHz
    {PMGR_CLK_MCU_REF,              0x83100000}, // mcu_ref                      on  SOC_PERF_STATE
    {PMGR_CLK_CPU,                  0x83100000}, // cpu                          on  SOC_PERF_STATE
    {PMGR_CLK_GFX,                  0x83100000}, // gfx                          on  133.25MHz
    {PMGR_CLK_MEDIA_FAB,            0x83100000}, // media_fab                    on  SOC_PERF_STATE
    {PMGR_CLK_PIO,                  0x83100000}, // pio                          on  SOC_PERF_STATE
    {PMGR_CLK_SDIO,                 0x83100000}, // sdio                         on  SOC_PERF_STATE
    {PMGR_CLK_LIO,                  0x83100000}, // lio                          on  SOC_PERF_STATE
    {PMGR_CLK_AES,                  0x83100000}, // aes                          on  SOC_PERF_STATE
    {PMGR_CLK_AUE,                  0x83100000}, // aue                          on  53.3MHz
    {PMGR_CLK_SPI0_N,               0x83100000}, // spi0_n                       on  53.3MHz
    {PMGR_CLK_SPI1_N,               0x83100000}, // spi1_n                       on  53.3MHz
    {PMGR_CLK_NCO_REF0,             0x83100000}, // nco_ref0                     on  266.5MHz
    {PMGR_CLK_NCO_REF1,             0x83100000}, // nco_ref1                     on  266.5MHz
    {PMGR_CLK_NCO_ALG0,             0x83100000}, // nco_alg0                     on  133.25MHz
    {PMGR_CLK_NCO_ALG1,             0x83100000}, // nco_alg1                     on  133.25MHz
    {PMGR_CLK_MCA0_M,               0x83100000}, // mca0_m                       on  133.25MHz
    {PMGR_CLK_MCA1_M,               0x84100000}, // mca1_m                       on  133.25MHz
    {PMGR_CLK_MCA2_M,               0x84100000}, // mca2_m                       on  133.25MHz
    {PMGR_CLK_TMPS,                 0x83100000}, // tmps                         on  1.2MHz
    {PMGR_CLK_UVD,                  0x83100000}, // uvd                          on  266.5MHz
    {PMGR_CLK_ISP_C,                0x83100000}, // isp_c                        on  480MHz
    {PMGR_CLK_ISP,                  0x83100000}, // isp                          on  160MHz
    {PMGR_CLK_RT_FAB,               0x83100000}, // rt_fab                       on  266.5MHz
    {PMGR_CLK_ISP_SENSOR0,          0x81100000}, // isp_sensor0_ref              on  48MHz
    {PMGR_CLK_VENC,                 0x83100000}, // venc_clk                     on  160MHz
    {PMGR_CLK_SEP,                  0x83100000}, // sep_clk                      on  355.33MHz
    {PMGR_CLK_SEP_FAB,              0x83100000}, // sep_fab_clk                  on  177.7MHz
    {PMGR_CLK_ANS,                  0x83100000}, // ans_c_clk                    on  106.6MHz
    {PMGR_CLK_ANC_LINK,             0x83100000}, // anc_link_clk                 on  106.6MHz
    {PMGR_CLK_VDEC,                 0x83100000}, // vdec_clk                     on  266.5MHz
    {PMGR_CLK_MSR,                  0x83100000}, // msr_clk                      on  133.25MHz
    {PMGR_CLK_AJPEG_IP,             0x83100000}, // ajpeg_ip_clk                 on  106.6MHz
    {PMGR_CLK_AJPEG_WRAP,           0x83100000}, // ajpeg_wrap_clk               on  133.25MHz
    {PMGR_CLK_DISP,                 0x83100000}, // disp_clk                     on  106.6MHz
    {PMGR_CLK_MIPI_DSI,             0x83100000}, // mipi_dsi_clk                 on  533MHz
    {PMGR_CLK_VID,                  0x80100000}, // vid_clk                      on  24MHz
    {PMGR_CLK_HFD,                  0x83100000}, // hfd_clk                      on  106.6MHz
    {PMGR_CLK_HSICPHY_REF_12M,      0x83100000}, // hsicphy_ref_12M_clk          on  12MHz
    {PMGR_CLK_USB480_0,             0x83100000}, // usb480_0_clk                 on  480MHz
    {PMGR_CLK_USB_EHCI,             0x83100000}, // usb_ehci_clk                 on  106.6MHz
};

static const struct clock_config_active spare_clock_configs_active[] = {
    {PMGR_CLK_S0,                   0x00000000}, // s0               off
    {PMGR_CLK_S1,                   0x00000000}, // s1               off
    {PMGR_CLK_ISP_REF0,             0x83000005}, // isp_ref0         on  48MHz
};

#endif /* APPLICATION_IBOOT */

struct pmgr_soc_perf_state {
    uint32_t    entry[1];
};

static struct pmgr_soc_perf_state pmgr_soc_perf_states[] = {
    [kSOC_PERF_STATE_BYPASS] = {{ 0x00000000 }},
    
#if APPLICATION_SECUREROM
    [kSOC_PERF_STATE_SECUREROM] = {
        // PIO              = 106.6 / 2
        // SDIO             = 24
        // MEDIA_FAB        = 24
        // CPU              = 533 / 2
        // AES              = 24
        // LIO              = 24
        // MCU_REF_CFG_SEL  = 0
        // MCU_REF          = 24
        { 0x30030000 }
     },
#endif

#if APPLICATION_IBOOT
    [kSOC_PERF_STATE_VMIN] = {
        // PIO              = 106.6
        // SDIO             = 106.6
        // MEDIA_FAB        = 266.5
        // CPU              = 533
        // AES              = 266.5
        // LIO              = 106.6
        // MCU_REF_CFG_SEL  = 48
        // MCU_REF          = 48
        { 0x33333334 }
    },
#endif
};

/* ******************************************************************************** */
#if APPLICATION_SECUREROM

/* PLL0 @532.8MHz */
#define PLL0            0
#define PLL0_P          5
#define PLL0_M          222
#define PLL0_S          1
#define PLL0_T          PLL_FREQ_TARGET(PLL0)
#define PLL0_VCO_OUT    false

// TODO: Figure out which ones aren't necessary during SecureROM
// We won't touch the clk gen's that aren't necessary during SecureROM.
static const struct clock_config_active clk_configs_active[] = {
    {PMGR_CLK_AOP,                  0x80100000}, // minipmgr_aop                 on  24MHz
    {PMGR_CLK_UART0,                0x80100000}, // minipmgr_uart0               on  24MHz
    {PMGR_CLK_UART1,                0x80100000}, // minipmgr_uart1               on  24MHz
    {PMGR_CLK_UART2,                0x80100000}, // minipmgr_uart2               on  24MHz
    {PMGR_CLK_SENSE_X2,             0x80100000}, // minipmgr_sense_x2            on  24MHz
    {PMGR_CLK_DETECT,               0x80100000}, // minipmgr_detect              on  24MHz
    {PMGR_CLK_I2CM,                 0x80100000}, // minipmgr_i2cm                on  24MHz
    {PMGR_CLK_I2CM1,                0x80100000}, // minipmgr_i2cm1               on  24MHz
    {PMGR_CLK_PROXY_FABRIC,         0x80100000}, // minipmgr_proxy_fabric        on  24MHz
    {PMGR_CLK_PROXY_MCU_REF,        0x80100000}, // minipmgr_proxy_mcu_ref       on  24MHz
    {PMGR_CLK_GPIO,                 0x80100000}, // gpio                         on  24MHz
    {PMGR_CLK_MCU_REF,              0x83100000}, // mcu_ref                      on  SOC_PERF_STATE
    {PMGR_CLK_CPU,                  0x83100000}, // cpu                          on  SOC_PERF_STATE
    {PMGR_CLK_GFX,                  0x80100000}, // gfx                          on  24MHz
    {PMGR_CLK_MEDIA_FAB,            0x80100000}, // media_fab                    on  SOC_PERF_STATE
    {PMGR_CLK_PIO,                  0x83100000}, // pio                          on  SOC_PERF_STATE
    {PMGR_CLK_SDIO,                 0x80100000}, // sdio                         on  SOC_PERF_STATE
    {PMGR_CLK_LIO,                  0x80100000}, // lio                          on  SOC_PERF_STATE
    {PMGR_CLK_AES,                  0x80100000}, // aes                          on  SOC_PERF_STATE
    {PMGR_CLK_AUE,                  0x80100000}, // aue                          on  24MHz
    {PMGR_CLK_SPI0_N,               0x80100000}, // spi0_n                       on  24MHz
    {PMGR_CLK_SPI1_N,               0x80100000}, // spi1_n                       on  24MHz
    {PMGR_CLK_NCO_REF0,             0x80100000}, // nco_ref0                     on  24MHz
    {PMGR_CLK_NCO_REF1,             0x80100000}, // nco_ref1                     on  24MHz
    {PMGR_CLK_NCO_ALG0,             0x80100000}, // nco_alg0                     on  24MHz
    {PMGR_CLK_NCO_ALG1,             0x80100000}, // nco_alg1                     on  24MHz
    {PMGR_CLK_MCA0_M,               0x80100000}, // mca0_m                       on  24MHz
    {PMGR_CLK_MCA1_M,               0x80100000}, // mca1_m                       on  24MHz
    {PMGR_CLK_MCA2_M,               0x80100000}, // mca2_m                       on  24MHz
    {PMGR_CLK_TMPS,                 0x83100000}, // tmps                         on  1.2MHz
    {PMGR_CLK_UVD,                  0x80100000}, // uvd                          on  24MHz
    {PMGR_CLK_ISP_C,                0x80100000}, // isp_c                        on  24MHz
    {PMGR_CLK_ISP,                  0x80100000}, // isp                          on  24MHz
    {PMGR_CLK_RT_FAB,               0x80100000}, // rt_fab                       on  24MHz
    {PMGR_CLK_ISP_SENSOR0,          0x80100000}, // isp_sensor0_ref              on  24MHz
    {PMGR_CLK_VENC,                 0x80100000}, // venc_clk                     on  24MHz
    {PMGR_CLK_SEP,                  0x83100000}, // sep_clk                      on  177.67MHz
    {PMGR_CLK_SEP_FAB,              0x83100000}, // sep_fab_clk                  on  88.85MHz
    {PMGR_CLK_ANS,                  0x83100000}, // ans_c_clk                    on  53.3MHz
    {PMGR_CLK_ANC_LINK,             0x83100000}, // anc_link_clk                 on  53.3MHz
    {PMGR_CLK_VDEC,                 0x80100000}, // vdec_clk                     on  24MHz
    {PMGR_CLK_MSR,                  0x80100000}, // msr_clk                      on  24MHz
    {PMGR_CLK_AJPEG_IP,             0x80100000}, // ajpeg_ip_clk                 on  24MHz
    {PMGR_CLK_AJPEG_WRAP,           0x80100000}, // ajpeg_wrap_clk               on  24MHz
    {PMGR_CLK_DISP,                 0x80100000}, // disp_clk                     on  24MHz
    {PMGR_CLK_MIPI_DSI,             0x80100000}, // mipi_dsi_clk                 on  24MHz
    {PMGR_CLK_VID,                  0x80100000}, // vid_clk                      on  24MHz
    {PMGR_CLK_HFD,                  0x80100000}, // hfd_clk                      on  24MHz
    {PMGR_CLK_HSICPHY_REF_12M,      0x80100000}, // hsicphy_ref_12M_clk          on  24MHz
    {PMGR_CLK_USB480_0,             0x80100000}, // usb480_0_clk                 on  24MHz
    {PMGR_CLK_USB_EHCI,             0x80100000}, // usb_ehci_clk                 on  24MHz
};

static const struct clock_config_active spare_clock_configs_active[] = {
    {PMGR_CLK_S0,                   0x80000001}, // s0               on  266.4MHz
    {PMGR_CLK_S1,                   0x80000001}, // s1               on  266.4MHz
    {PMGR_CLK_ISP_REF0,             0x81000006}, // isp_ref0         on  22.2MHz    // Should be less than 24 MHz
};

#endif /* APPLICATION_SECUREROM */
/* ******************************************************************************** */
#define CLOCK_SOURCES_MAX   8

static const struct clock_config clk_configs[PMGR_CLK_COUNT] = {
    [PMGR_CLK_GPIO] = { 
        &rPMGR_CLK_CFG(GPIO), 
        { 
            {PMGR_CLK_OSC, 1}
        }
    },
    [PMGR_CLK_MCU_REF] = { 
        &rPMGR_CLK_CFG(MCU_REF),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL1, 10},
            {PMGR_CLK_PROXY_MCU_REF, 1},
        }
    },
    [PMGR_CLK_CPU] = { 
        &rPMGR_CLK_CFG(CPU),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
        }
    },
    [PMGR_CLK_GFX] = { 
        &rPMGR_CLK_CFG(GFX),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
        }
    },
    [PMGR_CLK_MEDIA_FAB] = { 
        &rPMGR_CLK_CFG(MEDIA_FAB),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL1, 3},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
        }
    },
    [PMGR_CLK_PIO] = { 
        &rPMGR_CLK_CFG(PIO),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PROXY_FABRIC, 1},
            {PMGR_CLK_PLL1, 5},
        }
    },
    [PMGR_CLK_SDIO] = { 
        &rPMGR_CLK_CFG(SDIO),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_LIO] = { 
        &rPMGR_CLK_CFG(LIO),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 20},
            {PMGR_CLK_OSC, 4},
        }
    },
    [PMGR_CLK_AES] = { 
        &rPMGR_CLK_CFG(AES),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
        }
    },
    [PMGR_CLK_AUE] = { 
        &rPMGR_CLK_CFG(AUE),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 20},
            {PMGR_CLK_PLL1, 8},
        }
    },
    [PMGR_CLK_SPI0_N] = { 
        &rPMGR_CLK_CFG(SPI0_N),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 20},
            {PMGR_CLK_PLL0, 40},
        }
    },
    [PMGR_CLK_SPI1_N] = { 
        &rPMGR_CLK_CFG(SPI1_N),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 20},
            {PMGR_CLK_PLL0, 40},
        }
    },
    [PMGR_CLK_NCO_REF0] = { 
        &rPMGR_CLK_CFG(NCO_REF0),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 8},
        }
    },
    [PMGR_CLK_NCO_REF1] = { 
        &rPMGR_CLK_CFG(NCO_REF1),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 8},
        }
    },
    [PMGR_CLK_NCO_ALG0] = { 
        &rPMGR_CLK_CFG(NCO_ALG0),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_NCO_ALG1] = { 
        &rPMGR_CLK_CFG(NCO_ALG1),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_MCA0_M] = { 
        &rPMGR_CLK_CFG(MCA0_M),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_NCO_REF0, 1},
            {PMGR_CLK_NCO_REF1, 1},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 4},
        }
    },
    [PMGR_CLK_MCA1_M] = { 
        &rPMGR_CLK_CFG(MCA1_M),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_NCO_REF0, 1},
            {PMGR_CLK_NCO_REF1, 1},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 4},
        }
    },
    [PMGR_CLK_MCA2_M] = { 
        &rPMGR_CLK_CFG(MCA2_M),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_NCO_REF0, 1},
            {PMGR_CLK_NCO_REF1, 1},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 4},
        }
    },
    [PMGR_CLK_TMPS] = { 
        &rPMGR_CLK_CFG(TMPS),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_OSC, 20},
        }
    },
    [PMGR_CLK_UVD] = { 
        &rPMGR_CLK_CFG(UVD),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
        }
    },
    [PMGR_CLK_ISP_C] = { 
        &rPMGR_CLK_CFG(ISP_C),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL1, 2},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL1, 4},
        }
    },
    [PMGR_CLK_ISP] = { 
        &rPMGR_CLK_CFG(ISP),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL1, 3},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 16},
        }
    },
    [PMGR_CLK_RT_FAB] = { 
        &rPMGR_CLK_CFG(RT_FAB),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL1, 3},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 16},
        }
    },
    [PMGR_CLK_ISP_SENSOR0] = { 
        &rPMGR_CLK_CFG(ISP_SENSOR0_REF),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_ISP_REF0, 1},
        }
    },
    [PMGR_CLK_VENC] = { 
        &rPMGR_CLK_CFG(VENC),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL1, 3},
            {PMGR_CLK_PLL0, 8},
        }
    },
    [PMGR_CLK_SEP] = { 
        &rPMGR_CLK_CFG(SEP),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 3},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
        }
    },
    [PMGR_CLK_SEP_FAB] = { 
        &rPMGR_CLK_CFG(SEP_FAB),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 6},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
        }
    },
    [PMGR_CLK_ANS] = { 
        &rPMGR_CLK_CFG(ANS_C),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_ANC_LINK] = { 
        &rPMGR_CLK_CFG(ANC_LINK),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_VDEC] = { 
        &rPMGR_CLK_CFG(VDEC),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
        }
    },
    [PMGR_CLK_MSR] = { 
        &rPMGR_CLK_CFG(MSR),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_AJPEG_IP] = { 
        &rPMGR_CLK_CFG(AJPEG_IP),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_AJPEG_WRAP] = { 
        &rPMGR_CLK_CFG(AJPEG_WRAP),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 8},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_DISP] = { 
        &rPMGR_CLK_CFG(DISP),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
            {PMGR_CLK_PLL0, 16},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_MIPI_DSI] = { 
        &rPMGR_CLK_CFG(MIPI_DSI),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 12},
        }
    },
    [PMGR_CLK_VID] = { 
        &rPMGR_CLK_CFG(VID),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 48},
            {PMGR_CLK_OSC, 2},
            {PMGR_CLK_OSC, 3},
            {PMGR_CLK_OSC, 4},
        }
    },
    [PMGR_CLK_HFD] = { 
        &rPMGR_CLK_CFG(HFD),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 20},
            {PMGR_CLK_PLL0, 32},
            {PMGR_CLK_OSC, 1},
        }
    },
    [PMGR_CLK_HSICPHY_REF_12M] = { 
        &rPMGR_CLK_CFG(HSICPHY_REF_12M),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_OSC, 2},
        }
    },
    [PMGR_CLK_USB_EHCI] = { 
        &rPMGR_CLK_CFG(USB_EHCI),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL0, 10},
            {PMGR_CLK_PLL0, 20},
        }
    },
    [PMGR_CLK_USB480_0] = { 
        &rPMGR_CLK_CFG(USB480_0),
        {
            {PMGR_CLK_OSC, 1},
            {PMGR_CLK_S0, 1},
            {PMGR_CLK_S1, 1},
            {PMGR_CLK_PLL1, 1},
        }
    },
    [PMGR_CLK_S0] = { 
        &rPMGR_CLK_CFG(S0),
        {
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL1, 1},
            {PMGR_CLK_PLL1, 2},
        }
    },
    [PMGR_CLK_S1] = { 
        &rPMGR_CLK_CFG(S1),
        {
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL1, 1},
            {PMGR_CLK_PLL1, 2},
        }
    },
    [PMGR_CLK_ISP_REF0] = { 
        &rPMGR_CLK_CFG(ISP_REF0),
        {
            {PMGR_CLK_PLL0, 2},
            {PMGR_CLK_PLL0, 4},
            {PMGR_CLK_PLL1, 1},
            {PMGR_CLK_PLL1, 2},
        }
    },   
    [PMGR_CLK_AOP] = { 
        &rMINIPMGR_CLK_CFG(AOP),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },   
    [PMGR_CLK_UART0]        =   {
        &rMINIPMGR_CLK_CFG(UART0),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },
    [PMGR_CLK_UART1]        =   {
        &rMINIPMGR_CLK_CFG(UART1),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },
    [PMGR_CLK_UART2]        =   {
        &rMINIPMGR_CLK_CFG(UART2),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },
    [PMGR_CLK_SENSE_X2]        =   {
        &rMINIPMGR_CLK_CFG(SENSE_X2),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
            { PMGR_CLK_LPPLL, 7 },
            { PMGR_CLK_LPPLL, 14 },
        }
    },
    [PMGR_CLK_DETECT]        =   {
        &rMINIPMGR_CLK_CFG(DETECT),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },
    [PMGR_CLK_I2CM]        =   {
        &rMINIPMGR_CLK_CFG(I2CM),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 4 },
            { PMGR_CLK_LPPLL, 8 },
        }
    },
    [PMGR_CLK_I2CM1]        =   {
        &rMINIPMGR_CLK_CFG(I2CM1),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 4 },
            { PMGR_CLK_LPPLL, 8 },
        }
    },
    [PMGR_CLK_PROXY_FABRIC]        =   {
        &rMINIPMGR_CLK_CFG(PROXY_FABRIC),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },
    [PMGR_CLK_PROXY_MCU_REF]        =   {
        &rMINIPMGR_CLK_CFG(PROXY_MCU_REF),
        {
            { PMGR_CLK_OSC, 1 },
            { PMGR_CLK_LPPLL, 1 },
            { PMGR_CLK_LPPLL, 2 },
            { PMGR_CLK_LPPLL, 4 },
        }
    },
};

static const struct device_config device_configs[PMGR_DEVICE_COUNT] = {
    // Mini PMGR
    [PMGR_DEVICE_INDEX(CLK_AOP)]                = {&rMINIPMGR_PS(AOP)},
    [PMGR_DEVICE_INDEX(CLK_DEBUG)]              = {&rMINIPMGR_PS(DEBUG)},
    [PMGR_DEVICE_INDEX(CLK_AOP_GPIO)]           = {&rMINIPMGR_PS(AOP_GPIO)},
    [PMGR_DEVICE_INDEX(CLK_AOP_I2CM1)]          = {&rMINIPMGR_PS(AOP_I2CM1)},
    [PMGR_DEVICE_INDEX(CLK_AOP_CPU)]            = {&rMINIPMGR_PS(AOP_CPU)},
    [PMGR_DEVICE_INDEX(CLK_AOP_RTCPU)]          = {&rMINIPMGR_PS(AOP_RTCPU)},
    [PMGR_DEVICE_INDEX(CLK_AOP_FILTER)]         = {&rMINIPMGR_PS(AOP_FILTER)},
    [PMGR_DEVICE_INDEX(CLK_AOP_UART0)]          = {&rMINIPMGR_PS(AOP_UART0)},
    [PMGR_DEVICE_INDEX(CLK_AOP_UART1)]          = {&rMINIPMGR_PS(AOP_UART1)},
    [PMGR_DEVICE_INDEX(CLK_AOP_UART2)]          = {&rMINIPMGR_PS(AOP_UART2)},
    [PMGR_DEVICE_INDEX(CLK_AOP_I2CM)]           = {&rMINIPMGR_PS(AOP_I2CM)},
    [PMGR_DEVICE_INDEX(CLK_AOP_FILTER_DMA)]     = {&rMINIPMGR_PS(AOP_FILTER_DMA)},
    [PMGR_DEVICE_INDEX(CLK_AOP_LPD0)]           = {&rMINIPMGR_PS(AOP_LPD0)},
    [PMGR_DEVICE_INDEX(CLK_AOP_HPDS)]           = {&rMINIPMGR_PS(AOP_HPDS)},
    [PMGR_DEVICE_INDEX(CLK_AOP_HPDSC)]          = {&rMINIPMGR_PS(AOP_HPDSC)},
    [PMGR_DEVICE_INDEX(CLK_AOP_HPDD)]           = {&rMINIPMGR_PS(AOP_HPDD)},
    [PMGR_DEVICE_INDEX(CLK_MARCONI)]            = {&rMINIPMGR_PS(MARCONI)},

    // PMGR
    [PMGR_DEVICE_INDEX(CLK_SCU)]                = {&rPMGR_PS(SCU)},
    [PMGR_DEVICE_INDEX(CLK_CPU0)]               = {&rPMGR_PS(CPU0)},
    [PMGR_DEVICE_INDEX(CLK_CPU1)]               = {&rPMGR_PS(CPU1)},
    [PMGR_DEVICE_INDEX(CLK_PIO)]                = {&rPMGR_PS(PIO)},
    [PMGR_DEVICE_INDEX(CLK_CPU_FAB)]            = {&rPMGR_PS(CPU_FAB)},
    [PMGR_DEVICE_INDEX(CLK_NRT_FAB)]            = {&rPMGR_PS(NRT_FAB)},
    [PMGR_DEVICE_INDEX(CLK_RT_FAB)]             = {&rPMGR_PS(RT_FAB)},
    [PMGR_DEVICE_INDEX(CLK_AIC)]                = {&rPMGR_PS(AIC)},
    [PMGR_DEVICE_INDEX(CLK_GPIO)]               = {&rPMGR_PS(GPIO)},
    [PMGR_DEVICE_INDEX(CLK_ISPSENS0)]           = {&rPMGR_PS(ISPSENS0)},
    [PMGR_DEVICE_INDEX(CLK_UVD)]                = {&rPMGR_PS(UVD)},
    [PMGR_DEVICE_INDEX(CLK_HSIC0PHY)]           = {&rPMGR_PS(HSIC0PHY)},
    [PMGR_DEVICE_INDEX(CLK_AMC)]                = {&rPMGR_PS(AMC)},
    [PMGR_DEVICE_INDEX(CLK_LIO_FAB)]            = {&rPMGR_PS(LIO_FAB)},
    [PMGR_DEVICE_INDEX(CLK_LIO_LOGIC)]          = {&rPMGR_PS(LIO_LOGIC)},
    [PMGR_DEVICE_INDEX(CLK_LIO)]                = {&rPMGR_PS(LIO)},
    [PMGR_DEVICE_INDEX(CLK_AES0)]               = {&rPMGR_PS(AES)},
    [PMGR_DEVICE_INDEX(CLK_MCA0)]               = {&rPMGR_PS(MCA0)},
    [PMGR_DEVICE_INDEX(CLK_MCA1)]               = {&rPMGR_PS(MCA1)},
    [PMGR_DEVICE_INDEX(CLK_MCA2)]               = {&rPMGR_PS(MCA2)},
    [PMGR_DEVICE_INDEX(CLK_HFD)]                = {&rPMGR_PS(HFD)},
    [PMGR_DEVICE_INDEX(CLK_SPI0)]               = {&rPMGR_PS(SPI0)},
    [PMGR_DEVICE_INDEX(CLK_SPI1)]               = {&rPMGR_PS(SPI1)},
    [PMGR_DEVICE_INDEX(CLK_UART0)]              = {&rPMGR_PS(UART0)},
    [PMGR_DEVICE_INDEX(CLK_UART1)]              = {&rPMGR_PS(UART1)},
    [PMGR_DEVICE_INDEX(CLK_UART2)]              = {&rPMGR_PS(UART2)},
    [PMGR_DEVICE_INDEX(CLK_UART3)]              = {&rPMGR_PS(UART3)},
    [PMGR_DEVICE_INDEX(CLK_UART4)]              = {&rPMGR_PS(UART4)},
    [PMGR_DEVICE_INDEX(CLK_UART5)]              = {&rPMGR_PS(UART5)},
    [PMGR_DEVICE_INDEX(CLK_UART6)]              = {&rPMGR_PS(UART6)},
    [PMGR_DEVICE_INDEX(CLK_UART7)]              = {&rPMGR_PS(UART7)},
    [PMGR_DEVICE_INDEX(CLK_I2C0)]               = {&rPMGR_PS(I2C0)},
    [PMGR_DEVICE_INDEX(CLK_I2C1)]               = {&rPMGR_PS(I2C1)},
    [PMGR_DEVICE_INDEX(CLK_I2C2)]               = {&rPMGR_PS(I2C2)},
    [PMGR_DEVICE_INDEX(CLK_PWM0)]               = {&rPMGR_PS(PWM0)},
    [PMGR_DEVICE_INDEX(CLK_USB)]                = {&rPMGR_PS(USB)},
    [PMGR_DEVICE_INDEX(CLK_ETH)]                = {&rPMGR_PS(ETH)},
    [PMGR_DEVICE_INDEX(CLK_ANS)]                = {&rPMGR_PS(ANS)},
    [PMGR_DEVICE_INDEX(CLK_SDIO)]               = {&rPMGR_PS(SDIO)},
    [PMGR_DEVICE_INDEX(CLK_DISP)]               = {&rPMGR_PS(DISP)},
    [PMGR_DEVICE_INDEX(CLK_MIPI_DSI)]           = {&rPMGR_PS(MIPI_DSI)},
    [PMGR_DEVICE_INDEX(CLK_ISP)]                = {&rPMGR_PS(ISP)},
    [PMGR_DEVICE_INDEX(CLK_GFX)]                = {&rPMGR_PS(GFX)},
    [PMGR_DEVICE_INDEX(CLK_MEDIA_FAB)]          = {&rPMGR_PS(MEDIA_FAB)},
    [PMGR_DEVICE_INDEX(CLK_MSR)]                = {&rPMGR_PS(MSR)},
    [PMGR_DEVICE_INDEX(CLK_VDEC)]               = {&rPMGR_PS(VDEC)},
    [PMGR_DEVICE_INDEX(CLK_JPG)]                = {&rPMGR_PS(JPG)},
    [PMGR_DEVICE_INDEX(CLK_VENC_CPU)]           = {&rPMGR_PS(VENC_CPU)},
    [PMGR_DEVICE_INDEX(CLK_USB2HOST1)]          = {&rPMGR_PS(USB2HOST1)},
    [PMGR_DEVICE_INDEX(CLK_SEP)]                = {&rPMGR_PS(SEP)},
    [PMGR_DEVICE_INDEX(CLK_VENC_PIPE)]          = {&rPMGR_PS(VENC_PIPE)},
    [PMGR_DEVICE_INDEX(CLK_VENC_ME)]            = {&rPMGR_PS(VENC_ME)},
};

/* ******************************************************************************** */

static void set_pll(uint32_t pll, uint32_t p, uint32_t m, uint32_t s, bool vco_out);
static uint32_t get_pll_frequency(uint32_t pll);
static uint32_t get_spare_frequency(uint32_t cnt);

static void clocks_get_frequencies(void);
static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk);
static void clocks_quiesce_internal(void);
static void apply_tunables(void);
static void power_on_sep(void);

static void update_memory_clk_config(uint32_t performance_level);

static void config_soc_perf_state(uint32_t state);
static void set_soc_perf_state(uint32_t target_state);
static void set_lppll(void);

static void clock_update_range(uint32_t first, uint32_t last, const uint32_t clkdata);

// current clock frequencies
static uint32_t clks[PMGR_CLK_COUNT + 1];

/* ******************************************************************************** */

static void set_pll(uint32_t pll, uint32_t p, uint32_t m, uint32_t s, bool vco_out)
{
    if (pll >= PMGR_PLL_COUNT)
        panic("Invalid PLL %u", pll);

    rPMGR_PLL_CFG(pll) |= PMGR_PLL_CFG_VCO_OUT_SEL_INSRT(vco_out ? 1 : 0);
    rPMGR_PLL_CTL(pll) = (PMGR_PLL_CTL_PRE_DIVN_INSRT(p) | PMGR_PLL_CTL_FB_DIVN_INSRT(m) | PMGR_PLL_CTL_OP_DIVN_INSRT(s) | PMGR_PLL_CTL_LOAD_INSRT(1));
    while (PMGR_PLL_CTL_PENDING_XTRCT(rPMGR_PLL_CTL(pll)));

    rPMGR_PLL_CTL(pll) |= PMGR_PLL_CTL_ENABLE_INSRT(1);
    while (PMGR_PLL_CTL_PENDING_XTRCT(rPMGR_PLL_CTL(pll)));
}

static uint32_t get_pll_frequency(uint32_t pll)
{
    uint32_t pllctl, pllcfg;
    uint64_t freq = OSC_FREQ;

    if (pll >= PMGR_PLL_COUNT)
        panic("Invalid PLL %u", pll);

    pllctl = rPMGR_PLL_CTL(pll);
    pllcfg = rPMGR_PLL_CFG(pll);

    if (PMGR_PLL_CTL_ENABLE_XTRCT(pllctl) == 0) {
        return 0;
    }
    else if (PMGR_PLL_CTL_BYPASS_XTRCT(pllctl)) {
        return freq;
    }

    if (PMGR_PLL_CFG_VCO_OUT_SEL_XTRCT(pllcfg)) {
        // F_out = F_vco = ((24 / P) * M) * 2
        freq /= PMGR_PLL_CTL_PRE_DIVN_XTRCT(pllctl);
        freq *= PMGR_PLL_CTL_FB_DIVN_XTRCT(pllctl);
        freq *= 2;
    }
    else {
        // F_out = ((24 / P) * M) / (S + 1)
        freq /= PMGR_PLL_CTL_PRE_DIVN_XTRCT(pllctl);
        freq *= PMGR_PLL_CTL_FB_DIVN_XTRCT(pllctl);
        freq /= (1 + PMGR_PLL_CTL_OP_DIVN_XTRCT(pllctl));
    }

#if DEBUG_BUILD
    if (freq > 0xFFFFFFFF)
        panic("Frequency value does not fit in uint32_t");
#endif

    return (uint32_t)freq;
}

static uint32_t get_spare_frequency(uint32_t spare)
{
    uint32_t reg_val, src_idx, src_clk, src_factor, div;
    volatile uint32_t *spare_clkcfg = clk_configs[PMGR_CLK_S0 + spare].clock_reg;

    reg_val = *spare_clkcfg;

    div = PMGR_CLKCFG_DIVISOR_XTRCT(reg_val);

    if ((PMGR_CLKCFG_ENABLE_XTRCT(reg_val) == 0) || div == 0)
          return 0;

    src_idx = PMGR_CLKCFG_SRC_SEL_XTRCT(reg_val);
    src_clk = clk_configs[PMGR_CLK_S0 + spare].sources[src_idx].src_clk;
    src_factor = clk_configs[PMGR_CLK_S0 + spare].sources[src_idx].factor;

    return (clks[src_clk] / src_factor) / div;
}

static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk)
{
    volatile uint32_t *reg;
    uint32_t cnt, val, src_idx, src_clk, src_factor;

    if ((start_clk >= PMGR_CLK_SOURCE_FIRST && end_clk <= PMGR_CLK_SOURCE_LAST) ||
        (start_clk >= PMGR_CLK_MINI_FIRST && end_clk <= PMGR_CLK_MINI_LAST)) {
        for (cnt = start_clk; cnt <= end_clk; cnt++) {
            reg = clk_configs[cnt].clock_reg;
            val = *reg;

            if (PMGR_CLKCFG_ENABLE_XTRCT(val) == 0) {
                clks[cnt] = 0;
                continue;
            }

            src_idx = PMGR_CLKCFG_SRC_SEL_XTRCT(val);
            src_clk = clk_configs[cnt].sources[src_idx].src_clk;
            src_factor = clk_configs[cnt].sources[src_idx].factor;
            clks[cnt] = clks[src_clk] / src_factor;
        }
    }
}

static void clocks_get_frequencies(void)
{
#if SUPPORT_FPGA
    uint32_t cnt;
    uint32_t freq = OSC_FREQ;

    for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
        clks[cnt] = freq;
    
    clks[PMGR_CLK_MCU_REF] = 10000000;
    //clks[PMGR_CLK_MCU_FIXED]= 10000000;
    //clks[PMGR_CLK_USB]	= 12000000;

#elif CONFIG_SIM
    uint32_t cnt;
    uint32_t freq = OSC_FREQ;

    for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
        clks[cnt] = freq;

#else
    uint32_t cnt;

    clks[PMGR_CLK_OSC] = OSC_FREQ;

    // Use get_pll_frequency() to establish the frequency (unconfigured PLLs will bypass OSC)
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++)
        clks[PMGR_CLK_PLL0 + cnt] = get_pll_frequency(cnt);

    // Use get_spare_frequencies() to establish the frequencies for spare clocks (unconfigured will be skipped)
    for (cnt = 0; cnt < sizeof(spare_clock_configs_active) / sizeof(spare_clock_configs_active[0]); cnt++)
        clks[PMGR_CLK_S0 + cnt] = get_spare_frequency(cnt);

    clocks_get_frequencies_range(PMGR_CLK_MINI_FIRST, PMGR_CLK_MINI_LAST);
    clocks_get_frequencies_range(PMGR_CLK_SOURCE_FIRST, PMGR_CLK_SOURCE_LAST);
#endif
}

int clocks_init(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS))
    clocks_get_frequencies();
#endif

    return 0;
}

void clock_gate(int device, bool enable)
{
    volatile uint32_t *reg;
    
    // Make sure we are within limits
    if (!PMGR_VALID_DEVICE(device))
        return;

    reg = device_configs[PMGR_DEVICE_INDEX(device)].ps_reg;

    // Set the PS field to the requested level
    if (enable) {
        *reg |= PMGR_PS_MANUAL_PS_INSRT(PMGR_PS_RUN_MAX);
    } else {
        *reg &= ~PMGR_PS_MANUAL_PS_INSRT(PMGR_PS_RUN_MAX);
    }

    // Wait for the MANUAL_PS and ACTUAL_PS fields to be equal
    while (PMGR_PS_MANUAL_PS_XTRCT(*reg) != PMGR_PS_ACTUAL_PS_XTRCT(*reg));
}

static void restore_clock_config_reset_state(void)
{
    uint32_t current_select, entry_a;
    uint32_t cnt;

    // 1. Restore PWR_*_CFG* registers to their reset defaults
    rPMGR_CLK_POWER_CONFIG = 0;

    // 2. Write reset value to ACG, CLK_DIVIDER_ACG_CFG, CLK_DIVIDER_ACG_CFG1, and PLL_ACG_CFG
    rPMGR_MISC_CFG_ACG = 0;
    rPMGR_CLK_DIVIDER_ACG_CFG = 0;

    // 5a. Write reset value for all mux clock configs (excluding spares)
    clock_update_range(PMGR_CLK_SOURCE_FIRST, PMGR_CLK_TMPS - 1, 0x80100000);
    clock_update_range(PMGR_CLK_TMPS, PMGR_CLK_TMPS, 0x83100000);
    clock_update_range(PMGR_CLK_TMPS + 1, PMGR_CLK_SOURCE_LAST, 0x80100000);

    // 5b. Write the desired DRAM clock configuration state into the SOC_PERF_STATE ENTRY_0A register, MCU_REF_CFG_SEL,
    // MCU_REF_SRC_SEL fields; Write the reset value to all other fields in ENTRY_0
    current_select = PMGR_SOC_PERF_STATE_CTL_CURRENT_SELECT_XTRCT(rPMGR_SOC_PERF_STATE_CTL);
    entry_a = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[0];
    entry_a &= ~PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;
    entry_a |= rPMGR_SOC_PERF_STATE_ENTRY_A(current_select) & PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;

    rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS)) = entry_a;

    // 6. Write the reset value to the SOC_PERF_STATE_CTL register
    rPMGR_SOC_PERF_STATE_CTL = 0;
    while (PMGR_SOC_PERF_STATE_CTL_PENDING_XTRCT(rPMGR_SOC_PERF_STATE_CTL));

    // 7. Write the reset values to the SOC_PERF_STATE entry registers, except for BYPASS ENTRY_A
    for (cnt = PMGR_SOC_PERF_STATE_FIRST_ENTRY; cnt < PMGR_SOC_PERF_STATE_ENTRY_COUNT; cnt++) {
        if (cnt == PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS))
            continue;

        rPMGR_SOC_PERF_STATE_ENTRY_A(cnt) = 0;
    }

    // 11. Write reset value to all PLLx_CTL, except for PLL0
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
        if (cnt == 0)
            continue;

        rPMGR_PLL_CTL(cnt) = PMGR_PLL_CTL_ENABLE_INSRT(1) | PMGR_PLL_CTL_BYPASS_INSRT(1) | PMGR_PLL_CTL_FB_DIVN_INSRT(1) | PMGR_PLL_CTL_PRE_DIVN_INSRT(1);

        while (PMGR_PLL_CTL_PENDING_XTRCT(rPMGR_PLL_CTL(cnt)));
    }

    // 12. Write reset value to all other PLL registers, except for PLL0
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
        if (cnt == 0)
            continue;

        rPMGR_PLL_CFG(cnt) = PMGR_PLL_CFG_OFF_MODE_INSRT(PMGR_PLL_OFF_MODE_POWER_DOWN) |
            PMGR_PLL_CFG_FRAC_LOCK_TIME_INSRT(0x48) | PMGR_PLL_CFG_LOCK_TIME_INSRT(0x348);
    }

    // 13. Write reset value to spare and ISP_REF0
    clock_update_range(PMGR_CLK_SPARE_FIRST, PMGR_CLK_SPARE_LAST, 0x80000001);

    // Mini-PMGR

    // 2. Write the reset value to the CLK_DIVIDER_ACG_CFG register
    rMINIPMGR_CLK_DIVIDER_ACG_CFG = 0;

    // 3. Write the reset value to all mux clock config registers
    clock_update_range(PMGR_CLK_MINI_FIRST, PMGR_CLK_MINI_LAST, 0x80100000);

    // 4. Write the reset value to LPPLL_CTL register
    rMINIPMGR_LPPLL_CTL = MINIPMGR_LPPLL_CTL_ENABLE_INSRT(1) | MINIPMGR_LPPLL_CTL_BYPASS_INSRT(1);
    while (MINIPMGR_LPPLL_CTL_PENDING_XTRCT(rMINIPMGR_LPPLL_CTL));

    // 5. Write the reset value to LPPLL_CFG register
    rMINIPMGR_LPPLL_CFG = 0x00000010;

    // 6. Write the reset value to the MISC_CFG_ACG register
    rMINIPMGR_MISC_CFG_ACG = 0;
}

static void clocks_quiesce_internal(void)
{
#if APPLICATION_SECUREROM
    // Workaround for <rdar://problem/19158202> M8: Set ASYNC_XOVER_ACG.GLOBAL_ENABLE at boot before other PS registers
    rPMGR_MISC_CFG_ASYNC_XOVER_ACG = PMGR_MISC_CFG_ASYNC_XOVER_ACG_GLOBAL_ENABLE_INSRT(1);
#endif

    // Turn on these domains
    clock_gate(CLK_AOP, 1);
    clock_gate(CLK_DEBUG, 1);
    clock_gate(CLK_AOP_GPIO, 1);
    clock_gate(CLK_AOP_CPU, 1);
    clock_gate(CLK_AOP_FILTER, 1);
    clock_gate(CLK_AOP_FILTER_DMA, 1);		// PL080 DMA to zero out memory - rdar://problem/19986351

    clock_gate(CLK_SCU, 1);
    clock_gate(CLK_CPU0, 1);
    clock_gate(CLK_PIO, 1);
    clock_gate(CLK_CPU_FAB, 1);
    clock_gate(CLK_NRT_FAB, 1);

    clock_gate(CLK_AIC, 1);
    clock_gate(CLK_AMC, 1);
    clock_gate(CLK_LIO_FAB, 1);
    clock_gate(CLK_LIO_LOGIC, 1);
    clock_gate(CLK_LIO, 1);
    clock_gate(CLK_GPIO, 1);
    clock_gate(CLK_AES0, 1);
    clock_gate(CLK_USB, 1);
    clock_gate(CLK_ETH, 1);
    clock_gate(CLK_ANS, 1);

    // Turn off these domains
    clock_gate(CLK_AOP_I2CM1, 0);
    clock_gate(CLK_AOP_RTCPU, 0);
    clock_gate(CLK_AOP_UART0, 0);
    clock_gate(CLK_AOP_UART1, 0);
    clock_gate(CLK_AOP_UART2, 0);
    clock_gate(CLK_AOP_I2CM, 0);
    clock_gate(CLK_AOP_LPD0, 0);
    clock_gate(CLK_AOP_HPDS, 0);
    clock_gate(CLK_AOP_HPDSC, 0);
    clock_gate(CLK_AOP_HPDD, 0);

    clock_gate(CLK_CPU1, 0);
    clock_gate(CLK_RT_FAB, 0);
    clock_gate(CLK_ISPSENS0, 0);
    clock_gate(CLK_UVD, 0);
    clock_gate(CLK_HSIC0PHY, 0);
    clock_gate(CLK_MCA0, 0);
    clock_gate(CLK_MCA1, 0);
    clock_gate(CLK_MCA2, 0);
    clock_gate(CLK_HFD, 0);
    clock_gate(CLK_SPI0, 0);
    clock_gate(CLK_SPI1, 0);
    clock_gate(CLK_UART0, 0);
    clock_gate(CLK_UART1, 0);
    clock_gate(CLK_UART2, 0);
    clock_gate(CLK_UART3, 0);
    clock_gate(CLK_UART4, 0);
    clock_gate(CLK_UART5, 0);
    clock_gate(CLK_UART6, 0);
    clock_gate(CLK_UART7, 0);
    clock_gate(CLK_I2C0, 0);
    clock_gate(CLK_I2C1, 0);
    clock_gate(CLK_I2C2, 0);
    clock_gate(CLK_PWM0, 0);
    clock_gate(CLK_SDIO, 0);
    clock_gate(CLK_DISP, 0);
    clock_gate(CLK_MIPI_DSI, 0);
    clock_gate(CLK_ISP, 0);
    clock_gate(CLK_GFX, 0);
    clock_gate(CLK_MEDIA_FAB, 0);
    clock_gate(CLK_MSR, 0);
    clock_gate(CLK_VDEC, 0);
    clock_gate(CLK_JPG, 0);
    clock_gate(CLK_VENC_CPU, 0);
    clock_gate(CLK_USB2HOST1, 0);

    restore_clock_config_reset_state();
}

static void enable_bira_work_around(void)
{
    // Workaround for <rdar://problem/20253847> M8: DFT logic isn't reset when a partition powers up while another one is repaired
    // Note: The reconfig part of this workaround is not done by pmgr driver.
    static const uint32_t domain[] = {
        CLK_SDIO,
        CLK_DISP,
        CLK_MIPI_DSI,
        CLK_ISP,
        CLK_GFX,
        CLK_MSR,
        CLK_VDEC,
        CLK_JPG,
        CLK_VENC_CPU,
        CLK_VENC_PIPE,
        CLK_VENC_ME,
    };

    // Turn on the blocks
    for (uint32_t i = 0; i < sizeof(domain)/sizeof(domain[0]); i++) {
        clock_gate(domain[i], true);
    }

    // Turn off the blocks
    for (uint32_t i = sizeof(domain)/sizeof(domain[0]); i > 0; i--) {
        clock_gate(domain[i - 1], false);
    }
}

static void apply_tunables(void)
{
    uint32_t i, j;
    for (i = 0; tunables_pmgr[i].chip_rev < UINT32_MAX; i++) {
        const struct tunable_struct *tunable_chip = tunables_pmgr[i].tunable;
        void *tunable_base = (void *)tunables_pmgr[i].base_address;
        uint32_t tunable_index;

        if (tunables_pmgr[i].chip_rev > chipid_get_chip_revision()) {
            continue;
        }

        for (tunable_index = 0; tunable_chip[tunable_index].offset != -1; tunable_index++) {
            uint32_t val;
            bool filtered = false;

            // Skip filtered cold boot tunables
            for (j = 0; tunables_filtered_pmgr[j].chip_rev < UINT32_MAX; j++) {
                if (tunables_pmgr[i].chip_rev == tunables_filtered_pmgr[j].chip_rev &&
                        tunables_pmgr[i].base_address + tunable_chip[tunable_index].offset >= tunables_filtered_pmgr[j].starting_address &&
                        tunables_pmgr[i].base_address + tunable_chip[tunable_index].offset <= tunables_filtered_pmgr[j].ending_address &&
                        tunables_filtered_pmgr[j].cold_boot) {
                    filtered = true;
                    break;
                }
            }

            if (filtered)
                continue;

            // TODO: Handle reconfig tunables and filtered reconfig tunables

            val = *((volatile uint32_t *)(tunable_base + tunable_chip[tunable_index].offset));

            if ((val & tunable_chip[tunable_index].mask) == tunable_chip[tunable_index].value) {
                // Tunable already applied. No need to apply it again
                continue;
            }

            val &= ~tunable_chip[tunable_index].mask;
            val |= tunable_chip[tunable_index].value;

            *((volatile uint32_t *)(tunable_base + tunable_chip[tunable_index].offset)) = (uint32_t)val;
        }
    }
}

static void set_nco_clocks(void)
{
    // Enable this NCO with alg_ref0_clk and nco_ref0_clk.
    // Note: clk_configs assumes all NCOs use nco_ref0_clk
    rPMGR_NCO_CLK_CFG(0) |= (PMGR_CLKCFG_NCO_REF0_CLK_CFG_ENABLE_INSRT(1));
    rPMGR_NCO_CLK_CFG(1) |= (PMGR_CLKCFG_NCO_REF1_CLK_CFG_ENABLE_INSRT(1));
}

static void set_clkcfg(volatile uint32_t *clkcfg, uint32_t value)
{
    uint32_t val;
    // Modify only the fields in mask
    uint32_t mask = PMGR_CLKCFG_ENABLE_UMASK | PMGR_CLKCFG_SRC_SEL_UMASK |
                    PMGR_CLKCFG_DIVISOR_UMASK | PMGR_CLKCFG_WAIT_COUNTER_UMASK;

    if (value & ~mask)
        panic("Fields mismatch with mask for value %#u in CLK_CFG %#u", value, (uint32_t) clkcfg);

    val = (*clkcfg) & ~mask;
    val |= value & mask;

    *clkcfg = val;

    while (PMGR_CLKCFG_PENDING_XTRCT(*clkcfg));
}

/* 
 * clocks_set_default - called by SecureROM, LLB, iBSS main via
 * platform_init_setup_clocks, so the current state of the chip is
 * either POR, or whatever 'quiesce' did when leaving SecureROM.
 */
int clocks_set_default(void)
{
    uint32_t cnt;
    volatile uint32_t *clkcfg;

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC) || APPLICATION_OSV
    enable_bira_work_around();
#endif

    clks[PMGR_CLK_OSC] = OSC_FREQ;

    // Change all clocks to something safe
    clocks_quiesce_internal();

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC) || APPLICATION_OSV
    apply_tunables();
#endif

// Setup active DVFM and SOC PERF states for the stage of boot.
#if APPLICATION_SECUREROM
    config_soc_perf_state(kSOC_PERF_STATE_SECUREROM);
#endif

    config_soc_perf_state(kSOC_PERF_STATE_VMIN);

#ifdef LPPLL_T
    set_lppll();
#endif
#ifdef PLL0_T
    set_pll(PLL0, PLL0_P, PLL0_M, PLL0_S, PLL0_VCO_OUT);
#endif
#ifdef PLL1_T
    set_pll(PLL1, PLL1_P, PLL1_M, PLL1_S, PLL1_VCO_OUT);
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
    // Turn on NCO clocks before enabling MCA clocks.
    set_nco_clocks();
#endif

    set_soc_perf_state(SOC_PERF_STATE_ACTIVE);

    // Set all spare clock divs to their active values
    for (cnt = 0; cnt < sizeof(spare_clock_configs_active) / sizeof(spare_clock_configs_active[0]); cnt++) {
        clkcfg = clk_configs[spare_clock_configs_active[cnt].clk].clock_reg;

        set_clkcfg(clkcfg, spare_clock_configs_active[cnt].clock_reg_val);
    }

    // Set all non-spare clock divs to their active values
    for (cnt = 0; cnt < sizeof(clk_configs_active) / sizeof(clk_configs_active[0]); cnt++) {
        clkcfg = clk_configs[clk_configs_active[cnt].clk].clock_reg;
        
        set_clkcfg(clkcfg, clk_configs_active[cnt].clock_reg_val);
    }

    power_on_sep();
    
    clocks_get_frequencies();

    return 0;
}

void clocks_quiesce(void)
{
    clocks_quiesce_internal();
}

uint32_t clocks_set_performance(uint32_t performance_level)
{
    uint32_t old_perf_level = perf_level;

#if APPLICATION_IBOOT
    uint32_t entry_a;

    switch (performance_level) {
        case kPerformanceMemoryLow:
        case kPerformanceMemoryMid:
        case kPerformanceMemoryFull:
            entry_a = rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN));
            entry_a &= ~PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;

            if (performance_level == kPerformanceMemoryLow)
                entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x3, 0x4); // 48 MHz MCU_REF_CLK, 48 MHz bin
            else
                entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x0, 0x3); // 88.8 MHz MCU_REF_CLK, 1600 MHz bin

            rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN)) = entry_a;

            set_soc_perf_state(kSOC_PERF_STATE_VMIN);
            perf_level = performance_level;
            break;
        default:
            break;
      }

    clocks_get_frequencies_range(PMGR_CLK_MCU_REF, PMGR_CLK_MCU_REF);
#endif

    return old_perf_level;
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
            freq = clks[PMGR_CLK_MCU_REF];
            break;            
        case CLK_CPU:
            freq = clks[PMGR_CLK_MCU_REF];
            break;
        case CLK_AOP:
            freq = clks[PMGR_CLK_AOP];	    
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
        reg &= ~(PMGR_CLKCFG_SRC_SEL_UMASK);
        reg |= PMGR_CLKCFG_SRC_SEL_INSRT(src_idx);
        set_clkcfg(clkcfg, reg);
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
    volatile uint32_t *reg;

    // Make sure we are within limits.
    if (!PMGR_VALID_DEVICE(device))
        return;

    reg = device_configs[PMGR_DEVICE_INDEX(device)].ps_reg;

    switch (device) {
        case CLK_AMC:
        default:
            *reg |= PMGR_PS_RESET_INSRT(1);
            spin(1);
            *reg &= ~PMGR_PS_RESET_INSRT(1);
        break;
    }
}

void platform_system_reset(bool panic)
{
#if WITH_BOOT_STAGE
    if (!panic) 
        boot_set_stage(kPowerNVRAMiBootStageOff);
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

static void config_soc_perf_state(uint32_t state)
{
    uint32_t index = state;
    
#if APPLICATION_IBOOT
    if (index >= kSOC_PERF_STATE_IBOOT_CNT)
        index = kSOC_PERF_STATE_VMIN;
#endif

    rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[index].entry[0];
}

static void set_soc_perf_state(uint32_t target_state)
{
    rPMGR_SOC_PERF_STATE_CTL = PMGR_SOC_PERF_STATE_TO_ENTRY(target_state);

    while (PMGR_SOC_PERF_STATE_CTL_PENDING_XTRCT(rPMGR_SOC_PERF_STATE_CTL));
}

static void set_lppll(void)
{    
    rMINIPMGR_LPPLL_CTL = MINIPMGR_LPPLL_CTL_ENABLE_INSRT(1);
    
    while (MINIPMGR_LPPLL_CTL_PENDING_XTRCT(rMINIPMGR_LPPLL_CTL));
}

static void clock_update_range(uint32_t first, uint32_t last, const uint32_t clkdata)
{
    volatile uint32_t *reg;
    uint32_t cnt;

    for (cnt = first; cnt <= last; cnt++) {
        reg = clk_configs[cnt].clock_reg;
        set_clkcfg(reg, clkdata);
    }
}

static void power_on_sep(void)
{
    volatile uint32_t *reg;
    uint32_t val;

    reg = device_configs[PMGR_DEVICE_INDEX(CLK_SEP)].ps_reg;

    val = *reg;
    val &= ~(PMGR_PS_SEP_PS_AUTO_PM_EN_INSRT(1) | PMGR_PS_SEP_PS_FORCE_NOACCESS_INSRT(1));
    *reg = val;
    while (PMGR_PS_SEP_PS_ACTUAL_PS_XTRCT(*reg) != PMGR_PS_RUN_MAX); // Wait for SEP to turn on.

    return;
}
