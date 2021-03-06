/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_HWREGBASE_H
#define __PLATFORM_SOC_HWREGBASE_H

/* S5L8940X/S5L8942X/S5L8947X Reg Base Defs */

#define AIC_VERSION 			0
#define AIC_INT_COUNT			(192)
#define PMGR_WDG_VERSION		0

#if !SUB_PLATFORM_S5L8947X
#define SDIO_BASE_ADDR			(0x30000000)

#define SHA1_BASE_ADDR			(0x30100000)
#endif

#define SHA2_BASE_ADDR			(0x30200000)

#define FMI0_BASE_ADDR			(0x31200000)
#define FMI1_BASE_ADDR			(0x31300000)
#define FMI2_BASE_ADDR			(0x31400000)
#define FMI3_BASE_ADDR			(0x31500000)
#define FMI_VERSION			(3)
#if !SUB_PLATFORM_S5L8940X
#define FMI_VERSION_MINOR		(1)
#endif // SUB_PLATFORM_S5L8942X

#if !SUB_PLATFORM_S5L8947X
#define SPI0_BASE_ADDR			(0x32000000)
#define SPI1_BASE_ADDR			(0x32100000)
#define SPI2_BASE_ADDR			(0x32200000)
#define SPI3_BASE_ADDR			(0x32300000)
#define SPI4_BASE_ADDR			(0x32400000)
#define SPI_VERSION			(1)
#define SPIS_COUNT			(5)
#else
#define SPI0_BASE_ADDR			(0x32000000)
#define SPI_VERSION			(1)
#define SPIS_COUNT			(1)

#define ADC_BASE_ADDR			(0x32100000)
#endif

#define UART_VERSION			(1)
#define UART0_BASE_ADDR			(0x32500000)
#define UART1_BASE_ADDR			(0x32600000)
#define UART2_BASE_ADDR			(0x32700000)
#define UART3_BASE_ADDR			(0x32800000)
#define UART4_BASE_ADDR			(0x32900000)
#if SUB_PLATFORM_S5L8947X
#define UARTS_COUNT			(5)
#elif SUB_PLATFORM_S5L8942X
#define UART5_BASE_ADDR			(0x32A00000)
#define UARTS_COUNT			(6)
#elif SUB_PLATFORM_S5L8940X
#define UART5_BASE_ADDR			(0x32A00000)
#define UART6_BASE_ADDR			(0x32B00000)
#define UARTS_COUNT			(7)
#endif

#define PKE_BASE_ADDR			(0x33100000)

#define IIC_BASE_ADDR			(0x33200000)
#define IIC_SPACING			(0x00100000)
#define IICS_COUNT			(3)

#define AUDIO_BASE_ADDR			(0x34000000)

#define USBPHY_BASE_ADDR		(0x36000000)
#if SUB_PLATFORM_S5L8940X
#define USBPHY_VERSION			(3)
#else
#define USBPHY_VERSION			(4)
#endif

#define USBOTG_BASE_ADDR		(0x36100000)
#define UPERF_WIDGETS_BASE_ADDR		(0x36E00000)
#define UPERF_PL301_BASE_ADDR		(0x36F00000)

#define CDMA_BASE_ADDR			(0x37000000)
#define CDMA_VERSION			3

#if !SUB_PLATFORM_S5L8947X
#define VENC_BASE_ADDR			(0x38000000)
#endif

#define VDEC_BASE_ADDR			(0x38100000)

#define JPEG_BASE_ADDR			(0x38200000)

#define SCALER_BASE_ADDR		(0x38300000)
#define SCALER_SPACING			(0x00100000)

#define NRT_DART_BASE_ADDR		(0x38B00000)
#define NRT_DART_WIDGETS_BASE_ADDR	(0x38C00000)
#define NRT_DART_PL301_BASE_ADDR	(0x38D00000)
#define NRT_TOP_WIDGETS_BASE_ADDR	(0x38E00000)
#define NRT_TOP_PL301_BASE_ADDR		(0x38F00000)

#define CLCD_BASE_ADDR			(0x39200000)
#define CLCD_VERSION			(0)
#define CLCD_DITHER_BASE_ADDR		(0x39300000)
#define DITHER_VERSION			(1)

#define TVOUT_BASE_ADDR			(0x39400000)

#if !SUB_PLATFORM_S5L8947X
#define DSIM_BASE_ADDR			(0x39500000)
#define DSIM_LANE_COUNT			(4)
#define DSIM_VERSION			(1)
#endif

#define RGBOUT_BASE_ADDR		(0x39600000)

#if !SUB_PLATFORM_S5L8947X
#define DP_BASE_ADDR			(0x39700000)
#define DISPLAYPORT_VERSION		(1)
#else
#define HDMI_BASE_ADDR			(0x39700000)
#define DPBDITHER_BASE_ADDR		(0x39800000)
#endif

#if !SUB_PLATFORM_S5L8947X
#define ISP_BASE_ADDR			(0x3A000000)

#define DISP0_BASE_ADDR			(0x3A100000)
#define CLCD_DISPLAYPIPE_BASE_ADDR	(DISP0_BASE_ADDR)
#endif
#define DISP1_BASE_ADDR			(0x3A200000)
#define RGBOUT_DISPLAYPIPE_BASE_ADDR	(DISP1_BASE_ADDR)
#define DISP_VERSION			(3)

#if !SUB_PLATFORM_S5L8947X
#define RT_DART_BASE_ADDR		(0x3A400000)
#endif
#define RT_TOP_WIDGETS_BASE_ADDR	(0x3A500000)
#define RT_TOP_PL301_BASE_ADDR		(0x3A600000)

#if SUB_PLATFORM_S5L8947X
#define ETHERNET_BASE_ADDR		(0x3B000000)
#else
#define HP_BASE_ADDR			(0x3B000000)
#endif

#if SUB_PLATFORM_S5L8947X
#define AMP_BASE_ADDR			(0x3D000000)
#define AMP_SPACING			(0x00100000)
#else
#define AMG_BASE_ADDR			(0x3D000000)
#define AMG_SPACING			(0x00100000)
#endif

#define PL310_BASE_ADDR			(0x3E000000)
#define SCU_BASE_ADDR			(0x3E100000)

#define PMGR_BASE_ADDR			(0x3F100000)
#define AIC_BASE_ADDR			(0x3F200000)

#define IOP_BASE_ADDR			(0x3F300000)

#define ROSC_BASE_ADDR			(0x3F400000)
#define ROSC_MASK			(0x57FF)
#define ROSC_WITH_CLOCK			(0)

#define CHIPID_BASE_ADDR		(0x3F500000)

#define SWI_BASE_ADDR			(0x3F600000)
#if !SUB_PLATFORM_S5L8947X
#define DWI_BASE_ADDR			(0x3F700000)
#endif

#define AMC_BASE_ADDR			(0x3F800000)

#define GPIO_BASE_ADDR			(0x3FA00000)
#define GPIO_VERSION			(1)
#if SUB_PLATFORM_S5L8940X
#define GPIO_GROUP_COUNT		(29)
#elif SUB_PLATFORM_S5L8942X
#define GPIO_GROUP_COUNT		(26)
#elif SUB_PLATFORM_S5L8947X
#define GPIO_GROUP_COUNT		(18)
#endif
#define GPIO_PAD_SPI			(GPIO_GROUP_COUNT)

#define PIO_BASE_ADDR			(0x3FB00000)
#define PIO_SPACING			(0x00100000)

#define CIF_BASE_ADDR			(0x3FD00000)

#define CDIO_WIDGETS_BASE_ADDR		(0x3FE00000)
#define CDIO_PL301_BASE_ADDR		(0x3FF00000)

#endif /* ! __PLATFORM_SOC_HWREGBASE_H */
