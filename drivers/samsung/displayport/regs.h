/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Registers for Display Port General Control
#define	DPTX_DP_TX_VERSION                              0x0010

#define	DPTX_FUNC_EN_1                                  0x0018

    #define DPTX_FUNC_EN_1_VIDCAP               (0x1<<6)
    #define DPTX_FUNC_EN_1_VIDFIF0              (0x1<<5)
    #define DPTX_FUNC_EN_1_AUDFIF0              (0x1<<4)
    #define DPTX_FUNC_EN_1_AUDCAP               (0x1<<3)
    #define DPTX_FUNC_EN_1_HDCP                 (0x1<<2)
    #define DPTX_FUNC_EN_1_SW_ALL               (0x1<<0)

#define	DPTX_FUNC_EN_2                                  0x001C
    #define DPTX_FUNC_EN_2_SSC                  (0x1<<7)
    #define DPTX_FUNC_EN_2_AUX                  (0x1<<2)
    #define DPTX_FUNC_EN_2_SERDES_FIFO          (0x1<<1)
    #define DPTX_FUNC_EN_2_LS_CLK_DOMAIN        (0x1<<0)

#define	DPTX_VIDEO_CTL_1                                0x0020
    #define DPTX_VIDEO_CTL_1_VIDEO_EN_SHIFT     (7)
    #define DPTX_VIDEO_CTL_1_VIDEO_EN           (0x1<<DPTX_VIDEO_CTL_1_VIDEO_EN_SHIFT)
    #define DPTX_VIDEO_CTL_1_VIDEO_MUTE_SHIFT   (6)
    #define DPTX_VIDEO_CTL_1_VIDEO_MUTE         (0x1<<DPTX_VIDEO_CTL_1_VIDEO_MUTE_SHIFT)

#define	DPTX_VIDEO_CTL_2                                0x0024
    #define DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT   (7)
    #define DPTX_VIDEO_CTL_2_IN_D_RANGE_MASK    (0x1<<DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_D_RANGE_VESA    (0x0<<DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_D_RANGE_CEA     (0x1<<DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT)
    
    #define DPTX_VIDEO_CTL_2_IN_BPC_SHIFT       (4)
    #define DPTX_VIDEO_CTL_2_IN_BPC_MASK        (0x3<<DPTX_VIDEO_CTL_2_IN_BPC_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_BPC_12BIT       (0x3<<DPTX_VIDEO_CTL_2_IN_BPC_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_BPC_10BIT       (0x2<<DPTX_VIDEO_CTL_2_IN_BPC_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_BPC_8BIT        (0x1<<DPTX_VIDEO_CTL_2_IN_BPC_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_BPC_6BIT        (0x0<<DPTX_VIDEO_CTL_2_IN_BPC_SHIFT)
    
    #define DPTX_VIDEO_CTL_2_IN_COLOR_F_SHIFT       (0)
    #define DPTX_VIDEO_CTL_2_IN_COLOR_F_MASK        (0x3<<DPTX_VIDEO_CTL_2_IN_COLOR_F_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_COLOR_F_YCbCr444    (0x2<<DPTX_VIDEO_CTL_2_IN_COLOR_F_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_COLOR_F_YCbCr22     (0x1<<DPTX_VIDEO_CTL_2_IN_COLOR_F_SHIFT)
    #define DPTX_VIDEO_CTL_2_IN_COLOR_F_RGB         (0x0<<DPTX_VIDEO_CTL_2_IN_COLOR_F_SHIFT)

#define	DPTX_VIDEO_CTL_3                                0x0028
    #define DPTX_VIDEO_CTL_3_IN_YC_COEFFI_SHIFT     (7)
    #define DPTX_VIDEO_CTL_3_IN_YC_COEFFI_MASK      (0x1<<DPTX_VIDEO_CTL_3_IN_YC_COEFFI_SHIFT)
    #define DPTX_VIDEO_CTL_3_IN_YC_COEFFI_ITU709    (0x1<<DPTX_VIDEO_CTL_3_IN_YC_COEFFI_SHIFT)
    #define DPTX_VIDEO_CTL_3_IN_YC_COEFFI_ITU601    (0x0<<DPTX_VIDEO_CTL_3_IN_YC_COEFFI_SHIFT)

#define	DPTX_VIDEO_CTL_4                                0x002C
    #define DPTX_VIDEO_CTL_4_BIST_EN                (0x1<<3)
    
    #define DPTX_VIDEO_CTL_4_BIST_WIDTH_SHIFT       (2)
    #define DPTX_VIDEO_CTL_4_BIST_WIDTH_MASK        (0x1<<DPTX_VIDEO_CTL_4_BIST_WIDTH_SHIFT)
    #define DPTX_VIDEO_CTL_4_BIST_WIDTH_64          (0x1<<DPTX_VIDEO_CTL_4_BIST_WIDTH_SHIFT)
    #define DPTX_VIDEO_CTL_4_BIST_WIDTH_32          (0x0<<DPTX_VIDEO_CTL_4_BIST_WIDTH_SHIFT)
    
    #define DPTX_VIDEO_CTL_4_BIST_TYPE_SHIFT        (0)
    #define DPTX_VIDEO_CTL_4_BIST_TYPE_MASK         (0x3<<DPTX_VIDEO_CTL_4_BIST_TYPE_SHIFT)
    #define DPTX_VIDEO_CTL_4_BIST_TYPE_COLOR_BAR    (0x0<<DPTX_VIDEO_CTL_4_BIST_TYPE_SHIFT)
    #define DPTX_VIDEO_CTL_4_BIST_TYPE_W_G_B_BAR    (0x1<<DPTX_VIDEO_CTL_4_BIST_TYPE_SHIFT)
    #define DPTX_VIDEO_CTL_4_BIST_TYPE_MOBILE_WHITE (0x2<<DPTX_VIDEO_CTL_4_BIST_TYPE_SHIFT)

#define	DPTX_VIDEO_CTL_8                                0x003C
#define	DPTX_VIDEO_CTL_10                               0x0044
    #define DPTX_VIDEO_CTL_10_F_SEL             (0x1<<4)

    #define DPTX_VIDEO_CTL_10_SLAVE_I_SCAN_CFG  (0x1<<2)
    #define DPTX_VIDEO_CTL_10_VSYNC_P_CFG       (0x1<<1)
    #define DPTX_VIDEO_CTL_10_HSYNC_P_CFG       (0x1<<0)

#define	DPTX_TOTAL_LINE_CFG_L                           0x0048
#define	DPTX_TOTAL_LINE_CFG_H                           0x004C
#define	DPTX_ACTIVE_LINE_CFG_L                          0x0050
#define	DPTX_ACTIVE_LINE_CFG_H                          0x0054
#define	DPTX_V_F_PORCH_CFG                              0x0058
#define	DPTX_V_SYNC_WIDTH_CFG                           0x005C
#define	DPTX_V_B_PORCH_CFG                              0x0060
#define	DPTX_TOTAL_PIXEL_CFG_L                          0x0064
#define	DPTX_TOTAL_PIXEL_CFG_H                          0x0068
#define	DPTX_ACTIVE_PIXEL_CFG_L                         0x006C
#define	DPTX_ACTIVE_PIXEL_CFG_H                         0x0070
#define	DPTX_H_F_PORCH_CFG_L                            0x0074
#define	DPTX_H_F_PORCH_CFG_H                            0x0078
#define	DPTX_H_SYNC_CFG_L                               0x007C
#define	DPTX_H_SYNC_CFG_H                               0x0080
#define	DPTX_H_B_PORCH_CFG_L                            0x0084
#define	DPTX_H_B_PORCH_CFG_H                            0x0088
#define	DPTX_VIDEO_STATUS                               0x008C
#define	DPTX_TOTAL_LINE_STA_L                           0x0090
#define	DPTX_TOTAL_LINE_STA_H                           0x0094
#define	DPTX_ACTIVE_LINE_STA_L                          0x0098
#define	DPTX_ACTIVE_LINE_STA_H                          0x009C
#define	DPTX_V_F_PORCH_STA                              0x00A0
#define	DPTX_V_SYNC_STA                                 0x00A4
#define	DPTX_V_B_PORCH_STA                              0x00A8
#define	DPTX_TOTAL_PIXEL_STA_L                          0x00AC
#define	DPTX_TOTAL_PIXEL_STA_H                          0x00B0
#define	DPTX_ACTIVE_PIXEL_STA_L                         0x00B4
#define	DPTX_ACTIVE_PIXEL_STA_H                         0x00B8
#define	DPTX_H_F_PORCH_STA_L                            0x00BC
#define	DPTX_H_F_PORCH_STA_H                            0x00C0
#define	DPTX_H_SYNC_STA_L                               0x00C4
#define	DPTX_H_SYNC_STA_H                               0x00C8
#define	DPTX_H_B_PORCH_STA_L                            0x00CC
#define	DPTX_H_B_PORCH_STA_H                            0x00D0
#if DISPLAYPORT_VERSION < 3
#define	DPTX_SPDIF_AUDIO_CTL_0                          0x00D8
    #define DPTX_BF_C_AUD_SPDIF_EN                      (0x1<<7)    //Not Touched: For Audio Slave mode
    #define DPTX_BF_C_FORCE_SPDIF_DET                   (0x1<<2)    //Not Touched: Test Purpose Only
    #define DPTX_BF_P_SPDIF_PARITY_CTRL                 (0x1<<1)    //Not Touched: Test Purpose Only
    #define DPTX_BF_P_SPDIF_CLK_DET_RESET_BYPASS        (0x1<<0)    //Not Touched: Diffculty of MF code

#define	DPTX_DP_AUDIO_CTL_1                             0x00DC
#define	DPTX_SPDIF_AUDIO_STA_0                          0x00E0
#define	DPTX_SPDIF_AUDIO_STA_1                          0x00E4
#define	DPTX_SPDIF_ERR_THRD                             0x00E8
#define	DPTX_SPDIF_ERR_CNT                              0x00EC
#define	DPTX_AUDIO_BIST_CTL                             0x00F0
#define	DPTX_AUD_FREQ_CNT_1                             0x00F4
#define	DPTX_AUD_FREQ_CNT_2                             0x00F8
#endif //DISPLAYPORT_VERSION

#define	DPTX_AVI_DB1                                    0x01D0
#define	DPTX_AVI_DB2                                    0x01D4
#define	DPTX_AVI_DB3                                    0x01D8
#define	DPTX_AVI_DB4                                    0x01DC
#define	DPTX_AVI_DB5                                    0x01E0
#define	DPTX_AVI_DB6                                    0x01E4
#define	DPTX_AVI_DB7                                    0x01E8
#define	DPTX_AVI_DB8                                    0x01EC
#define	DPTX_AVI_DB9                                    0x01F0
#define	DPTX_AVI_DB10                                   0x01F4
#define	DPTX_AVI_DB11                                   0x01F8
#define	DPTX_AVI_DB12                                   0x01FC
#define	DPTX_AVI_DB13                                   0x0200

#if DISPLAYPORT_VERSION < 3
#define	DPTX_AUDIO_DB1                                  0x021C
#define	DPTX_AUDIO_DB2                                  0x0220
#define	DPTX_AUDIO_DB3                                  0x0224
#define	DPTX_AUDIO_DB4                                  0x0228
#define	DPTX_AUDIO_DB5                                  0x022C
#define	DPTX_AUDIO_DB6                                  0x0230
#define	DPTX_AUDIO_DB7                                  0x0234
#define	DPTX_AUDIO_DB8                                  0x0238
#define	DPTX_AUDIO_DB9                                  0x023C
#define	DPTX_AUDIO_DB10                                 0x0240
#endif //DISPLAYPORT_VERSION

#define	DPTX_IF_TYPE                                    0x0244

#define	DPTX_IF_PKT_DB1                                 0x0254
#define	DPTX_IF_PKT_DB2                                 0x0258
#define	DPTX_IF_PKT_DB3                                 0x025C
#define	DPTX_IF_PKT_DB4                                 0x0260
#define	DPTX_IF_PKT_DB5                                 0x0264
#define	DPTX_IF_PKT_DB6                                 0x0268
#define	DPTX_IF_PKT_DB7                                 0x026C
#define	DPTX_IF_PKT_DB8                                 0x0270
#define	DPTX_IF_PKT_DB9                                 0x0274
#define	DPTX_IF_PKT_DB10                                0x0278
#define	DPTX_IF_PKT_DB11                                0x027C
#define	DPTX_IF_PKT_DB12                                0x0280
#define	DPTX_IF_PKT_DB13                                0x0284
#define	DPTX_IF_PKT_DB14                                0x0288
#define	DPTX_IF_PKT_DB15                                0x028C
#define	DPTX_IF_PKT_DB16                                0x0290
#define	DPTX_IF_PKT_DB17                                0x0294
#define	DPTX_IF_PKT_DB18                                0x0298
#define	DPTX_IF_PKT_DB19                                0x029C
#define	DPTX_IF_PKT_DB20                                0x02A0
#define	DPTX_IF_PKT_DB21                                0x02A4
#define	DPTX_IF_PKT_DB22                                0x02A8
#define	DPTX_IF_PKT_DB23                                0x02AC
#define	DPTX_IF_PKT_DB24                                0x02B0
#define	DPTX_IF_PKT_DB25                                0x02B4

#define	DPTX_MPEG_DB1                                	0x02D0
#define	DPTX_MPEG_DB2                                	0x02D4
#define	DPTX_MPEG_DB3                                	0x02D8
#define	DPTX_MPEG_DB4                                	0x02DC
#define	DPTX_MPEG_DB5                                	0x02E0
#define	DPTX_MPEG_DB6                                	0x02E4
#define	DPTX_MPEG_DB7                                	0x02E8
#define	DPTX_MPEG_DB8                                	0x02EC
#define	DPTX_MPEG_DB9                                	0x02F0
#define	DPTX_MPEG_DB10                                	0x02F4
#if DISPLAYPORT_VERSION < 3
#define	DPTX_AUDIO_BIST_CH_STA1                         0x0340
#define	DPTX_AUDIO_BIST_CH_STA2                         0x0344
#define	DPTX_AUDIO_BIST_CH_STA3                         0x0348
#define	DPTX_AUDIO_BIST_CH_STA4                         0x034C
#define	DPTX_AUDIO_BIST_CH_STA5                         0x0350
#endif //DISPLAYPORT_VERSION

#define	DPTX_LANE_MAP                                	0x035C
    #define	DPTX_LANE_MAP_LANE1_SHIFT       2
    #define	DPTX_LANE_MAP_LANE0_SHIFT       0


#define	DPTX_ANALOG_CTL_1                               0x0370
#if DISPLAYPORT_VERSION >= 3
    #define DPTX_ANALOG_CTL_1_SEL_BG                    (1<<6)
#endif //DISPLAYPORT_VERSION

    #define DPTX_ANALOG_CTL_1_TX_TERM_CTL_SHIFT         4
    #define DPTX_ANALOG_CTL_1_TX_TERM_CTL_73_OHM_VALUE  0
    #define DPTX_ANALOG_CTL_1_TX_TERM_CTL_50_OHM_VALUE  1
    #define DPTX_ANALOG_CTL_1_TX_TERM_CTL_61_OHM_VALUE  2
    #define DPTX_ANALOG_CTL_1_TX_TERM_CTL_45_OHM_VALUE  3
    
    #define DPTX_ANALOG_CTL_1_TX_SWING_INC_30PER        (1<<3)

    #define DPTX_ANALOG_CTL_1_TEST_BIT_MASK             (0x7)
    #define DPTX_ANALOG_CTL_1_TEST_BIT_SHIFT            (0)
    
#define	DPTX_ANALOG_CTL_2                               0x0374
    #define	DPTX_ANALOG_CTL_2_SEL_CLK_24M   (1<<3)

    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_MASK      (0x7)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT     (0)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_DVDD      (0<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_0_875V    (1<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_0_9375V   (2<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_1_000V    (3<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_1_0625V   (4<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_1_125V    (5<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_1_1875V   (6<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    #define DPTX_ANALOG_CTL_2_TX_DVDD_BIT_1_250V    (7<<DPTX_ANALOG_CTL_2_TX_DVDD_BIT_SHIFT)
    
#define	DPTX_ANALOG_CTL_3                               0x0378
    #define	DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT (0)
    #define	DPTX_ANALOG_CTL_3_VCO_000uA     (0x00<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_200uA     (0x01<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_300uA     (0x02<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_400uA     (0x03<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_500uA     (0x04<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_600uA     (0x05<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_700uA     (0x06<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_VCO_900uA     (0x07<<DPTX_ANALOG_CTL_3_VCO_BIT_SHIFT)
    
    #define	DPTX_ANALOG_CTL_3_SEL_CUR_SHIFT         (3)
    #define	DPTX_ANALOG_CTL_3_SEL_CUR_LPP_LC_AVG    (0x00<<DPTX_ANALOG_CTL_3_SEL_CUR_SHIFT)
    #define	DPTX_ANALOG_CTL_3_SEL_CUR_LC            (0x01<<DPTX_ANALOG_CTL_3_SEL_CUR_SHIFT)
    #define	DPTX_ANALOG_CTL_3_SEL_CUR_LPP           (0x02<<DPTX_ANALOG_CTL_3_SEL_CUR_SHIFT)
    #define	DPTX_ANALOG_CTL_3_SEL_CUR_LPP_LC        (0x03<<DPTX_ANALOG_CTL_3_SEL_CUR_SHIFT)
    
    #define	DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT  (5)
    #define	DPTX_ANALOG_CTL_3_DRIVE_DVDD            (0x00<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_0_8750V         (0x01<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_0_9375V         (0x02<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_1_1000V         (0x03<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_1_0625V         (0x04<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_1_1250V         (0x05<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_1_1875V         (0x06<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)
    #define	DPTX_ANALOG_CTL_3_DRIVE_1_2500V         (0x07<<DPTX_ANALOG_CTL_3_DRIVE_DVDD_BIT_SHIFT)


#define	DPTX_PLL_FILTER_CTL_1                           0x037C
    #define	DPTX_PLL_FILTER_CTL_1_PD_RING_OSC_PWR_DWN   (0x1<<6)
    
    #define	DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_SHIFT    (4)
    #define	DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_200_OHM  (0x0<<DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_102_OHM  (0x1<<DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_69_OHM   (0x2<<DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_52_OHM   (0x3<<DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_SHIFT)

    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_SHIFT     (2)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_1         (0x0<<DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_2         (0x1<<DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_3         (0x2<<DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_SHIFT)

    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_SHIFT          (0)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_1mA            (0x0<<DPTX_PLL_FILTER_CTL_1_TX_CUR_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_2mA            (0x1<<DPTX_PLL_FILTER_CTL_1_TX_CUR_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_3mA            (0x2<<DPTX_PLL_FILTER_CTL_1_TX_CUR_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_1_TX_CUR_4mA            (0x3<<DPTX_PLL_FILTER_CTL_1_TX_CUR_SHIFT)
    
#if DISPLAYPORT_VERSION < 3
#define	DPTX_PLL_FILTER_CTL_2                           0x0380
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT         (4)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_NEG_120mV     (0x0<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_NEG_80mV      (0x1<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_NEG_40mV      (0x2<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_0mV           (0x3<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_40mV          (0x4<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_80mV          (0x5<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_120mV         (0x6<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH1_AMP_160mV         (0x7<<DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT)

    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT         (0)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_NEG_120mV     (0x0<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_NEG_80mV      (0x1<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_NEG_40mV      (0x2<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_0mV           (0x3<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_40mV          (0x4<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_80mV          (0x5<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_120mV         (0x6<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_160mV         (0x7<<DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT)
    
    #define	DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT         (0)
#endif //DISPLAYPORT_VERSION
    
#define DPTX_AUX_HW_RETRY_CTL                           0x0390

    #define     DPTX_AUX_HW_RETRY_BIT_PERIOD_DELAY_SHIFT    (8)
    #define     DPTX_AUX_HW_RETRY_BIT_PERIOD_DELAY_MASK     (0x7)

    #define     DPTX_AUX_HW_RETRY_INTERVAL_SHIFT            (3)
    #define     DPTX_AUX_HW_RETRY_INTERVAL_600us            (0x00<<DPTX_AUX_HW_RETRY_INTERVAL_SHIFT)
    #define     DPTX_AUX_HW_RETRY_INTERVAL_800us            (0x01<<DPTX_AUX_HW_RETRY_INTERVAL_SHIFT)
    #define     DPTX_AUX_HW_RETRY_INTERVAL_1000us           (0x02<<DPTX_AUX_HW_RETRY_INTERVAL_SHIFT)
    #define     DPTX_AUX_HW_RETRY_INTERVAL_1800us           (0x03<<DPTX_AUX_HW_RETRY_INTERVAL_SHIFT)

    #define     DPTX_AUX_HW_RETRY_COUNT_SHIFT               (0)
    #define     DPTX_AUX_HW_RETRY_COUNT_MASK                (0x7)

#define	DPTX_INT_STATE                                	0x03C0
#define	DPTX_COMMON_INT_STA_1                           0x03C4
    #define	DPTX_COMMON_INT_1_VSYNC_DET     (0x1<<7)
    #define	DPTX_COMMON_INT_1_PLL_LCK_CHG   (0x1<<6)
    #define	DPTX_COMMON_INT_1_SPDIF_ERR     (0x1<<5)
    #define	DPTX_COMMON_INT_1_SPDIF_UNSTBL  (0x1<<4)
    #define	DPTX_COMMON_INT_1_VID_FMT_CHG   (0x1<<3)
    #define	DPTX_COMMON_INT_1_AUD_CLK_CHG   (0x1<<2)
    #define	DPTX_COMMON_INT_1_VID_CLK_CHG   (0x1<<1)
    #define	DPTX_COMMON_INT_1_VSW_INT       (0x1<<0)
    
#if DISPLAYPORT_VERSION < 3
#define	DPTX_COMMON_INT_STA_2                           0x03C8
    #define	DPTX_COMMON_INT_2_HDCP_CHG      (0x1<<6)
    #define	DPTX_COMMON_INT_2_HW_BKSV_RDY   (0x1<<3)
    #define	DPTX_COMMON_INT_2_HW_SHA_DONE   (0x1<<2)
    #define	DPTX_COMMON_INT_2_HW_AUTH_CHG   (0x1<<1)
    #define	DPTX_COMMON_INT_2_HW_AUTH_DONE  (0x1<<0)
    
#define	DPTX_COMMON_INT_STA_3                           0x03CC
    #define	DPTX_COMMON_INT_3_AFIFO_UNDER   (0x1<<7)
    #define	DPTX_COMMON_INT_3_AFIFO_OVER    (0x1<<6)
    #define	DPTX_COMMON_INT_3_R0_CHK        (0x1<<5)
#endif //DISPLAYPORT_VERSION

#define	DPTX_COMMON_INT_STA_4                           0x03D0
    #define	DPTX_COMMON_INT_4_SPDIF_BI_PHASE_ERR    (0x1<<5)
    #define	DPTX_COMMON_INT_4_HP_CHG                (0x1<<2)
    #define	DPTX_COMMON_INT_4_HPD_LOST              (0x1<<1)
    #define	DPTX_COMMON_INT_4_PLUG                  (0x1<<0)

#define	DPTX_DP_INT_STA                                 0x03DC
    #define	DPTX_DP_INT_STA_INT_HPD         (0x1<<6)
    #define	DPTX_DP_INT_STA_HW_LINK_TR_FIN  (0x1<<5)
    #define	DPTX_DP_INT_STA_RPLY_RCV        (0x1<<1)
    #define	DPTX_DP_INT_STA_AUX_ERR         (0x1<<0)

#define	DPTX_COMMON_INT_MASK_1                          0x03E0
#if DISPLAYPORT_VERSION < 3
#define	DPTX_COMMON_INT_MASK_2                          0x03E4
#define	DPTX_COMMON_INT_MASK_3                          0x03E8
#endif //DISPLAYPORT_VERSION

#define	DPTX_COMMON_INT_MASK_4                          0x03EC
    
#define	DPTX_DP_INT_STA_MASK                            0x03F8
#define	DPTX_INT_CTL                                	0x03FC
    #define DPTX_INT_SET_SOFT_INT           (1<<2)
    #define DPTX_INT_POLARITY_HIGH          (1<<0)

// Registers for HDCP Function
#if DISPLAYPORT_VERSION < 3
#define	DPTX_HDCP_STA                                	0x0400
    #define DPTX_HDCP_STA_BKSV_VALID        (0x1<<3)
    #define DPTX_HDCP_STA_ENCRYPT           (0x1<<2)
    #define DPTX_HDCP_STA_HW_AUTHEN_PASS    (0x1<<1)
    #define DPTX_HDCP_STA_AKSV_VALID        (0x1<<0)

#define	DPTX_HDCP_CTL_0                                 0x0404
    #define DPTX_HDCP_CTL_0_SW_STORE_AN             (0x1<<7)
    #define DPTX_HDCP_CTL_0_SW_RX_REPEATER          (0x1<<6)
    #define DPTX_HDCP_CTL_0_HW_RE_AUTHEN            (0x1<<5)
    #define DPTX_HDCP_CTL_0_SW_AUTH_OK              (0x1<<4)
    #define DPTX_HDCP_CTL_0_HW_AUTH_EN              (0x1<<3)
    #define DPTX_HDCP_CTL_0_HDCP_ENC_EN             (0x1<<2)
    #define DPTX_HDCP_CTL_0_HW_1ST_PART_ATHENTICATION_EN            (0x1<<1)
    #define DPTX_HDCP_CTL_0_HW_2ND_PART_ATHENTICATION_EN            (0x1<<0)

#define	DPTX_AKSV0                                      0x0414
#define	DPTX_AKSV1                                      0x0418
#define	DPTX_AKSV2                                      0x041C
#define	DPTX_AKSV3                                      0x0420
#define	DPTX_AKSV4                                      0x0424
#define	DPTX_AN0                                        0x0428
#define	DPTX_AN1                                        0x042C
#define	DPTX_AN2                                        0x0430
#define	DPTX_AN3                                        0x0434
#define	DPTX_AN4                                        0x0438
#define	DPTX_AN5                                        0x043C
#define	DPTX_AN6                                        0x0440
#define	DPTX_AN7                                        0x0444
#define	DPTX_BKSV0                                      0x0448
#define	DPTX_BKSV1                                      0x044C
#define	DPTX_BKSV2                                      0x0450
#define	DPTX_BKSV3                                      0x0454
#define	DPTX_BKSV4                                      0x0458
#define	DPTX_RI0                                        0x045C
#define	DPTX_RI1                                        0x0460
#define	DPTX_RX_CAPS                                	0x0468
#define	DPTX_RX_BSTATUS_0                               0x046C
#define	DPTX_RX_BSTATUS_1                               0x0470
#define	DPTX_SKIP_RPT_ZERO_DEV                          0x0474
#define	DPTX_HDCP_KEY_STA                               0x0478
#define	DPTX_HDCP_KEY_CMD                               0x047C
#define	DPTX_SPSRAM_CFG_1                               0x0488
#define     DPTX_SPSRAM_CFG_1_KEY_MODE                       (0x1<<6)
#define     DPTX_SPSRAM_CFG_1_SRAM_RW_DONE                   (0x1<<5)
#define     DPTX_SPSRAM_CFG_1_SRAM_WRITE_EN                  (0x1<<2)
#define     DPTX_SPSRAM_CFG_1_SRAM_READ_EN                   (0x1<<1)      
#define     DPTX_SPSRAM_CFG_1_SRAM_ADDR8                     (0x1<<0)                 

#define	DPTX_SPSRAM_CFG_2                               0x048C
#define	DPTX_RW_ROM_ADDR                                0x0490
#define	DPTX_RD_DATA                                    0x0494
#define	DPTX_W_DATA                                     0x0498
#define	DPTX_HDCP_FRAME_NUM                             0x04A4
#define	DPTX_HDCP_VID_0                                 0x04A8
#define	DPTX_HDCP_VID_1                                 0x04AC
#define	DPTX_HDCP_VID_2                                 0x04B0
#define	DPTX_HDCP_AM0_0                                 0x04C0
#define	DPTX_HDCP_AM0_1                                 0x04C4
#define	DPTX_HDCP_AM0_2                                 0x04C8
#define	DPTX_HDCP_AM0_3                                 0x04CC
#define	DPTX_HDCP_AM0_4                                 0x04D0
#define	DPTX_HDCP_AM0_5                                 0x04D4
#define	DPTX_HDCP_AM0_6                                 0x04D8
#define	DPTX_HDCP_AM0_7                                 0x04DC
#define	DPTX_WRITE_AKSV_WAIT                            0x0500
#define	DPTX_LINK_CHK_TIMER                             0x0504
#define	DPTX_RPTR_RDY_TIMER                             0x0508
#endif //DISPLAYPORT_VERSION

//Registers for Display Port Functions
#define	DPTX_SYS_CTL_1                                	0x0600
    #define	DPTX_SYS_CTL_1_DET_STA (1<<2)


#define	DPTX_SYS_CTL_2                                	0x0604
    #define	DPTX_SYS_CTL_2_STRM_CHG_STA (1<<2)
    
#define	DPTX_SYS_CTL_3                                	0x0608
    #define DPTX_SYS_CTL_3_HPD_STATUS           (0x1<<6)
    #define DPTX_SYS_CTL_3_F_HPD                (0x1<<5)
    #define DPTX_SYS_CTL_3_HPD_CTRL             (0x1<<4)
    #define DPTX_SYS_CTL_3_HDCP_RDY             (0x1<<3)
    #define DPTX_SYS_CTL_3_STRM_VALID           (0x1<<2)
    #define DPTX_SYS_CTL_3_F_VALID              (0x1<<1)
    #define DPTX_SYS_CTL_3_VALID_CTRL           (0x1<<0)


#define	DPTX_SYS_CTL_4                                	0x060c
    #define DPTX_SYS_CTL_4_FIX_M_AUD            (0x1<<4)
    #define DPTX_SYS_CTL_4_ENHANCED             (0x1<<3)
    #define DPTX_SYS_CTL_4_FIX_M_VID            (0x1<<2)

#define	DPTX_DP_VID_CTL                                 0x0610
#if DISPLAYPORT_VERSION < 3
#define	DPTX_DP_AUD_CTL                                 0x0618
    #define DPTX_BF_C_DP_AUDIO_EN                       (0x1<<0)
#endif //DISPLAYPORT_VERSION

#define	DPTX_PKT_SEND_CTL                               0x0640
    #define DPTX_BF_C_AUDIO_INFO_UP                     (0x1<<7)
    #define DPTX_BF_C_AVI_UP                            (0x1<<6)
    #define DPTX_BF_C_MPEG_UP                           (0x1<<5)
    #define DPTX_BF_C_IF_UP                             (0x1<<4)
    #define DPTX_BF_C_AUDIO_INFOR_EN                    (0x1<<3)
    #define DPTX_BF_C_AVI_EN                            (0x1<<2)
    #define DPTX_BF_C_MPEG_EN                           (0x1<<1)
    #define DPTX_BF_C_IF_EN                             (0x1<<0)

#define	DPTX_DP_HDCP_CTL                                0x0648
    #define DPTX_DP_HDCP_CTL_HW_HDCP_INT        (0x1<<0)

#if DISPLAYPORT_VERSION < 3
#define	DPTX_SPDIF_PHASE1_CTL_0                         0x0650
#define	DPTX_SPDIF_PHASE1_CTL_1                         0x0654
#define	DPTX_SPDIF_PHASE2_CTL_0                         0x0658
#define	DPTX_SPDIF_PHASE2_CTL_1                         0x065C
#define	DPTX_SPDIF_PHASE3_CTL_0                         0x0660
#define	DPTX_SPDIF_PHASE3_CTL_1                         0x0664
#endif //DISPLAYPORT_VERSION

#define	DPTX_LINK_BW_SET                                0x0680
#define	DPTX_LANE_COUNT_SET                             0x0684
#define	DPTX_DP_TRAINING_PTRN_SET                       0x0688
    #define DPTX_DP_TRAINING_PTRN_SET_SCRM_EDP      (1<<9)
    #define	DPTX_DP_TRAINING_PTRN_SET_SCRM_DIS      (1<<5)
    
    #define DPTX_DP_TRAINING_PTRN_SET_LQ_SHIFT          2
    #define DPTX_DP_TRAINING_PTRN_SET_SW_SHIFT		0

    #define DPTX_DP_TRAINING_PTRN_SET_SW_MASK           0x3

#define	DPTX_DP_LN0_LINK_TRAINING_CTL                   0x068C
#define	DPTX_DP_LN1_LINK_TRAINING_CTL                   0x0690
#define DPTX_DP_LN2_LINK_TRAINING_CTL                   0x0694
#define DPTX_DP_LN3_LINK_TRAINING_CTL                   0x0698

#if DISPLAYPORT_VERSION < 3
    #define     DPTX_DP_LN_LINK_TRAINING_CTL_FINE_ADJ_PRE_EMPHASIS_SHIFT (6)


    #define	DPTX_DP_LN_LINK_TRAINING_CTL_MAX_PRE_REACH   (1<<5)
    
    #define	DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT   (3)
    #define DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_MASK    (0x3 << DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT)
    #define	DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_9_5dB   (3<<DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT)
    #define	DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_6_0dB   (2<<DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT)
    #define	DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_3_5dB   (1<<DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT)
    #define	DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_0dB     (0<<DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT)

    #define	DPTX_DP_LN_LINK_TRAINING_CTL_MAX_DRV_REACH   (1<<2)
    
    #define	DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT   (0)
    #define DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_MASK    (0x3 << DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT)
    #define DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_1200mV  (3<<DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT)
    #define DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_800mV   (2<<DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT)
    #define DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_600mV   (1<<DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT)
    #define DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_400mV   (0<<DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT)
#else //DISPLAYPORT_VERSION
    #define LOW_POWER_MODE		1
    #define DPTX_DP_LN_LINK_TRAINING_CTL_PC2_SHIFT	16
    #define DPTX_DP_LN_LINK_TRAINING_CTL_PRE_EMP_SHIFT	8
    #define DPTX_DP_LN_LINK_TRAINING_CTL_SWING_SHIFT	0
    #define DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(_pc2, _pre_emph, _swing) \
	((_pc2 << DPTX_DP_LN_LINK_TRAINING_CTL_PC2_SHIFT) | (_pre_emph << DPTX_DP_LN_LINK_TRAINING_CTL_PRE_EMP_SHIFT) | (_swing << DPTX_DP_LN_LINK_TRAINING_CTL_SWING_SHIFT))

    //Table 5_ Low Swing Mode when SEL_LOWPOWER is 1
    #define	CHX_SWING_150mV_PRE_EMP1_0dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0x1e)
    #define	CHX_SWING_200mV_PRE_EMP1_0dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0x28)
    #define	CHX_SWING_250mV_PRE_EMP1_0dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0x32)
    #define	CHX_SWING_300mV_PRE_EMP1_0dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0x3c)
    #define	CHX_SWING_150mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0xf, 0x26)
    #define	CHX_SWING_200mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x14, 0x32)
    #define	CHX_SWING_250mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x19, 0x3f)
    #define	CHX_SWING_300mV_PRE_EMP1_6dB_PRE_EMP2_0dB		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x1e, 0x4b)
    //Table 6_ Post Cursor 0
    #define	CHX_SWING_400mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0x50)
    #define	CHX_SWING_600mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0x78)
    #define	CHX_SWING_800mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0xa0)
    #define	CHX_SWING_1200mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x0, 0xf0)
    #define	CHX_SWING_400mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x28, 0x64)
    #define	CHX_SWING_600mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x3c, 0x96)
    #define	CHX_SWING_800mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x50, 0xc8)
    #define	CHX_SWING_400mV_PRE_EMP1_6dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x50, 0x78)
    #define	CHX_SWING_600mV_PRE_EMP1_6dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0x78, 0xb4)
    #define	CHX_SWING_400mV_PRE_EMP1_9_5dB_PRE_EMP2_0dB_PC0		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x0, 0xa0, 0xa0)
    //Table 7_ Post Cursor 0_05
    #define	CHX_SWING_400mV_PRE_EMP1_0dB_PRE_EMP2__0_92dB_PC005	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x4, 0x0, 0x4c)
    #define	CHX_SWING_600mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC005		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x6, 0x0, 0x72)
    #define	CHX_SWING_800mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC005		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x8, 0x0, 0x98)
    #define	CHX_SWING_1200mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC005		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0x0, 0xe4)
    #define	CHX_SWING_400mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC005	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x6, 0x28, 0x5e)
    #define	CHX_SWING_600mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC005	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x9, 0x3c, 0x8d)
    #define	CHX_SWING_800mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC005	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0x50, 0xbc)
    #define	CHX_SWING_400mV_PRE_EMP1_6dB_PRE_EMP2_0dB_PC005		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x8, 0x50, 0x70)
    #define	CHX_SWING_600mV_PRE_EMP1_6dB_PRE_EMP2_0dB_PC005		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0x78, 0xa8)
    #define	CHX_SWING_400mV_PRE_EMP1_9_5dB_PRE_EMP2_0dB_PC005	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0xa0, 0x94)
    //Table 8_ Post Cursor 0_1
    #define	CHX_SWING_400mV_PRE_EMP1_0dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x8, 0x0, 0x48)
    #define	CHX_SWING_600mV_PRE_EMP1_0dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0x0, 0x6c)
    #define	CHX_SWING_800mV_PRE_EMP1_0dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x10, 0x0, 0x90)
    #define	CHX_SWING_1200mV_PRE_EMP1_0dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x18, 0x0, 0xd8)
    #define	CHX_SWING_400mV_PRE_EMP1_3_5dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0x28, 0x58)
    #define	CHX_SWING_600mV_PRE_EMP1_3_5dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x12, 0x3c, 0x84)
    #define	CHX_SWING_800mV_PRE_EMP1_3_5dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x18, 0x50, 0xb0)
    #define	CHX_SWING_400mV_PRE_EMP1_6dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x10, 0x50, 0x68)
    #define	CHX_SWING_600mV_PRE_EMP1_6dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x18, 0x78, 0x9c)
    #define	CHX_SWING_400mV_PRE_EMP1_9_5dB_PRE_EMP2__1_94dB_PC01	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x18, 0xa0, 0x88)
    //Table 9_ Post Cursor 0_15
    #define	CHX_SWING_400mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC015		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0xc, 0x0, 0x44)
    #define	CHX_SWING_600mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC015		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x12, 0x0, 0x66)
    #define	CHX_SWING_800mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC015		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x18, 0x0, 0x88)
    #define	CHX_SWING_1200mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC015	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x24, 0x0, 0xcc)
    #define	CHX_SWING_400mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC015	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x12, 0x28, 0x52)
    #define	CHX_SWING_600mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC015	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x1b, 0x3c, 0x7b)
    #define	CHX_SWING_800mV_PRE_EMP1_3_5dB_PRE_EMP2_0dB_PC015	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x24, 0x50, 0xa4)
    #define	CHX_SWING_400mV_PRE_EMP1_6dB_PRE_EMP2_0dB_PC015		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x18, 0x50, 0x60)
    #define	CHX_SWING_600mV_PRE_EMP1_6dB_PRE_EMP2_0dB_PC015		DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x24, 0x78, 0x90)
    #define	CHX_SWING_400mV_PRE_EMP1_9_5dB_PRE_EMP2_0dB_PC015	DPTX_DP_LN_LINK_TRAINING_CTL_GEN_VALUE(0x24, 0xa0, 0x7c)
#endif //DISPLAYPORT_VERSION


#define	DPTX_DP_DN_SPREAD_CTL                           0x069C
    #define DPTX_DP_DN_SPREAD_ENABLE            (0x1<<4)
    #define DPTX_DP_DN_SPREAD_MOD_FREQ_33Khz    (0x1<<0)

#if DISPLAYPORT_VERSION < 3
#define	DPTX_DP_HW_LINK_TRAINING_CTL                    0x06A0
#endif //DISPLAYPORT_VERSION

#define	DPTX_DP_DEBUG_CTL                               0x06C0

    #define DPTX_DP_DEBUG_CTL_PLL_LOCK          (0x1<<4)
    #define DPTX_DP_DEBUG_CTL_F_PLL_LOCK        (0x1<<3)
    #define DPTX_DP_DEBUG_CTL_PLL_LOCK_CTRL     (0x1<<2)


#define	DPTX_HPD_DEGLITCH_L                             0x06C4
#define	DPTX_HPD_DEGLITCH_H                             0x06C8
#define	DPTX_DP_LINK_DEBUG_CTL                          0x06E0

#define	DPTX_M_VID_0                                	0x0700
#define	DPTX_M_VID_1                                	0x0704
#define	DPTX_M_VID_2                                	0x0708
#define	DPTX_N_VID_0                                	0x070C
#define	DPTX_N_VID_1                                	0x0710
#define	DPTX_N_VID_2                                	0x0714
#define	DPTX_M_VID_MON                                	0x0718
#define	DPTX_DP_PLL_CTL                                 0x071C
    #define DPTX_DP_PLL_CTL_PLL_HS2_BIT		    (1<<8)
    #define DPTX_DP_PLL_CTL_PWR_DN                  (1<<7)
    #define DPTX_DP_PLL_CTL_RESET                   (1<<6)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT     (4)
    // For HBR 2.7Gbps
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_125Khz    (0<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_175Khz    (1<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_248Khz    (2<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_305Khz    (3<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    // For HBR 1.62Gbps
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_166Khz    (0<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_234Khz    (1<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_333Khz    (2<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    #define DPTX_DP_PLL_CTL_LOOP_FLT_BAND_406Khz    (3<<DPTX_DP_PLL_CTL_LOOP_FLT_BAND_SHIFT)
    
#define	DPTX_DP_PHY_PD                                	0x0720
    #define	DPTX_DP_PHY_SEL_LOWPOWER	    (1<<6)
    #define	DPTX_DP_PHY_PWR_DWN                 (1<<5)
    #define	DPTX_DP_PHY_AUX_PWR_DWN             (1<<4)
    #define	DPTX_DP_PHY_CH1_PWR_DWN             (1<<1)
    #define	DPTX_DP_PHY_CH0_PWR_DWN             (1<<0)

#define	DPTX_DP_PHY_TEST                                0x0724
    #define	DPTX_DP_PHY_TEST_RST_MACRO          (1<<5)
    #define	DPTX_DP_PHY_TEST_PLL                (1<<4)
    #define	DPTX_DP_PHY_TEST_CH1                (1<<1)
    #define	DPTX_DP_PHY_TEST_CH0                (1<<0)
    
#define	DPTX_DP_VIDEO_FIFO_THRD                         0x0730

#if DISPLAYPORT_VERSION < 3
#define	DPTX_DP_AUDIO_MARGIN                            0x073C
#endif //DISPLAYPORT_VERSION

#define	DPTX_DP_DN_SPREAD_CTL_1                         0x0740
#define	DPTX_DP_DN_SPREAD_CTL_2                         0x0744

#if DISPLAYPORT_VERSION < 3
#define	DPTX_M_AUD_0                                	0x0748
#endif //DISPLAYPORT_VERSION

#define	DPTX_M_AUD_1                                	0x074C
#define	DPTX_M_AUD_2                                	0x0750
#define	DPTX_N_AUD_0                                	0x0754
#define	DPTX_N_AUD_1                                	0x0758
#define	DPTX_N_AUD_2                                	0x075C
#define	DPTX_DP_M_CAL_CTL                               0x0760
    #define DPTX_DP_M_CAL_CTL_AUD_GEN_FILTER_EN (0x1<<3)
    #define DPTX_DP_M_CAL_CTL_VID_GEN_FILTER_EN (0x1<<2)
    #define DPTX_DP_M_CAL_CTL_GEN_CLK_SEL       (0x1<<0)

#define	DPTX_M_VID_GEN_FILTER_TH                        0x0764
#define	DPTX_M_AUD_GEN_FILTER_TH                        0x0778

#define DPTX_AUX_CH_STA                                 0x0780
#define DPTX_AUX_CH_STA_BUSY                (0x1<<4)
#define DPTX_AUX_CH_STA_STATUS_MASK         (0xf)
#define DPTX_AUX_CH_STA_STATUS_SHIFT        (0)
#define DPTX_AUX_CH_STA_SUCCESS             0
#define DPTX_AUX_CH_STA_NACK                1
#define DPTX_AUX_CH_STA_TIMEOUT             2
#define DPTX_AUX_CH_STA_UNKNOWN             3
#define DPTX_AUX_CH_STA_DEFER               4
#define DPTX_AUX_CH_STA_TX_SHORT            5
#define DPTX_AUX_CH_STA_RX_SHORT            6
#define DPTX_AUX_CH_STA_NACK_W_O_M_ERROR    7
#define DPTX_AUX_CH_STA_I2C_NACK_ERROR      8

#define DPTX_AUX_ERROR_NUM                              0x0784

#define DPTX_AUX_CH_DEFER_CTL                           0x0788
    #define DPTX_AUX_CH_DEFER_CTL_EN                (0x1<<7)
    #define DPTX_AUX_CH_DEFER_CTL_DEFER_COUNT_SHIFT (0)
    #define DPTX_AUX_CH_DEFER_CTL_DEFER_COUNT_MASK  (0x7f)

#define DPTX_BUFFER_DATA_CTL                            0x0790
    #define DPTX_BUFFER_DATA_CTL_CLR            (1<<7)
    #define DPTX_BUFFER_DATA_CTL_COUNT_MASK     (0xf)
    
       
#define DPTX_AUX_CH_CTL_1                               0x0794
    #define DPTX_AUX_CH_CTL_1_AUX_LENGTH_MASK       (0xf)
    #define DPTX_AUX_CH_CTL_1_AUX_LENGTH_SHIFT      (0x4)
    #define DPTX_AUX_CH_CTL_1_AUX_TX_COMM_NATIVE    (0x1<<3)
    #define DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT       (0x1<<2)
    #define DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE     (0x0<<0)
    #define DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ      (0x1<<0)

#define DPTX_AUX_ADDR_7_0                               0x0798
#define DPTX_AUX_ADDR_15_8                              0x079C
#define DPTX_AUX_ADDR_19_16                             0x07A0

#define DPTX_AUX_CH_CTL_2                               0x07A4
    #define DPTX_AUX_CH_CTL_2_AUX_PN_INVERT         (1<<2)
    #define DPTX_AUX_CH_CTL_2_AUX_ADDR_ONLY         (1<<1)
    #define DPTX_AUX_CH_CTL_2_AUX_EN                (1<<0)

#define DPTX_BUF_DATA_0                                 0x07C0
    #define DPTX_BUF_DATA_COUNT     16

// Registers for SOC Implementation
#define	DPTX_SOC_GENERAL_CTL                            0x0800
    #define DPTX_SOC_GENERAL_CTL_AUD_M_VALUE_CMP_SPD_MASTER_SHIFT   21
    #define DPTX_SOC_GENERAL_CTL_AUD_DMA_BURST_SEL_SHIFT            19
    #define DPTX_SOC_GENERAL_CTL_AUD_BIT_MAP_TYPE_SHIFT             16
    #define DPTX_SOC_GENERAL_CTL_PCM_SIZE_SHIFT                     13
    #define DPTX_SOC_GENERAL_CTL_AUDIO_MASTER_MODE_EN_SHIFT         9
    #define DPTX_SOC_GENERAL_CTL_AUDIO_MODE_SHIFT                   8
    #define DPTX_SOC_GENERAL_CTL_AUDIO_CH_STATUS_SAME_SHIFT         5
    #define DPTX_SOC_GENERAL_CTL_MASTER_VIDEO_INTERLACE_EN_SHIFT    4
    #define DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_MODE_EN_SHIFT         1
    #define DPTX_SOC_GENERAL_CTL_VIDEO_MODE_SHIFT                   0
    
    
    #define DPTX_SOC_GENERAL_CTL_AUD_M_VALUE_CMP_SPD_MASTER_MASK    (0x3<<DPTX_SOC_GENERAL_CTL_AUD_M_VALUE_CMP_SPD_MASTER_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_AUD_DMA_BURST_SEL_MASK             (0x3<<DPTX_SOC_GENERAL_CTL_AUD_DMA_BURST_SEL_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_AUD_BIT_MAP_TYPE_MASK              (0x3<<DPTX_SOC_GENERAL_CTL_AUD_BIT_MAP_TYPE_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_PCM_SIZE_MASK                      (0x3<<DPTX_SOC_GENERAL_CTL_PCM_SIZE_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_AUDIO_MASTER_MODE_EN_MASK          (0x1<<DPTX_SOC_GENERAL_CTL_AUDIO_MASTER_MODE_EN_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_AUDIO_MODE_MASK                    (0x1<<DPTX_SOC_GENERAL_CTL_AUDIO_MODE_SHIFT)
    
    
    #define DPTX_SOC_GENERAL_CTL_AUDIO_CH_STATUS_SAME       (0x1<<DPTX_SOC_GENERAL_CTL_AUDIO_CH_STATUS_SAME_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_MASTER_VIDEO_INTERLACE_EN  (0x1<<DPTX_SOC_GENERAL_CTL_MASTER_VIDEO_INTERLACE_EN_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_MODE_EN       (0x1<<DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_MODE_EN_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_VIDEO_MODE                 (0x1<<DPTX_SOC_GENERAL_CTL_VIDEO_MODE_SHIFT)

#define	DPTX_H_TOTAL_MASTER                             0x0804
#define	DPTX_V_TOTAL_MASTER                             0x0808
#define	DPTX_H_F_PORCH_MASTER                           0x080C
#define	DPTX_H_B_PORCH_MASTER                           0x0810
#define	DPTX_H_ACTIVE_MASTER                            0x0814
#define	DPTX_V_F_PORCH_MASTER                           0x0818
#define	DPTX_V_B_PORCH_MASTER                           0x081C
#define	DPTX_V_ACTIVE_MASTER                            0x0820
#define	DPTX_M_VID_MASTER                               0x0824
#define	DPTX_N_VID_MASTER                               0x0828
#if DISPLAYPORT_VERSION < 3
#define	DPTX_M_AUD_MASTER                               0x082C
#define	DPTX_N_AUD_MASTER                               0x0830
#define	DPTX_MASTER_AUD_BUFFER_CTL                      0x0834
    #define DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_MASK_SHIFT     16
    #define DPTX_BF_P_AUDIO_CHANNEL_COUNT_SHIFT             13
    #define DPTX_BF_P_AUDIO_BUFFER_LEVEL_SHIFT              7
    #define DPTX_BF_P_AUDIO_BUFFER_THRESHOLD_SHIFT          2
    #define DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_SHIFT          1
    #define DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_EN_SHIFT       0

    #define DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_MASK           (0x01<<DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_MASK_SHIFT)
    #define DPTX_BF_P_AUDIO_CHANNEL_COUNT_MASK              (0x07<<DPTX_BF_P_AUDIO_CHANNEL_COUNT_SHIFT)
    #define DPTX_BF_P_AUDIO_BUFFER_LEVEL_MASK               (0x3f<<DPTX_BF_P_AUDIO_BUFFER_LEVEL_SHIFT)
    #define DPTX_BF_P_AUDIO_BUFFER_THRESHOLD_MASK           (0x1f<<DPTX_BF_P_AUDIO_BUFFER_THRESHOLD_SHIFT)
    #define DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT                (0x01<<DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_SHIFT)
    #define DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_EN             (0x01<<DPTX_BF_P_AUDIO_BUFFER_EMPTY_INT_EN_SHIFT)
    
#define	DPTX_MASTER_AUDIO_DATA_FROM_APB                 0x0838
#define	DPTX_MASTER_AUDIO_PACKET_DATA                   0x083C
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH1_SHIFT 0
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH2_SHIFT 4
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH3_SHIFT 8
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH4_SHIFT 12
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH5_SHIFT 16
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH6_SHIFT 20
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH7_SHIFT 24
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH8_SHIFT 28

    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH1_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH1_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH2_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH2_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH3_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH3_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH4_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH4_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH5_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH5_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH6_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH6_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH7_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH7_SHIFT)
    #define	DPTX_MASTER_AUDIO_PACKET_DATA_CH8_MASK (0xf<<DPTX_MASTER_AUDIO_PACKET_DATA_CH8_SHIFT)

#define	DPTX_MASTER_AUDIO_GP0_STATUS_0                  0x0840
#define	DPTX_MASTER_AUDIO_GP0_STATUS_1                  0x0844
#define	DPTX_MASTER_AUDIO_GP0_STATUS_2                  0x0848
#define	DPTX_MASTER_AUDIO_GP0_STATUS_3                  0x084C
#define	DPTX_MASTER_AUDIO_GP0_STATUS_4                  0x0850
#define	DPTX_MASTER_AUDIO_GP1_STATUS_0                  0x0854
#define	DPTX_MASTER_AUDIO_GP1_STATUS_1                  0x0858
#define	DPTX_MASTER_AUDIO_GP1_STATUS_2                  0x085C
#define	DPTX_MASTER_AUDIO_GP1_STATUS_3                  0x0860
#define	DPTX_MASTER_AUDIO_GP1_STATUS_4                  0x0864

#define	DPTX_MASTER_AUDIO_GP2_STATUS_0                  0x0868
#define	DPTX_MASTER_AUDIO_GP2_STATUS_1                  0x086C
#define	DPTX_MASTER_AUDIO_GP2_STATUS_2                  0x0870
#define	DPTX_MASTER_AUDIO_GP2_STATUS_3                  0x0874
#define	DPTX_MASTER_AUDIO_GP2_STATUS_4                  0x0878

#define	DPTX_MASTER_AUDIO_GP3_STATUS_0                  0x087C
#define	DPTX_MASTER_AUDIO_GP3_STATUS_1                  0x0880
#define	DPTX_MASTER_AUDIO_GP3_STATUS_2                  0x0884
#define	DPTX_MASTER_AUDIO_GP3_STATUS_3                  0x0888
#define	DPTX_MASTER_AUDIO_GP3_STATUS_4                  0x088C
#endif //DISPLAYPORT_VERSION

#define	DPTX_CRC_CON                                    0x0890
#define	DPTX_CRC_RESULT                                 0x0894
#if DISPLAYPORT_VERSION < 3
#define	DPTX_EFUSE_CON                                  0x0898
    #define DPTX_EFUSE_CON_EFUSE_COM                         (0x1<<0)

#define	DPTX_EFUSE_STATUS                               0x089c
    #define DPTX_EFUSE_STATUS_ECC_FAIL                      (0x1<<1)
    #define DPTX_EFUSE_STATUS_EFUSE_BUSY                    (0x1<<0)
#endif //DISPLAYPORT_VERSION

#define DPTX_TRAINIG_DEBUG                              0x0904

//////////////////////////////////////////////////////////////////////
//////////////// version 1+ specific
//////////////////////////////////////////////////////////////////////

#define	DPTX_SW_RESET                                   0x0014
    #define DPTX_SW_RESET_OPERATION         (0x1<<0)

// DPTX_FUNC_EN_1: 0x0018
    #define DPTX_FUNC_EN_1_MASTER           (0x1<<7)
    #define DPTX_FUNC_EN_1_SLAVE            (0x1<<5)
    #define DPTX_FUNC_EN_1_CRC              (0x1<<1)
    
// DPTX_INT_STATE: 0x03C0
    #define	DPTX_INT_STATE_1                (0x1<<1)


// DPTX_INT_CTL: 0x03FC
    #define DPTX_INT_1_POLARITY_HIGH        (1<<1)

// DPTX_DP_TRAINING_PTRN_SET: 0x0688
    #define	DPTX_DP_TRAINING_PTRN_SET_SCRAMBLER_PATTERN_EMBEDDED        (1<<9)
    #define	DPTX_DP_TRAINING_PTRN_SET_HW_LINK_TRAINING_PATTERN_EMBEDDED (1<<8)

// DPTX_SOC_GENERAL_CTL: 0x0800
    #define DPTX_SOC_GENERAL_CTL_AUDIO_DMA_APB_SEL_SHIFT            3
    #define DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_CLK_SEL_SHIFT         2

    #define DPTX_SOC_GENERAL_CTL_AUDIO_DMA_APB_SEL          (0x1<<DPTX_SOC_GENERAL_CTL_AUDIO_DMA_APB_SEL_SHIFT)
    #define DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_CLK_SEL       (0x1<<DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_CLK_SEL_SHIFT)

#if DISPLAYPORT_VERSION < 3
#define	DPTX_COMMON_INT_1_MASK_1                            0x08A0
#define	DPTX_COMMON_INT_1_MASK_2                            0x08A4
#define	DPTX_COMMON_INT_1_MASK_3                            0x08A8
#define	DPTX_COMMON_INT_1_MASK_4                            0x08AC
    
#define	DPTX_DP_INT_1_STA_MASK                              0x08B0
#endif //DISPLAYPORT_VERSION

//////////////////////////////////////////////////////////////////////
//////////////// version 2+ specific
//////////////////////////////////////////////////////////////////////

// DPTX_PLL_FILTER_CTL_2
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT         (8)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_NEG_120mV     (0x0<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_NEG_80mV      (0x1<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_NEG_40mV      (0x2<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_0mV           (0x3<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_40mV          (0x4<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_80mV          (0x5<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_120mV         (0x6<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH2_AMP_160mV         (0x7<<DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT)

    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT         (12)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_NEG_120mV     (0x0<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_NEG_80mV      (0x1<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_NEG_40mV      (0x2<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_0mV           (0x3<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_40mV          (0x4<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_80mV          (0x5<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_120mV         (0x6<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    #define	DPTX_PLL_FILTER_CTL_2_CH3_AMP_160mV         (0x7<<DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT)
    
#define	DPTX_DP_LN2_LINK_TRAINING_CTL                   0x0694
#define	DPTX_DP_LN3_LINK_TRAINING_CTL                   0x0698

// DPTX_DP_PHY
    #define	DPTX_DP_PHY_CH3_PWR_DWN             (1<<3)
    #define	DPTX_DP_PHY_CH2_PWR_DWN             (1<<2)
