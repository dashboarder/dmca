/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _REGS_H
#define _REGS_H

#define CTRL_BASE_ADDR	(HDMI_BASE_ADDR + 0x00000)	//controller
#define	HDMI_CORE_BASE_ADDR	(HDMI_BASE_ADDR + 0x10000)	//HDMI 
#define	SPDIF_BASE_ADDR	(HDMI_BASE_ADDR + 0x30000)	//SPDIF receiver
#define	I2S_BASE_ADDR	(HDMI_BASE_ADDR + 0x40000)	//I2S receiver
#define	TG_BASE_ADDR		(HDMI_BASE_ADDR + 0x50000)	//HDMI timing generator
#define	E_FUSE_BASE_ADDR	(HDMI_BASE_ADDR + 0x60000)	//E-FUSE controller
#define	CEC_BASE_ADDR	(HDMI_BASE_ADDR + 0x70000)	//CEC 
#define	I2C_BASE_ADDR	(HDMI_BASE_ADDR + 0x80000)	//I2C

#define PHY_I2C_ADDRESS		0x70
#define	IICCONFIG_IICCON	0x00000201
#define	IICCONFIG_FIFOCON	0x00000000
#define	IICCONFIG_IICBUSCON	0x00000000
#define IICCONFIG_TIMEOUTCON	0xffffffff
//------------------------------------------------------------------------------
// MARK: - Generic HDMI -
//------------------------------------------------------------------------------

// General Control Packet Subpacket Byte 0
#define GCP_SB0_CLR_AVMUTE      0x10
#define GCP_SB0_SET_AVMUTE      0x01

// General Control Packet Subpacket Byte 1
#define GCP_SB1_CD_SHIFT        0
#define GCP_SB1_CD_MASK         0x07
#define GCP_SB1_CD_NONE         0
#define GCP_SB1_CD_24_BPP       4
#define GCP_SB1_CD_30_BPP       5
#define GCP_SB1_CD_36_BPP       6
#define GCP_SB1_CD_48_BPP       7

//------------------------------------------------------------------------------
// MARK: - PHY registers -
//
//  Note:
//      Register names are in hexadecimal.
//      Definitions of registers 01~1E are PHY version dependent.
//      The HDMI V1.3 TX may be used with either PHY version (V1.3 or V1.4).
//------------------------------------------------------------------------------

#define PHY_CONFIG_REG00        0x00u   // (R/W) Unused Reserved register
#define PHY_CONFIG_REG01        0x01u   // (R/W) First byte of PHY configuration
#define PHY_CONFIG_REG02        0x02u   // (R/W)
#define PHY_CONFIG_REG03        0x03u   // (R/W)
#define PHY_CONFIG_REG04        0x04u   // (R/W)
#define PHY_CONFIG_REG05        0x05u   // (R/W)
#define PHY_CONFIG_REG06        0x06u   // (R/W)
#define PHY_CONFIG_REG07        0x07u   // (R/W)
#define PHY_CONFIG_REG08        0x08u   // (R/W)
#define PHY_CONFIG_REG09        0x09u   // (R/W)
#define PHY_CONFIG_REG0A        0x0Au   // (R/W)
#define PHY_CONFIG_REG0B        0x0Bu   // (R/W)
#define PHY_CONFIG_REG0C        0x0Cu   // (R/W)
#define PHY_CONFIG_REG0D        0x0Du   // (R/W)
#define PHY_CONFIG_REG0E        0x0Eu   // (R/W)
#define PHY_CONFIG_REG0F        0x0Fu   // (R/W)
#define PHY_CONFIG_REG10        0x10u   // (R/W)
#define PHY_CONFIG_REG11        0x11u   // (R/W)
#define PHY_CONFIG_REG12        0x12u   // (R/W)
#define PHY_CONFIG_REG13        0x13u   // (R/W)
#define PHY_CONFIG_REG14        0x14u   // (R/W)
#define PHY_CONFIG_REG15        0x15u   // (R/W)
#define PHY_CONFIG_REG16        0x16u   // (R/W)
#define PHY_CONFIG_REG17        0x17u   // (R/W)
#define PHY_CONFIG_REG18        0x18u   // (R/W)
#define PHY_CONFIG_REG19        0x19u   // (R/W)
#define PHY_CONFIG_REG1A        0x1Au   // (R/W)
#define PHY_CONFIG_REG1B        0x1Bu   // (R/W)
#define PHY_CONFIG_REG1C        0x1Cu   // (R/W)
#define PHY_CONFIG_REG1D        0x1Du   // (R/W)
#define PHY_CONFIG_REG1E        0x1Eu   // (R/W) Last byte of PHY configuration

#define PHY_CONFIG_MODE_SET     0x1Fu   // (R/W) Control register for PHY config
    #define PHY_CONFIG_MODE_SET_IN_PROGRESS         0x00   // Set this value before loading REG01-REG1E
    #define PHY_CONFIG_MODE_SET_DONE                0x80   // Set this value after loading REG01-REG1E

#define PHY_CONFIG_REG_COUNT    0x20u

//------------------------------------------------------------------------------
// MARK: - HDMI v1.3 PHY registers -
//------------------------------------------------------------------------------

// REG01
    #define PHY_V1P3_CONFIG_REG01_GEN_PD        0x80   // Sigma delta modulator clock generator Power Down
    #define PHY_V1P3_CONFIG_REG01_BIAS_PD       0x20   // Bias Power Down - PLL & Bias required for HPD

// REG05
    #define PHY_V1P3_CONFIG_REG05_PLL_PD        0x20   // PLL Power Down - PLL & Bias required for HPD

// REG17
    #define PHY_V1P3_CONFIG_REG17_TX_PD         0x02   // TMDS TX power down
    #define PHY_V1P3_CONFIG_REG17_PCG_PD        0x01   // Pixel clock generator Power Down

//------------------------------------------------------------------------------
// MARK: - HDMI v1.4 PHY registers -
//------------------------------------------------------------------------------

// REG1D
    #define PHY_V1P4_CONFIG_REG1D_I2C_PDEN      0x80   // Enable the power down controls below
    #define PHY_V1P4_CONFIG_REG1D_PLL_PD        0x40   // PLL & Bias Power Down - PLL & Bias required for HPD
    #define PHY_V1P4_CONFIG_REG1D_TX_CLKSER_PD  0x20   // Clock Serializer Power Down
    #define PHY_V1P4_CONFIG_REG1D_TX_CLKDRV_PD  0x10   // TMDS Clock Driver Power Down
    #define PHY_V1P4_CONFIG_REG1D_TX_DATDRV_PD  0x04   // TMDS Data Driver Power Down
    #define PHY_V1P4_CONFIG_REG1D_TX_DATSER_PD  0x02   // TMDS Data Serializer Power Down
    #define PHY_V1P4_CONFIG_REG1D_TX_CLK_PD     0x01   // TX Internal Clock Buffer/Divider Power Down

    // TX Power Down bits
    #define PHY_V1P4_CONFIG_REG1D_TX_DRV_PD     (PHY_V1P4_CONFIG_REG1D_TX_DATDRV_PD |\
                                                 PHY_V1P4_CONFIG_REG1D_TX_CLKDRV_PD)

    // Power Down bits (excluding I2C_PDEN and PLL_PD)
    #define PHY_V1P4_CONFIG_REG1D_PD_WITH_HPD   (PHY_V1P4_CONFIG_REG1D_TX_CLKSER_PD | \
                                                 PHY_V1P4_CONFIG_REG1D_TX_CLKDRV_PD | \
                                                 PHY_V1P4_CONFIG_REG1D_TX_DATDRV_PD | \
                                                 PHY_V1P4_CONFIG_REG1D_TX_DATSER_PD | \
                                                 PHY_V1P4_CONFIG_REG1D_TX_CLK_PD)

    // All Power Down bits (excluding I2C_PDEN)
    #define PHY_V1P4_CONFIG_REG1D_FULL_PD       (PHY_V1P4_CONFIG_REG1D_PLL_PD | PHY_V1P4_CONFIG_REG1D_PD_WITH_HPD)

//------------------------------------------------------------------------------
// MARK: - Control Registers (CTRL_BASE) -
//------------------------------------------------------------------------------


#define CTRL_INTC_CON           0x0000u // (R/W) Specifies the interrupt control register. [0x00]
    #define CTRL_INTC_CON_POL_SHIFT         7
    #define CTRL_INTC_CON_POL_MASK          0x80
    #define CTRL_INTC_CON_POL_HIGH          0               // Active high
    #define CTRL_INTC_CON_POL_LOW           1               // Active low
    #define CTRL_INTC_CON_EN_GLOBAL         0x40
    #define CTRL_INTC_CON_EN_I2S            0x20
    #define CTRL_INTC_CON_EN_CEC            0x10
    #define CTRL_INTC_CON_EN_HPD_PLUG       0x08
    #define CTRL_INTC_CON_EN_HPD_UNPLUG     0x04
    #define CTRL_INTC_CON_EN_SPDIF          0x02
    #define CTRL_INTC_CON_EN_HDCP           0x01

    #define CTRL_INTC_CON_EN_HPD_CHG        (CTRL_INTC_CON_EN_HPD_PLUG|CTRL_INTC_CON_EN_HPD_UNPLUG)

#define CTRL_INTC_FLAG          0x0004u // (R/W) Specifies the interrupt flag register. [0x00]
    #define CTRL_INTC_FLAG_I2S              0x20
    #define CTRL_INTC_FLAG_CEC              0x10
    #define CTRL_INTC_FLAG_HPD_PLUG         0x08
    #define CTRL_INTC_FLAG_HPD_UNPLUG       0x04
    #define CTRL_INTC_FLAG_SPDIF            0x02
    #define CTRL_INTC_FLAG_HDCP             0x01

    #define CTRL_INTC_FLAG_HPD_CHG          (CTRL_INTC_FLAG_HPD_PLUG|CTRL_INTC_FLAG_HPD_UNPLUG)

#define CTRL_HDCP_KEY_STATUS    0x0008u // (R/O) Specifies the HDCP key status. [0x00]
    #define CTRL_HDCP_KEY_STATUS_LOAD_DONE  0x01

#define CTRL_HPD_STATUS         0x000Cu // (R/O) Specifies the value of HPD signal. [0x00]
    #define CTRL_HPD_STATUS_PLUGGED         0x01

// Common definitions for AUDIO_CLKSEL
#define CTRL_AUDIO_CLK_MASK                 0x01
#define CTRL_AUDIO_CLK_PCLK                 0
#define CTRL_AUDIO_CLK_SPDIF                1

// Common definitions for PHY_RSTOUT
#define CTRL_HDMI_PHY_RSTOUT_RESET          0x01  // Reset HDMI PHY (active high)

// Common definitions for CORE_RSTOUT
#define CTRL_HDMI_CORE_RSTOUT_RESET_F       0x01  // Reset HDMI core (active low)

//------------------------------------------------------------------------------
// MARK: - HDMI Core Registers (HDMI_CORE_BASE) -
//------------------------------------------------------------------------------

#define HDMI_CON_0              0x0000u // (R/W) Specifies the HDMI system control register 0. [0x00]
    #define HDMI_CON_0_BLUE_SCR_EN                  0x20  // Enable "blue screen" mode (TX video mute)
    #define HDMI_CON_0_ENCODING_RETAIN_BIT_ORDER    0x10  // Reverse (0) or retain (1)
    #define HDMI_CON_0_ASP_EN                       0x04
    #define HDMI_CON_0_SYSTEM_EN                    0x01

#define HDMI_CON_1              0x0004u // (R/W) Specifies the HDMI system control register 1. [0x00]
    #define HDMI_CON_1_PXL_LMT_CTRL_SHIFT           5
    #define HDMI_CON_1_PXL_LMT_CTRL_MASK            0x60
    #define HDMI_CON_1_PXL_LMT_CTRL_BYPASS_MODE     0
    #define HDMI_CON_1_PXL_LMT_CTRL_RGB_MODE        1
    #define HDMI_CON_1_PXL_LMT_CTRL_YCBCR_MODE      2

#define HDMI_CON_2              0x0008u // (R/W) Specifies the HDMI system control register 2. [0x00]
    #define HDMI_CON_2_VID_PERIOD_EN                0x20
    #define HDMI_CON_2_DVI_BAND_EN                  0x02

#define HDMI_STATUS             0x0010u // (R/W) Specifies the HDMI system status register. [0x00]
    #define HDMI_STATUS_AUTHEN_ACK                  0x80
    #define HDMI_STATUS_AUD_FIFO_OVF_INT            0x40
    #define HDMI_STATUS_UPDATE_Ri_INT               0x10  // Indicates that the {Ri} value should be read (HDCP)
    #define HDMI_STATUS_An_WRITE_INT                0x04  // Indicates that the {An} random value is ready (HDCP)
    #define HDMI_STATUS_WATCHDOG_INT                0x02
    #define HDMI_STATUS_I2C_INIT_INT                0x01

#define HDMI_STATUS_EN          0x0020u // (R/W) Specifies the HDMI system status enable register. [0x00]
    #define HDMI_STATUS_EN_AUD_FIFO_OVF_EN          0x40
    #define HDMI_STATUS_EN_UPDATE_Ri_EN             0x10
    #define HDMI_STATUS_EN_An_WRITE_EN              0x04
    #define HDMI_STATUS_EN_WATCHDOG_EN              0x02
    #define HDMI_STATUS_EN_I2C_INIT_EN              0x01

    // Enable mask including all HDCP interrupts except WATCHDOG_INT, which is not used.
    #define HDMI_STATUS_EN_HDCP_EN                  (HDMI_STATUS_EN_UPDATE_Ri_EN|HDMI_STATUS_EN_An_WRITE_EN|HDMI_STATUS_EN_I2C_INIT_EN)

#define HDMI_HPD                0x0030u // (R/W) Specifies the HPD control register. [0x00]
    #define HDMI_HPD_SW_HPD                         0x02
    #define HDMI_HPD_HPD_SEL_SW_HPD                 0x01

#define HDMI_MODE_SEL           0x0040u // (R/W) Selects the HDMI/DVI mode. [0x00]
    #define HDMI_MODE_SEL_MODE_SHIFT                0
    #define HDMI_MODE_SEL_MODE_MASK                 0x03
    #define HDMI_MODE_SEL_MODE_DISABLED             0
    #define HDMI_MODE_SEL_MODE_DVI                  1
    #define HDMI_MODE_SEL_MODE_HDMI                 2

#define HDMI_ENC_EN             0x0044u // (R/W) Specifies the HDCP encryption enable register [0x00]
    #define HDMI_ENC_EN_HDCP_ENC_EN                 0x01

//------------------------------------------------------------------------------
// MARK: - HDMI Video Related Registers -
//------------------------------------------------------------------------------

#define HDMI_YMAX               0x0060u // (R/W) Specifies the maximum Y (or R, G, B) pixel value [0xeb]
#define HDMI_YMIN               0x0064u // (R/W) Specifies the minimum Y (or R, G, B) pixel value [0x10]

#define HDMI_CMAX               0x0068u // (R/W) Specifies the maximum Cb/Cr pixel value [0xf0]
#define HDMI_CMIN               0x006Cu // (R/W) Specifies the minimum Cb/Cr pixel value [0x10]

#define HDMI_H_BLANK_0          0x00A0u // (R/W) Specifies the horizontal blanking setting [0x00]
#define HDMI_H_BLANK_1          0x00A4u // (R/W) Specifies the horizontal blanking setting [0x00]

#define HDMI_VSYNC_POL          0x00E4u // (R/W) Specifies the vertical sync polarity control register [0x00]

#define HDMI_INT_PRO_MODE       0x00E8u // (R/W) Specifies the interlace/progressive control register [0x00]

//------------------------------------------------------------------------------
// MARK: - HDMI Audio Related Registers -
//------------------------------------------------------------------------------

// Common definitions for HDMI_ASP_CON
    #define HDMI_ASP_CON_AUD_TYPE_SHIFT             5               // Audio packet type field position
    #define HDMI_ASP_CON_AUD_TYPE_MASK              0x60
    #define HDMI_ASP_CON_AUD_TYPE_ASP               0u              // Audio Sample Packet
    #define HDMI_ASP_CON_AUD_TYPE_1BIT              1u              // One-bit audio packet
    #define HDMI_ASP_CON_AUD_TYPE_HBR               2u              // High Bit Rate packet
    #define HDMI_ASP_CON_AUD_TYPE_DST               3u              // Direct Stream Transport packet

    #define HDMI_ASP_CON_AUD_MODE_SHIFT             4               // Audio packet layout field position
    #define HDMI_ASP_CON_AUD_MODE_MASK              0x10
    #define HDMI_ASP_CON_AUD_MODE_TWO_CHANNEL       0u              // Layout 0 (two-channel mode)
    #define HDMI_ASP_CON_AUD_MODE_MULTI_CHANNEL     1u              // Layout 1 (multi-channel mode)

    #define HDMI_ASP_CON_SP_PRE_SHIFT               0               // Sample Present field position (not used in two-channel mode)
    #define HDMI_ASP_CON_SP_PRE_MASK                0x0F

// Common definitions for HDMI_ACR_CON:
    #define HDMI_ACR_CON_ALT_CTS_RATE_SHIFT         3               // Alternate CTS rate field position
    #define HDMI_ACR_CON_ALT_CTS_RATE_MASK          0x18
    #define HDMI_ACR_CON_ALT_CTS_RATE_CONSTANT_CTS  0               // Always use CTS value 1
    #define HDMI_ACR_CON_ALT_CTS_RATE_1_TO_1        1               // 1:1 (CTS value 1 : CTS value 2)
    #define HDMI_ACR_CON_ALT_CTS_RATE_2_TO_1        2               // 2:1 (CTS value 1 : CTS value 2)
    #define HDMI_ACR_CON_ALT_CTS_RATE_3_TO_1        3               // 3:1 (CTS value 1 : CTS value 2)

    #define HDMI_ACR_CON_ACR_TX_MODE_SHIFT          0               // ACR transmit mode field position
    #define HDMI_ACR_CON_ACR_TX_MODE_MASK           0x07
    #define HDMI_ACR_CON_ACR_TX_MODE_DISABLED       0               // Do not transmit the ACR packet
    #define HDMI_ACR_CON_ACR_TX_MODE_ONCE           1               // Transmit one packet, when it becomes available
    #define HDMI_ACR_CON_ACR_TX_MODE_INTERVAL       2               // Transmit ACR_TXCNT times during every VBI period
    #define HDMI_ACR_CON_ACR_TX_MODE_COUNTING       3               // Transfers by counting i_clk_vid for a given CTS value in ACR_CTS0:2
    #define HDMI_ACR_CON_ACR_TX_MODE_MEASURED_CTS   4               // Mesured CTS mode.  Note: The 7 LSBs of ACR_N should be zero.

//------------------------------------------------------------------------------
// MARK: - HDMI Packet Related Registers -
//------------------------------------------------------------------------------

// Common definitions for:
//  GCP_CON[GCP_CON]
//  ACP_CON[ACP_TX_CON]
//  ISRC_CON[ISRC_TX_CON]
//  AVI_CON[AVI_TX_CON]
//  AUI_CON[AUI_TX_CON]
//  MPG_CON[MPG_TX_CON]
//  SPD_CON[SPD_TX_CON
//  GAMUT_CON[GAMUT_CON]
#define HDMI_PKT_CON_PKT_TX_CON_SHIFT               0
#define HDMI_PKT_CON_PKT_TX_CON_MASK                0x03
#define HDMI_PKT_CON_PKT_TX_CON_DISABLED            0
#define HDMI_PKT_CON_PKT_TX_CON_SEND_ONCE           1
#define HDMI_PKT_CON_PKT_TX_CON_SEND_ALWAYS         2

// Common definitions for GCP_CON
#define HDMI_GCP_CON_VSYNC_SHIFT                    2
#define HDMI_GCP_CON_VSYNC_MASK                     0x0C
#define HDMI_GCP_CON_VSYNC_NEITHER_FIELD            0
#define HDMI_GCP_CON_VSYNC_FIELD2                   1
#define HDMI_GCP_CON_VSYNC_FIELD1                   2
#define HDMI_GCP_CON_VSYNC_BOTH_FIELDS              3

// Common definitions for DC_CONTROL
#define HDMI_DC_CONTROL_DEEP_COLOR_MODE_SHIFT       0
#define HDMI_DC_CONTROL_DEEP_COLOR_MODE_MASK        0x03
#define HDMI_DC_CONTROL_DEEP_COLOR_MODE_8_BPC       (0 << HDMI_DC_CONTROL_DEEP_COLOR_MODE_SHIFT)
#define HDMI_DC_CONTROL_DEEP_COLOR_MODE_10_BPC      (1 << HDMI_DC_CONTROL_DEEP_COLOR_MODE_SHIFT)
#define HDMI_DC_CONTROL_DEEP_COLOR_MODE_12_BPC      (2 << HDMI_DC_CONTROL_DEEP_COLOR_MODE_SHIFT)

// Common definitions for VIDEO_PATTERN_GEN
#define HDMI_VIDEO_PATTERN_GEN_PATTERN_ENABLE       1 // Uses internally generated video pattern

//------------------------------------------------------------------------------
// MARK: - HDCP Related Registers -
//------------------------------------------------------------------------------

#define HDMI_HDCP_SHA1_00       0x0000u // (R/W) Specifies the SHA-1 value from repeater. [0x00]
#define HDMI_HDCP_KSV_LIST_0    0x0050u // (R/W) Specifies the KSV list from repeater. [0x00]

#define HDMI_HDCP_KSV_LIST_CON  0x0064u // (R/W) Controls the KSV list. [0x01]
    #define HDMI_HDCP_KSV_LIST_CON_WRITE            0x08
    #define HDMI_HDCP_KSV_LIST_CON_EMPTY            0x04
    #define HDMI_HDCP_KSV_LIST_CON_END              0x02
    #define HDMI_HDCP_KSV_LIST_CON_READ_DONE        0x01

#define HDMI_HDCP_SHA_RESULT    0x0070u // (R/W) Specifies the SHA-1 checking result register. [0x00]
    #define HDMI_HDCP_SHA_RESULT_VALID_READY        0x02
    #define HDMI_HDCP_SHA_RESULT_VALID              0x01

    #define HDMI_HDCP_SHA_RESULT_OK                 (HDMI_HDCP_SHA_RESULT_VALID_READY|HDMI_HDCP_SHA_RESULT_VALID)

#define HDMI_HDCP_CTRL1         0x0080u // (R/W) Specifies the HDCP control register1. [0x00]
    #define HDMI_HDCP_CTRL1_TIMEOUT                 0x04
    #define HDMI_HDCP_CTRL1_CP_DESIRED              0x02

#define HDMI_HDCP_CTRL2         0x0084u // (R/W) Specifies the HDCP control register2. [0x00]
    #define HDMI_HDCP_CTRL2_REVOCATION_SET          0x01

#define HDMI_HDCP_CHECK_RESULT  0x0090u // (R/W) Checks the result of Ri and Pj values. [0x00]
    #define HDMI_HDCP_CHECK_RESULT_Ri_SHIFT         0
    #define HDMI_HDCP_CHECK_RESULT_Ri_MASK          0x03
    #define HDMI_HDCP_CHECK_RESULT_Ri_RESET         0
    #define HDMI_HDCP_CHECK_RESULT_Ri_MISMATCH      2
    #define HDMI_HDCP_CHECK_RESULT_Ri_MATCH         3

#define HDMI_HDCP_BKSV_0        0x00A0u // (R/W) Specifies the KSV of Rx. [0x00]
#define HDMI_HDCP_AKSV_0        0x00C0u // (R/W) Specifies the KSV of Tx. [0x00]
#define HDMI_HDCP_An_0          0x00E0u // (R/W) Specifies the an value. [0x00]
#define HDMI_HDCP_BCAPS         0x0100u // (R/W) Specifies the BCAPS from Rx. [0x00]
#define HDMI_HDCP_BSTATUS_0     0x0110u // (R/W) Specifies the BSTATUS from Rx. [0x00]
#define HDMI_HDCP_BSTATUS_1     0x0114u // (R/W) Specifies the BSTATUS from Rx. [0x00]
#define HDMI_HDCP_Ri_0          0x0140u // (R/W) Specifies the Ri value of Tx. [0x00]
#define HDMI_HDCP_Ri_1          0x0144u // (R/W) Specifies the Ri value of Tx. [0x00]
#define HDMI_HDCP_I2C_INT       0x0180u // (R/W) Specifies the I2C interrupt flag. [0x00]
#define HDMI_HDCP_AN_INT        0x0190u // (R/W) Specifies the An value ready interrupt flag. [0x00]
#define HDMI_HDCP_WATCHDOG_INT  0x01A0u // (R/W) Specifies the Watchdog interrupt flag. [0x00]
#define HDMI_HDCP_Ri_INT        0x01B0u // (R/W) Specifies the Ri value update interrupt flag. [0x00]

#define HDMI_HDCP_Ri_COMPARE_0  0x01D0u // (R/W) Specifies the HDCP Ri interrupt frame number index register 0. [0x80]
#define HDMI_HDCP_Ri_COMPARE_1  0x01D4u // (R/W) Specifies the HDCP Ri interrupt frame number index register 1. [0x7f]
    #define HDMI_HDCP_Ri_COMPARE_ENABLE             0x80
    #define HDMI_HDCP_FRAME_INDEX_MASK              0x7F

#define HDMI_HDCP_FRAME_COUNT   0x01E0u // (R/W) Specifies the current value of frame count index in the hardware. [0x00]

//------------------------------------------------------------------------------
// MARK: - SPDIF Receiver Registers (SPDIF_BASE) -
//------------------------------------------------------------------------------

#define SPDIFIN_CLK_CTRL        0x0000u // (R/W) Specifies the SPDIFIN clock control register. [0x02]
#define SPDIFIN_OP_CTRL         0x0004u // (R/W) Specifies the SPDIFIN operation control register 1. [0x00]
#define SPDIFIN_IRQ_MASK        0x0008u // (R/W) Specifies the SPDIFIN interrupt request mask register. [0x00]
#define SPDIFIN_IRQ_STATUS      0x000Cu // (R/W) Specifies the SPDIFIN interrupt request status register. [0x00]
#define SPDIFIN_CONFIG_1        0x0010u // (R/W) Specifies the SPDIFIN configuration register 1. [0x02]
#define SPDIFIN_CONFIG_2        0x0014u // (R/W) Specifies the SPDIFIN configuration register 2. [0x00]
#define SPDIFIN_USER_VALUE_1    0x0020u // (R/W) Specifies the SPDIFIN user value register 1. [0x00]
#define SPDIFIN_USER_VALUE_2    0x0024u // (R/W) Specifies the SPDIFIN user value register 2. [0x00]
#define SPDIFIN_USER_VALUE_3    0x0028u // (R/W) Specifies the SPDIFIN user value register 3. [0x00]
#define SPDIFIN_USER_VALUE_4    0x002Cu // (R/W) Specifies the SPDIFIN user value register 4. [0x00]
#define SPDIFIN_CH_STATUS_0_1   0x0030u // (R/O) Specifies the SPDIFIN channel status register 0-1. [0x00]
#define SPDIFIN_CH_STATUS_0_2   0x0034u // (R/O) Specifies the SPDIFIN channel status register 0-2. [0x00]
#define SPDIFIN_CH_STATUS_0_3   0x0038u // (R/O) Specifies the SPDIFIN channel status register 0-3. [0x00]
#define SPDIFIN_CH_STATUS_0_4   0x003Cu // (R/O) Specifies the SPDIFIN channel status register 0-4. [0x00]
#define SPDIFIN_CH_STATUS_1     0x0040u // (R/O) Specifies the SPDIFIN channel status register 1. [0x00]
#define SPDIFIN_FRAME_PERIOD_1  0x0048u // (R/O) Specifies the SPDIFIN frame period register 1. [0x00]
#define SPDIFIN_FRAME_PERIOD_2  0x004Cu // (R/O) Specifies the SPDIFIN frame period register 2. [0x00]
#define SPDIFIN_Pc_INFO_1       0x0050u // (R/O) Specifies the SPDIFIN PC info register 1. [0x00]
#define SPDIFIN_Pc_INFO_2       0x0054u // (R/O) Specifies the SPDIFIN PC info register 2. [0x00]
#define SPDIFIN_Pd_INFO_1       0x0058u // (R/O) Specifies the SPDIFIN PD info register 1. [0x00]
#define SPDIFIN_Pd_INFO_2       0x005Cu // (R/O) Specifies the SPDIFIN PD Info register 2. [0x00]
#define SPDIFIN_DATA_BUF_0_1    0x0060u // (R/O) Specifies the SPDIFIN data buffer register 0_1. [0x00]
#define SPDIFIN_DATA_BUF_0_2    0x0064u // (R/O) Specifies the SPDIFIN data buffer register 0_2. [0x00]
#define SPDIFIN_DATA_BUF_0_3    0x0068u // (R/O) Specifies the SPDIFIN data buffer register 0_3. [0x00]
#define SPDIFIN_USER_BUF_0      0x006Cu // (R/O) Specifies the SPDIFIN user buffer register 0. [0x00]
#define SPDIFIN_DATA_BUF_1_1    0x0070u // (R/O) Specifies the SPDIFIN data buffer register 1_1. [0x00]
#define SPDIFIN_DATA_BUF_1_2    0x0074u // (R/O) Specifies the SPDIFIN data buffer register 1_2. [0x00]
#define SPDIFIN_DATA_BUF_1_3    0x0078u // (R/O) Specifies the SPDIFIN data buffer register 1_3. [0x00]
#define SPDIFIN_USER_BUF_1      0x007Cu // (R/O) Specifies the SPDIFIN user buffer register 1. [0x00]

//------------------------------------------------------------------------------
// MARK: - I2S Receiver Registers (I2S_BASE) -
//------------------------------------------------------------------------------

#define I2S_CLK_CON             0x0000u // (R/W) Specifies the I2S clock enable register. [0x00]
    #define I2S_CLK_CON_I2S_EN                      0x01

#define I2S_CON_1               0x0004u // (R/W) Specifies the I2S control register 1. [0x00]
    #define I2S_CON_1_SCLK_POL_SHIFT                1               // SCLK polarity field (r_sc_pol) position
    #define I2S_CON_1_SCLK_POL_MASK                 0x02  // SCLK polarity field (r_sc_pol)
    #define I2S_CON_1_SCLK_POL_FALLING_EDGE         0               // SDATA is syncrhonous to SCLK falling edge
    #define I2S_CON_1_SCLK_POL_RISING_EDGE          1               // SDATA is synchronous to SCLK rising edge

    #define I2S_CON_1_LRCLK_POL_SHIFT               0               // LRCLK polarity field (r_ch_pol) position
    #define I2S_CON_1_LRCLK_POL_MASK                0x01  // LRCLK polarity field (r_ch_pol)
    #define I2S_CON_1_LRCLK_POL_LOW                 0               // Left channel for low polarity
    #define I2S_CON_1_LRCLK_POL_HIGH                1               // Left channel for high polarity

#define I2S_CON_2               0x0008u // (R/W) Specifies the I2S control register 2. [0x16]
    #define I2S_CON_2_MLSB_SHIFT                    6               // I2S bit order field position
    #define I2S_CON_2_MLSB_MASK                     0x40
    #define I2S_CON_2_MLSB_MSB_FIRST                0               // MSB first mode
    #define I2S_CON_2_MLSB_LSB_FIRST                1               // LSB first mode

    #define I2S_CON_2_BIT_CH_SHIFT                  4               // Bit clock per frame field position
    #define I2S_CON_2_BIT_CH_MASK                   0x30
    #define I2S_CON_2_BIT_CH_32FS                   0               // Bit clock = 32*Fs
    #define I2S_CON_2_BIT_CH_48FS                   1               // Bit clock = 48*Fs

    #define I2S_CON_2_DATA_NUM_SHIFT                2               // Serial data bits per channel field position
    #define I2S_CON_2_DATA_NUM_MASK                 0x0c
    #define I2S_CON_2_DATA_NUM_16_BITS              1               // 16 bits per channel
    #define I2S_CON_2_DATA_NUM_20_BITS              2               // 20 bits per channel
    #define I2S_CON_2_DATA_NUM_24_BITS              3               // 24 bits per channel

    #define I2S_CON_2_I2S_MODE_SHIFT                0               // I2S mode field position
    #define I2S_CON_2_I2S_MODE_MASK                 0x03
    #define I2S_CON_2_I2S_MODE_BASIC_FORMAT         0               // I2S basic format
    #define I2S_CON_2_I2S_MODE_LEFT_JUSTIFIED       2               // Left-justified format
    #define I2S_CON_2_I2S_MODE_RIGHT_JUSTIFIED      3               // Right-justified format 

#define I2S_PIN_SEL_0           0x000Cu // (R/W) Specifies the I2S input pin selection register 0. [0x77]
    #define I2S_PIN_SEL_0_LRCLK_SHIFT               0
    #define I2S_PIN_SEL_0_LRCLK_MASK                0x07

    #define I2S_PIN_SEL_0_SCLK_SHIFT                4
    #define I2S_PIN_SEL_0_SCLK_MASK                 0x70

#define I2S_PIN_SEL_1           0x0010u // (R/W) Specifies the I2S input pin selection register 1. [0x77]
    #define I2S_PIN_SEL_1_SDATA0_SHIFT              0
    #define I2S_PIN_SEL_1_SDATA0_MASK               0x07

    #define I2S_PIN_SEL_1_SDATA1_SHIFT              4
    #define I2S_PIN_SEL_1_SDATA1_MASK               0x70

#define I2S_PIN_SEL_2           0x0014u // (R/W) Specifies the I2S input pin selection register 2. [0x77]
    #define I2S_PIN_SEL_2_SDATA2_SHIFT              0
    #define I2S_PIN_SEL_2_SDATA2_MASK               0x07

    #define I2S_PIN_SEL_2_SDATA3_SHIFT              4
    #define I2S_PIN_SEL_2_SDATA3_MASK               0x70

#define I2S_PIN_SEL_3           0x0018u // (R/W) Specifies the I2S input pin selection register 3. [0x07]
    #define I2S_PIN_SEL_3_DSD_D5_SHIFT              0
    #define I2S_PIN_SEL_3_DSD_D5_MASK               0x07

#define I2S_DSD_CON             0x001Cu // (R/W) Specifies the I2S DSD control register. [0x02]

#define I2S_IN_MUX_CON          0x0020u // (R/W) Specifies the I2S In/Mux control register. [0x60]
    #define I2S_IN_MUX_CON_F_NUM_SHIFT              5
    #define I2S_IN_MUX_CON_F_NUM_MASK               0xe0
    #define I2S_IN_MUX_CON_F_NUM_NONE               0               // Does not filter
    #define I2S_IN_MUX_CON_F_NUM_2                  1               // 2-stage filter
    #define I2S_IN_MUX_CON_F_NUM_3                  2               // 3-stage filter
    #define I2S_IN_MUX_CON_F_NUM_4                  3               // 4-stage filter
    #define I2S_IN_MUX_CON_F_NUM_5                  4               // 5-stage filter

    #define I2S_IN_MUX_CON_IN_EN                    0x10

    #define I2S_IN_MUX_CON_AUDIO_SEL_SHIFT          2               // AUDIO_SEL field position
    #define I2S_IN_MUX_CON_AUDIO_SEL_MASK           0x0c
    #define I2S_IN_MUX_CON_AUDIO_SEL_SPDIF          0               // Selects SPDIF audio data
    #define I2S_IN_MUX_CON_AUDIO_SEL_I2S            1               // Selects I2S audio data
    #define I2S_IN_MUX_CON_AUDIO_SEL_DSD            2               // Selects DSD audio data

    #define I2S_IN_MUX_CON_CUV_SEL_SHIFT            1               // CUV_SEL field position
    #define I2S_IN_MUX_CON_CUV_SEL_MASK             0x02
    #define I2S_IN_MUX_CON_CUV_SEL_SPDIF            0               // Selects SPDIF CUV data
    #define I2S_IN_MUX_CON_CUV_SEL_I2S              1               // Selects I2S CUV data

    #define I2S_IN_MUX_CON_MUX_EN                   0x01

#define I2S_CH_ST_CON           0x0024u // (R/W) Specifies the I2S channel status control register. [0x00]
    #define I2S_CH_ST_CON_RELOAD                    0x01

#define I2S_CH_ST_REG_COUNT     5       // Number of channel status registers

#define I2S_CH_ST_0             0x0028u // (R/W) Specifies the I2S channel status block 0. [0x00]
#define I2S_CH_ST_1             0x002Cu // (R/W) Specifies the I2S channel status block 1. [0x00]
#define I2S_CH_ST_2             0x0030u // (R/W) Specifies the I2S channel status block 2. [0x00]
#define I2S_CH_ST_3             0x0034u // (R/W) Specifies the I2S channel status block 3. [0x00]
#define I2S_CH_ST_4             0x0038u // (R/W) Specifies the I2S channel status block 4. [0x00]

#define I2S_CH_ST_SH_0          0x003Cu // (R/O) Specifies the I2S channel status block shadow register 0. [0x00]
#define I2S_CH_ST_SH_1          0x0040u // (R/O) Specifies the I2S channel status block shadow register 1. [0x00]
#define I2S_CH_ST_SH_2          0x0044u // (R/O) Specifies the I2S channel status block shadow register 2. [0x00]
#define I2S_CH_ST_SH_3          0x0048u // (R/O) Specifies the I2S channel status block shadow register 3. [0x00]
#define I2S_CH_ST_SH_4          0x004Cu // (R/O) Specifies the I2S channel status block shadow register 4. [0x00]

#define I2S_VD_DATA             0x0050u // (R/W) Specifies the I2S audio sample validity register. [0x00]
#define I2S_MUX_CH              0x0054u // (R/W) Specifies the I2S channel enable register. [0x03]

#define I2S_MUX_CUV             0x0058u // (R/W) Specifies the I2S CUV enable register. [0x03]
    #define I2S_MUX_CUV_R_EN                        0x02
    #define I2S_MUX_CUV_L_EN                        0x01

#define I2S_CH0_L_0             0x0064u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_L_1             0x0068u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_L_2             0x006Cu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_L_3             0x0070u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_R_0             0x0074u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_R_1             0x0078u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_R_2             0x007Cu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH0_R_3             0x0080u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_L_0             0x0084u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_L_1             0x0088u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_L_2             0x008Cu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_L_3             0x0090u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_R_0             0x0094u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_R_1             0x0098u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_R_2             0x009Cu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH1_R_3             0x00A0u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_L_0             0x00A4u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_L_1             0x00A8u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_L_2             0x00ACu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_L_3             0x00B0u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_R_0             0x00B4u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_R_1             0x00B8u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH2_R_2             0x00BCu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_Ch2_R_3             0x00C0u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH3_L_0             0x00C4u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH3_L_1             0x00C8u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH3_L_2             0x00CCu // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH3_R_0             0x00D0u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH3_R_1             0x00D4u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CH3_R_2             0x00D8u // (R/O) Specifies the I2S PCM output data register. [0x00]
#define I2S_CUV_L_R             0x00DCu // (R/O) Specifies the I2S CUV output data register. [0x00]

//------------------------------------------------------------------------------
// MARK: - Timing Generator Registers (TG_BASE) -
//------------------------------------------------------------------------------

#define TG_CMD                  0x0000u // (R/W) Specifies the command register. [0x00]
    #define TG_CMD_FIELD_EN                         0x02  // Enables field mode (for interlaced formats)
    #define TG_CMD_TG_EN                            0x01

#define TG_CFG                  0x0004u // (R/W) Specifies the pixel repetition ratio. [0x00]

#define TG_H_FSZ_L              0x0018u // (R/W) Specifies the horizontal full size. [0x72]
#define TG_H_FSZ_H              0x001Cu // (R/W) Specifies the horizontal full size. [0x06]

#define TG_HACT_ST_L            0x0020u // (R/W) Specifies the horizontal active start. [0x05]
#define TG_HACT_ST_H            0x0024u // (R/W) Specifies the horizontal active start. [0x01]

#define TG_HACT_SZ_L            0x0028u // (R/W) Specifies the horizontal active size. [0x00]
#define TG_HACT_SZ_H            0x002Cu // (R/W) Specifies the horizontal active size. [0x05]

#define TG_V_FSZ_L              0x0030u // (R/W) Specifies the vertical full line size. [0xEE]
#define TG_V_FSZ_H              0x0034u // (R/W) Specifies the vertical full line size. [0x02]

#define TG_VSYNC_L              0x0038u // (R/W) Specifies the vertical sync position. [0x01]
#define TG_VSYNC_H              0x003Cu // (R/W) Specifies the vertical sync position. [0x00]

#define TG_VSYNC2_L             0x0040u // (R/W) Specifies the vertical sync position for bottom field. [0x33]
#define TG_VSYNC2_H             0x0044u // (R/W) Specifies the vertical sync position for bottom field. [0x02]

#define TG_VACT_ST_L            0x0048u // (R/W) Specifies the vertical sync active start position. [0x1a]
#define TG_VACT_ST_H            0x004Cu // (R/W) Specifies the vertical sync active start position. [0x00]

#define TG_VACT_SZ_L            0x0050u // (R/W) Specifies the vertical active size. [0xd0]
#define TG_VACT_SZ_H            0x0054u // (R/W) Specifies the vertical active size. [0x02]

#define TG_FIELD_CHG_L          0x0058u // (R/W) Specifies the HDMI field change position. [0x33]
#define TG_FIELD_CHG_H          0x005Cu // (R/W) Specifies the HDMI field change position. [0x02]

#define TG_VACT_ST2_L           0x0060u // (R/W) Specifies the HDMI vertical active start position for bottom field. [0x48]
#define TG_VACT_ST2_H           0x0064u // (R/W) Specifies the HDMI vertical active start position for bottom field. [0x02]

#define TG_VSYNC_TOP_HDMI_L     0x0078u // (R/W) Specifies the HDMI VSYNC position for top field. [0x01]
#define TG_VSYNC_TOP_HDMI_H     0x007Cu // (R/W) Specifies the HDMI VSYNC position for top field. [0x00]

#define TG_VSYNC_BOT_HDMI_L     0x0080u // (R/W) Specifies the HDMI VSYNC position for bottom field. [0x33]
#define TG_VSYNC_BOT_HDMI_H     0x0084u // (R/W) Specifies the HDMI VSYNC position for bottom field. [0x02]

#define TG_FIELD_TOP_HDMI_L     0x0088u // (R/W) Specifies the HDMI top field start position. [0x01]
#define TG_FIELD_TOP_HDMI_H     0x008Cu // (R/W) Specifies the HDMI top field start position. [0x00]

#define TG_FIELD_BOT_HDMI_L     0x0090u // (R/W) Specifies the HDMI bottom field start position. [0X33]
#define TG_FIELD_BOT_HDMI_H     0x0094u // (R/W) Specifies the HDMI bottom field start position. [0x02]

//------------------------------------------------------------------------------
// MARK: - HDCP E-Fuse Control Registers (eFUSE_BASE) -
//------------------------------------------------------------------------------

#define EFUSE_HDCP_CTRL         0x0000u // (  W) Specifies the HDCP e-fuse reading control. [0x00]
    #define EFUSE_HDCP_CTRL_HDCP_KEY_READ           0x01

#define EFUSE_HDCP_STATUS       0x0004u // (R/O) Specifies the HDCP e-fuse reading status. [0x00]
    #define EFUSE_HDCP_STATUS_ECC_FAIL              0x04
    #define EFUSE_HDCP_STATUS_ECC_BUSY              0x02
    #define EFUSE_HDCP_STATUS_ECC_DONE              0x01

#define EFUSE_ADDR_WIDTH        0x0008u // (R/W) Specifies the address width. [0x14]
#define EFUSE_SIGDEV_ASSERT     0x000Cu // (R/W) Specifies the SIGDEV asserting position. [0x00]
#define EFUSE_SIGDEV_DEASSERT   0x0010u // (R/W) Specifies the SIGDEV de-asserting position. [0x08]
#define EFUSE_PRCHG_ASSERT      0x0014u // (R/W) Specifies the PRCHG asserting position. [0x00]
#define EFUSE_PRCHG_DEASSERT    0x0018u // (R/W) Specifies the PRCHG de-asserting position. [0x0C]
#define EFUSE_FSET_ASSERT       0x001Cu // (R/W) Specifies the FSET asserting position. [0x04]
#define EFUSE_FSET_DEASSERT     0x0020u // (R/W) Specifies the FSET de-asserting position. [0x10]
#define EFUSE_SENSING           0x0024u // (R/W) Specifies the sensing width. [0x14]
#define EFUSE_SCK_ASSERT        0x0028u // (R/W) Specifies the SCK asserting position. [0x04]
#define EFUSE_SCK_DEASSERT      0x002Cu // (R/W) Specifies the SCK de-asserting position. [0x0C]
#define EFUSE_SDOUT_OFFSET      0x0030u // (R/W) Specifies the SDOUT offset. [0x10]
#define EFUSE_READ_OFFSET       0x0034u // (R/W) Specifies the READ offset. [0x14]

#define EFUSE_HDCP_SW_KEY       0x0040u // (R/W) Specifies the software-provided HDCP test key.
#define EFUSE_HDCP_KEY_CTRL     0x0580u // (R/W) Specifies the HDCP key control register.
    #define EFUSE_HDCP_KEY_CTRL_WRITE_KEY           0x01

//------------------------------------------------------------------------------
// MARK: - HDMI CEC Registers (CEC_BASE) -
//------------------------------------------------------------------------------

#define CEC_TX_STATUS_0         0x0000u // (R/O) Specifies the CEC TX status #0. [0x00]
#define CEC_TX_STATUS_1         0x0004u // (R/O) Specifies the CEC TX status #1. [0x00]
#define CEC_RX_STATUS_0         0x0008u // (R/O) Specifies the CEC RX status #0. [0x00]
#define CEC_RX_STATUS_1         0x000Cu // (R/O) Specifies the CEC Rx status #1. [0x00]
#define CEC_INTR_MASK           0x0010u // (R/W) Specifies the CEC interrupt masking control. [0x00]
#define CEC_INTR_CLEAR          0x0014u // (R/W) Specifies the CEC interrupt clearing control. [0x00]
#define CEC_LOGIC_ADDR          0x0020u // (R/W) Specifies the HDMI Tx logical address. [0x0F]
#define CEC_DIVISOR_0           0x0030u // (R/W) Specifies the divisor used in counting 0.05ms. [0x00]
#define CEC_DIVISOR_1           0x0034u // (R/W) Specifies the divisor used in counting 0.05ms. [0x00]
#define CEC_DIVISOR_2           0x0038u // (R/W) Specifies the divisor used in counting 0.05ms. [0x00]
#define CEC_DIVISOR_3           0x003Cu // (R/W) Specifies the divisor used in counting 0.05ms. [0x00]
#define CEC_TX_CTRL             0x0040u // (R/W) Specifies the CEC Tx control. [0x50]
#define CEC_TX_BYTE_NUM         0x0044u // (R/W) Specifies the number of blocks in a message that has to be sent. [0x00]
#define CEC_TX_STATUS_2         0x0060u // (R/O) Specifies the CEC Tx status #2. [0x00]
#define CEC_TX_STATUS_3         0x0064u // (R/O) Specifies the CEC Tx status #3. [0x00]
#define CEC_TX_BUFFER_0         0x0080u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_1         0x0084u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_2         0x0088u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_3         0x008Cu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_4         0x0090u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_5         0x0094u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_6         0x0098u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_7         0x009Cu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_8         0x00A0u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_9         0x00A4u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_10        0x00A8u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_11        0x00ACu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_12        0x00B0u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_13        0x00B4u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_14        0x00B8u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_TX_BUFFER_15        0x00BCu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_CTRL             0x00C0u // (R/W) Specifies the CEC RX control. [0x00]
#define CEC_RX_STATUS_2         0x00E0u // (R/O) Specifies the CEC RX status #2. [0x00]
#define CEC_RX_STATUS_3         0x00E4u // (R/O) Specifies the CEC RX status #3. [0x00]
#define CEC_RX_BUFFER_0         0x0100u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_1         0x0104u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_2         0x0108u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_3         0x010Cu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_4         0x0110u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_5         0x0114u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_6         0x0118u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_7         0x011Cu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_8         0x0120u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_9         0x0124u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_10        0x0128u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_11        0x012Cu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_12        0x0130u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_13        0x0134u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_14        0x0138u // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_RX_BUFFER_15        0x013Cu // (R/W) Specifies the byte #0 to #15 of CEC message. [0x00]
#define CEC_FILTER_CTRL         0x0180u // (R/W) Specifies the CEC filter control. [0x81]
#define CEC_FILTER_TH           0x0184u // (R/W) Specifies the filter threshold value. [0x03]

//------------------------------------------------------------------------------
// MARK: - Control Registers (CTRL_BASE) -
//------------------------------------------------------------------------------

#define CTRL_INTC_CON_1         0x0010u // (R/W) Interrupt Control Register 1 [0x00]

#define CTRL_INTC_FLAG_1        0x0014u // (R/W) Interrupt Flag Register 1 [0x00]

#define CTRL_PHY_STATUS_0       0x0020u // (R/O) PHY status Register 0 [0x00]
    #define CTRL_PHY_STATUS_0_PHY_READY             0x1

#define CTRL_PHY_STATUS_CMU     0x0024u // (R/O) PHY CMU status Register [0x00]

#define CTRL_PHY_STATUS_PLL     0x0028u // (R/O) PHY PLL status Register [0x00]

#define CTRL_PHY_CON_0          0x0030u // (R/W) PHY Control Register [0x01]
    #define CTRL_PHY_CON_0_PHY_PWR_OFF              0x01

#define CTRL_HPD_CTRL           0x0040u // (R/W) HPD Signal Control Register [0x02]

#define CTRL_HPD_ST             0x0044u // (R/W) HPD Status Register [0x00]

#define CTRL_HPD_TH_x           0x0050u // (R/W) HPD filter threshold value Register [0x00]

#define CTRL_AUDIO_CLKSEL       0x0070u // (R/W) Selects the audio system clock. [0x01]

#define CTRL_PHY_RSTOUT         0x0074u // (R/W) Specifies the HDMI PHY reset out. [0x01]

#define CTRL_PHY_VPLL           0x0078u // (R/O) Specifies the HDMI PHY VPLL monitor. [0x00]

#define CTRL_PHY_CMU            0x007Cu // (R/O) Specifies the HDMI PHY CMU monitor. [0x00]

#define CTRL_CORE_RSTOUT        0x0080u // (R/W) Specifies the HDMI TX core software reset. [0x01]

#define CTRL_HDMI_VERSION_0     0x0100u // (R/O) Specifies the HDMI_LINK Version information. HDMI_VERSION[7:0] [0x08]
#define CTRL_HDMI_VERSION_1     0x0104u // (R/O) Specifies the HDMI_LINK Version information. HDMI_VERSION[15:8] [0x00]
#define CTRL_HDMI_VERSION_2     0x0108u // (R/O) Specifies the HDMI_LINK Version information. HDMI_VERSION[23:16] [0x00]
#define CTRL_HDMI_VERSION_3     0x010Cu // (R/O) Specifies the HDMI_LINK Version information. HDMI_VERSION[31:24] [0x00]

//------------------------------------------------------------------------------
// MARK: - HDMI Video Related Registers -
//------------------------------------------------------------------------------

// HDMI_CON_0 definitions
    #define HDMI_CON_0_YCBCR422_SEL                 0x08  // YCbCr Video Input Mode (0=4:4:4, 1=4:2:2)

#define HDMI_V2_BLANK_0         0x00B0u // (R/W) Specifies the vertical blanking setting. [0x00]
#define HDMI_V2_BLANK_1         0x00B4u // (R/W) Specifies the vertical blanking setting. [0x00]
#define HDMI_V1_BLANK_0         0x00B8u // (R/W) Specifies the vertical blanking setting. [0x00]
#define HDMI_V1_BLANK_1         0x00BCu // (R/W) Specifies the vertical blanking setting. [0x00]

#define HDMI_V_LINE_0           0x00C0u // (R/W) Specifies the horizontal line setting. [0x00]
#define HDMI_V_LINE_1           0x00C4u // (R/W) Specifies the horizontal line setting. [0x00]
#define HDMI_H_LINE_0           0x00C8u // (R/W) Specifies the vertical line setting. [0x00]
#define HDMI_H_LINE_1           0x00CCu // (R/W) Specifies the vertical line setting. [0x00]

#define HDMI_HSYNC_POL          0x00E0u // (R/W) Specifies the Horizontal sync polarity control register [0x00]

#define HDMI_V_BLANK_F0_0       0x0110u // (R/W) Specifies the vertical blanking setting for bottom field. [0xff]
#define HDMI_V_BLANK_F0_1       0x0114u // (R/W) Specifies the vertical blanking setting for bottom field. [0x1f]
#define HDMI_V_BLANK_F1_0       0x0118u // (R/W) Specifies the vertical blanking setting for bottom field. [0xff]
#define HDMI_V_BLANK_F1_1       0x011Cu // (R/W) Specifies the vertical blanking setting for bottom field. [0x1f]

#define HDMI_H_SYNC_START_0     0x0120u // (R/W) Specifies Horizontal sync generation setting [0x00]
#define HDMI_H_SYNC_START_1     0x0124u // (R/W) Specifies Horizontal sync generation setting [0x00]

#define HDMI_H_SYNC_END_0       0x0128u // (R/W) Specifies Horizontal sync generation setting  [0x00]
#define HDMI_H_SYNC_END_1       0x012Cu // (R/W) Specifies Horizontal sync generation setting  [0x00]

#define HDMI_V_SYNC_LINE_BEF_2_0      0x0130u // (R/W) Specifies Vertical sync generation for top field or frame  [0xff]
#define HDMI_V_SYNC_LINE_BEF_2_1      0x0134u // (R/W) Specifies Vertical sync generation for top field or frame  [0x1f]
#define HDMI_V_SYNC_LINE_BEF_1_0      0x0138u // (R/W) Specifies Vertical sync generation for top field or frame  [0xff]
#define HDMI_V_SYNC_LINE_BEF_1_1      0x013Cu // (R/W) Specifies Vertical sync generation for top field or frame  [0x1f]

#define HDMI_V_SYNC_LINE_AFT_2_0      0x0140u // (R/W) Specifies Vertical sync generation for bottom field - vertical position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_2_1      0x0144u // (R/W) Specifies Vertical sync generation for bottom field - vertical position  [0x1f]
#define HDMI_V_SYNC_LINE_AFT_1_0      0x0148u // (R/W) Specifies Vertical sync generation for bottom field - vertical position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_1_1      0x014Cu // (R/W) Specifies Vertical sync generation for bottom field - vertical position  [0x1f]

#define HDMI_V_SYNC_LINE_AFT_PXL_2_0  0x0150u // (R/W) Specifies Vertical sync generation for bottom field - horizontal position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_PXL_2_1  0x0154u // (R/W) Specifies Vertical sync generation for bottom field - horizontal position  [0x1f]
#define HDMI_V_SYNC_LINE_AFT_PXL_1_0  0x0158u // (R/W) Specifies Vertical sync generation for bottom field - horizontal position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_PXL_1_1  0x015Cu // (R/W) Specifies Vertical sync generation for bottom field - horizontal position  [0x1f]

#define HDMI_V_BLANK_F2_0       0x0160u // (R/W) Specifies Vertical blanking setting for third field  [0xff]
#define HDMI_V_BLANK_F2_1       0x0164u // (R/W) Specifies Vertical blanking setting for third field  [0x1f]
#define HDMI_V_BLANK_F3_0       0x0168u // (R/W) Specifies Vertical blanking setting for thrid field  [0xff]
#define HDMI_V_BLANK_F3_1       0x016Cu // (R/W) Specifies Vertical blanking setting for thrid field  [0x1f]

#define HDMI_V_BLANK_F4_0       0x0170u // (R/W) Specifies Vertical blanking setting for fourth field  [0xff]
#define HDMI_V_BLANK_F4_1       0x0174u // (R/W) Specifies Vertical blanking setting for fourth field  [0x1f]
#define HDMI_V_BLANK_F5_0       0x0178u // (R/W) Specifies Vertical blanking setting for fourth field  [0xff]
#define HDMI_V_BLANK_F5_1       0x017Cu // (R/W) Specifies Vertical blanking setting for fourth field  [0x1f]

#define HDMI_V_SYNC_LINE_AFT_3_0      0x0180u // (R/W) Specifies Vertical sync generation for third field - vertical position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_3_1      0x0184u // (R/W) Specifies Vertical sync generation for third field - vertical position  [0x1f]
#define HDMI_V_SYNC_LINE_AFT_4_0      0x0188u // (R/W) Specifies Vertical sync generation for third field - vertical position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_4_1      0x018Cu // (R/W) Specifies Vertical sync generation for third field - vertical position  [0x1f]

#define HDMI_V_SYNC_LINE_AFT_5_0      0x0190u // (R/W) Specifies Vertical sync generation for fourth field - vertical position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_5_1      0x0194u // (R/W) Specifies Vertical sync generation for fourth field - vertical position  [0x1f]
#define HDMI_V_SYNC_LINE_AFT_6_0      0x0198u // (R/W) Specifies Vertical sync generation for fourth field - vertical position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_6_1      0x019Cu // (R/W) Specifies Vertical sync generation for fourth field - vertical position  [0x1f]

#define HDMI_V_SYNC_LINE_AFT_PXL_3_0  0x01A0u // (R/W) Specifies Vertical sync generation for third field - horizontal position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_PXL_3_1  0x01A4u // (R/W) Specifies Vertical sync generation for third field - horizontal position  [0x1f]
#define HDMI_V_SYNC_LINE_AFT_PXL_4_0  0x01A8u // (R/W) Specifies Vertical sync generation for third field - horizontal position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_PXL_4_1  0x01ACu // (R/W) Specifies Vertical sync generation for third field - horizontal position  [0x1f]

#define HDMI_V_SYNC_LINE_AFT_PXL_5_0  0x01B0u // (R/W) Specifies Vertical sync generation for fourth field - horizontal position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_PXL_5_1  0x01B4u // (R/W) Specifies Vertical sync generation for fourth field - horizontal position  [0x1f]
#define HDMI_V_SYNC_LINE_AFT_PXL_6_0  0x01B8u // (R/W) Specifies Vertical sync generation for fourth field - horizontal position  [0xff]
#define HDMI_V_SYNC_LINE_AFT_PXL_6_1  0x01BCu // (R/W) Specifies Vertical sync generation for fourth field - horizontal position  [0x1f]

#define HDMI_VACT_SPACE1_0      0x01C0u // (R/W) Specifies 1st Vertical Active Space start line  [0xff]
#define HDMI_VACT_SPACE1_1      0x01C4u // (R/W) Specifies 1st Vertical Active Space end line  [0x1f]
#define HDMI_VACT_SPACE2_0      0x01C8u // (R/W) Specifies 1st Vertical Active Space start line  [0xff]
#define HDMI_VACT_SPACE2_1      0x01CCu // (R/W) Specifies 1st Vertical Active Space end line  [0x1f]

#define HDMI_VACT_SPACE3_0      0x01D0u // (R/W) Specifies 2nd Vertical Active Space start line  [0xff]
#define HDMI_VACT_SPACE3_1      0x01D4u // (R/W) Specifies 2nd Vertical Active Space end line  [0x1f]
#define HDMI_VACT_SPACE4_0      0x01D8u // (R/W) Specifies 2nd Vertical Active Space start line  [0xff]
#define HDMI_VACT_SPACE4_1      0x01DCu // (R/W) Specifies 2nd Vertical Active Space end line  [0x1f]

#define HDMI_VACT_SPACE5_0      0x01E0u // (R/W) Specifies 3rd Vertical Active Space start line  [0xff]
#define HDMI_VACT_SPACE5_1      0x01E4u // (R/W) Specifies 3rd Vertical Active Space end line  [0x1f]
#define HDMI_VACT_SPACE6_0      0x01E8u // (R/W) Specifies 3rd Vertical Active Space start line  [0xff]
#define HDMI_VACT_SPACE6_1      0x01ECu // (R/W) Specifies 3rd Vertical Active Space end line  [0x1f]

#define HDMI_GCP_CON            0x0200u // (R/W) Specifies GCP packet control register  [0x04]
#define HDMI_GCP_BYTE1          0x0210u // (R/W) Specifies GCP packet body  [0x00]
#define HDMI_GCP_BYTE2          0x0214u // (R/W) Specifies GCP packet body  [0x00]
#define HDMI_GCP_BYTE3          0x0218u // (R/W) Specifies GCP packet body  [0x00]

//------------------------------------------------------------------------------
// MARK: - HDMI Audio Related Registers -
//------------------------------------------------------------------------------

#define HDMI_ASP_CON            0x0300u // (R/W) Specifies the ASP packet control register. [0x00]
#define HDMI_ASP_SP_FLAT        0x0304u // (R/W) Specifies the ASP packet sp_flat bit control. [0x00]
#define HDMI_ASP_CHCFG0         0x0310u // (R/W) Specifies the ASP audio channel configuration. [0x08]
#define HDMI_ASP_CHCFG1         0x0314u // (R/W) Specifies the ASP audio channel configuration. [0x1a]
#define HDMI_ASP_CHCFG2         0x0318u // (R/W) Specifies the ASP audio channel configuration. [0x2c]
#define HDMI_ASP_CHCFG3         0x031Cu // (R/W) Specifies the ASP audio channel configuration. [0x3e]

#define HDMI_ACR_CON            0x0400u // (R/W) Specifies the ACR packet control register. [0x00]

#define HDMI_ACR_MCTS0          0x0410u // (R/W) Specifies the measured CTS value. [0x01]
#define HDMI_ACR_MCTS1          0x0414u // (R/W) Specifies the measured CTS value. [0x00]
#define HDMI_ACR_MCTS2          0x0418u // (R/W) Specifies the measured CTS value. [0x00]

#define HDMI_ACR_N0             0x0430u // (R/W) Specifies the N value for ACR packet. [0xe8]
#define HDMI_ACR_N1             0x0434u // (R/W) Specifies the N value for ACR packet. [0x03]
#define HDMI_ACR_N2             0x0438u // (R/W) Specifies the N value for ACR packet. [0x00]

//------------------------------------------------------------------------------
// MARK: - HDMI Packet Related Registers -
//------------------------------------------------------------------------------

#define HDMI_ACP_CON            0x0500u // (R/W) Specifies the ACP packet control register. [0x00]
#define HDMI_ACP_TYPE           0x0514u // (R/W) Specifies the ACP packet header. [0x00]
#define HDMI_ACP_DATA00         0x0520u // (R/W) Specifies the ACP packet body. [0x00]

#define HDMI_ISRC_CON           0x0600u // (R/W) Specifies the ACR packet control register. [0x00]
#define HDMI_ISRC1_HEADER1      0x0614u // (R/W) Specifies the ISCR1 packet header. [0x00]
#define HDMI_ISRC1_DATA00       0x0620u // (R/W) Specifies the ISRC1 packet body. [0x00]
#define HDMI_ISRC2_DATA00       0x06A0u // (R/W) Specifies the ISRC2 packet body. [0x00]

#define HDMI_AVI_CON            0x0700u // (R/W) Specifies the AVI packet control register. [0x00]
#define HDMI_AVI_HEADER0        0x0710u // (R/W) Specifies the AVI packet header. [0x00]
#define HDMI_AVI_HEADER1        0x0714u // (R/W) Specifies the AVI packet header. [0x00]
#define HDMI_AVI_HEADER2        0x0718u // (R/W) Specifies the AVI packet header. [0x00]
#define HDMI_AVI_CHECK_SUM      0x071Cu // (R/W) Specifies the AVI packet checksum. [0x00]
#define HDMI_AVI_BYTE01         0x0720u // (R/W) Specifies the AVI packet body. [0x00]

#define HDMI_AUI_CON            0x0800u // (R/W) Specifies the AUI packet control register. [0x00]
#define HDMI_AUI_HEADER0        0x0810u // (R/W) Specifies AUI packet header. [0x00]
#define HDMI_AUI_HEADER1        0x0814u // (R/W) Specifies AUI packet header. [0x00]
#define HDMI_AUI_HEADER2        0x0818u // (R/W) Specifies AUI packet header. [0x00]
#define HDMI_AUI_CHECK_SUM      0x081Cu // (R/W) Specifies the AUI packet checksum. [0x00]
#define HDMI_AUI_BYTE1          0x0820u // (R/W) Specifies the AUI packet body. [0x00]

#define HDMI_MPG_CON            0x0900u // (R/W) Specifies the ACR packet control register. [0x00]
#define HDMI_MPG_CHECK_SUM      0x091Cu // (R/W) Specifies the MPG packet checksum. [0x00]
#define HDMI_MPG_BYTE1          0x0920u // (R/W) Specifies the MPG packet body. [0x00]

#define HDMI_SPD_CON            0x0A00u // (R/W) Specifies the SPD packet control register. [0x00]
#define HDMI_SPD_HEADER0        0x0A10u // (R/W) Specifies the SPD packet header. [0x00]
#define HDMI_SPD_HEADER1        0x0A14u // (R/W) Specifies the SPD packet header. [0x00]
#define HDMI_SPD_HEADER2        0x0A18u // (R/W) Specifies the SPD packet header. [0x00]
#define HDMI_SPD_DATA00         0x0A20u // (R/W) Specifies the SPD packet checksum. [0x00]
#define HDMI_SPD_DATA01         0x0A24u // (R/W) Specifies the SPD packet body. [0x00]

#define HDMI_GAMUT_CON          0x0B00u // (R/W) Specifies GAMUT packet control register. [0x00]
#define HDMI_GAMUT_HEADER0      0x0B10u // (R/W) Specifies GAMUT packet header. [0x00]
#define HDMI_GAMUT_HEADER1      0x0B14u // (R/W) Specifies GAMUT packet header. [0x00]
#define HDMI_GAMUT_HEADER2      0x0B18u // (R/W) Specifies GAMUT packet header. [0x00]
#define HDMI_GAMUT_METADATA00   0x0B20u // (R/W) Specifies GAMUT packet body.  [0x00]

#define HDMI_VSI_CON            0x0C00u // (R/W) Specifies VSI packet control register. [0x00]
#define HDMI_VSI_HEADER0        0x0C10u // (R/W) Specifies VSI packet header. [0x00]
#define HDMI_VSI_HEADER1        0x0C14u // (R/W) Specifies VSI packet header. [0x00]
#define HDMI_VSI_HEADER2        0x0C18u // (R/W) Specifies VSI packet header. [0x00]
#define HDMI_VSI_DATA00         0x0C20u // (R/W) Specifies VSI packet checksum. [0x00]
#define HDMI_VSI_DATA01         0x0C24u // (R/W) Specifies VSI packet body. [0x00]

//------------------------------------------------------------------------------
// MARK: - HDMI Miscellaneous Registers -
//------------------------------------------------------------------------------

#define HDMI_DC_CONTROL         0x0D00u // (R/W) Specifies Deep Color Control Register  [0x00]

#define HDMI_VIDEO_PATTERN_GEN  0x0D04u // (R/W) Specifies Video Pattern Generation Register  [0x00]

#define HDMI_An_Seed_Sel        0x0E48u // (R/W) An seed selection register [0xFF]
#define HDMI_An_Seed_0          0x0E58u // (R/W) An seed value register [0x00]
#define HDMI_An_Seed_1          0x0E5Cu // (R/W) An seed value register [0x00]
#define HDMI_An_Seed_2          0x0E60u // (R/W) An seed value register [0x00]
#define HDMI_An_Seed_3          0x0E64u // (R/W) An seed value register [0x00]

#define HDMI_RGB_ROUND_EN       0xD500u // (R/W) Specifies round enable for 8/10 bit R/G/B in video_receiver  [0x00]

#define HDMI_VACT_SPACE_R_0     0xD504u // (R/W) Specifies vertical active space R  [0x00]
#define HDMI_VACT_SPACE_R_1     0xD508u // (R/W) Specifies vertical active space R  [0x00]
#define HDMI_VACT_SPACE_G_0     0xD50Cu // (R/W) Specifies vertical active space G  [0x00]
#define HDMI_VACT_SPACE_G_1     0xD510u // (R/W) Specifies vertical active space G  [0x00]
#define HDMI_VACT_SPACE_B_0     0xD514u // (R/W) Specifies vertical active space B  [0x00]
#define HDMI_VACT_SPACE_B_1     0xD518u // (R/W) Specifies vertical active space B  [0x00]

#define HDMI_BLUE_SCREEN_R_0    0xD520u // (R/W) Specifies R Pixel values for blue screen [3:0]  [0x00]
#define HDMI_BLUE_SCREEN_R_1    0xD524u // (R/W) Specifies R Pixel values for blue screen [11:4]  [0x00]
#define HDMI_BLUE_SCREEN_G_0    0xD528u // (R/W) Specifies G Pixel values for blue screen [3:0]  [0x00]
#define HDMI_BLUE_SCREEN_G_1    0xD52Cu // (R/W) Specifies G Pixel values for blue screen [11:4]  [0x00]
#define HDMI_BLUE_SCREEN_B_0    0xD530u // (R/W) Specifies B Pixel values for blue screen [3:0]  [0x00]
#define HDMI_BLUE_SCREEN_B_1    0xD534u // (R/W) Specifies B Pixel values for blue screen [11:4]  [0x00]

//------------------------------------------------------------------------------
// MARK: - Timing Generator Registers (TG_BASE) -
//------------------------------------------------------------------------------

#define TG_VACT_ST3_L           0x0068u // (R/W) Specifies the HDMI vertical active start position for 3rd bottom field. [0x7B]
#define TG_VACT_ST3_H           0x006Cu // (R/W) Specifies the HDMI vertical active start position for 3rd bottom field. [0x04]

#define TG_VACT_ST4_L           0x0070u // (R/W) Specifies the HDMI vertical active start position for 4th bottom field. [0xAE]
#define TG_VACT_ST4_H           0x0074u // (R/W) Specifies the HDMI vertical active start position for 4th bottom field. [0x06]

#define TG_3D_FP                0x00F0u // (R/W) Specifies the Stereoscopy timing enable [0x00]

//------------------------------------------------------------------------------
// MARK: - I2C Bridge / HDMI PHY Registers (I2C_BASE) -
//------------------------------------------------------------------------------

// Note: Most IIC definitions based on AppleS5L8900XI2C/AppleS5L8747XI2C, which is the same controller

#define IICCON                  0x0000u // (R/W) Control register [0x0000_0000]
    #define kIICConAckGen           (0x00000080)
    #define kIICConBusHold          (0x00000010)

#define IICSTAT                 0x0004u // (R/W) Control/status register [0x0000_0000]
    #define kIICStatMaster          (0x00000080)
    #define kIICStatTx              (0x00000040)
    #define kIICStatBB              (0x00000020)
    #define kIICStatSOE             (0x00000010)
    #define kIICStatLRB             (0x00000001)

#define IICADD                  0x0008u // (R/W) Bus address register [0x0000_0000]
#define IICDS                   0x000Cu // (R/W) Transmit/receive data shift register [0x0000_0000]
#define FIFOCON                 0x0018u // (R/W) Tx/Rx FIFO control register [0x0000_0000]
#define FIFOSTAT                0x001Cu // (R/W) Tx/Rx FIFO status register [0x0000_0000]

#define IICINT                  0x0020u // (R/W) Interrupt status register [0x0000_X000]
    #define kIICIntAll              (0x00003F00u)
    #define kIICIntStop             (0x00002000u)
    #define kIICIntStart            (0x00001000u)
    #define kIICIntTx               (0x00000800u)
    #define kIICIntRx               (0x00000400u)
    #define kIICIntTimeOut          (0x00000200u)
    #define kIICIntBusHold          (0x00000100u)

#define IICVER                  0x0024u // (R/O) Version register [0x0000_0007]
#define SW_RESET                0x002Cu // (R/W) Software reset register [0x0000_0000]
    #define kIICSwReset             (0x00000001)

#define IICBUSCON               0x0030u // (R/W) Bus control register [0x0000_0000]
#define TIMEOUT_CON             0x0034u // (R/W) Internal timer setting register [0xFFFF_FFFF]

#define HDMIPHY_ID              0x0080u // (R/W) I2C device ID of HDMI PHY [0x0000_0000]

#define HDMIPHYCON_STAT         0x0084u // (R/O) Status of write operation to HDMIPHYCON0~8 [0x0000_0000]
    #define HDMIPHYCON_STAT_IN_PROGRESS             0x1

#define HDMIPHYCON0             0x80088 // (R/W) Reg00~Reg03 of HDMI PHY [0x101F_9101]
#define HDMIPHYCON1             0x8008C // (R/W) Reg04~Reg07 of HDMI PHY [0x08EF_5B40]
#define HDMIPHYCON2             0x80090 // (R/W) Reg08~Reg0B of HDMI PHY [0xD8B9_2081]
#define HDMIPHYCON3             0x80094 // (R/W) Reg0C~Reg0F of HDMI PHY [0x80AC_A045]
#define HDMIPHYCON4             0x80098 // (R/W) Reg10~Reg13 of HDMI PHY [0x8412_8008]
#define HDMIPHYCON5             0x8009C // (R/W) Reg14~Reg17 of HDMI PHY [0x8624_2408]
#define HDMIPHYCON6             0x800A0 // (R/W) Reg18~Reg1B of HDMI PHY [0x0124_A654]
#define HDMIPHYCON7             0x800A4 // (R/W) Reg1C~Reg1E of HDMI PHY [0x0001_0000]
#define HDMIPHYCON8             0x800A8 // (R/W) Reg1F of HDMI PHY [0x0000_0080]

#endif /* !_REGS_H */
