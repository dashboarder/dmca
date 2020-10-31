/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/miu.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>

static void miu_configure_bridge(const u_int32_t *bridge_settings);

#define BRIDGE_SHIFT		(16)
#define BRIDGE_MASK		(0xFFFF)
#define NRT_DART_WIDGETS	(0 << BRIDGE_SHIFT)
#define NRT_DART_BRIDGE		(1 << BRIDGE_SHIFT)
#define NRT_TOP_WIDGETS		(2 << BRIDGE_SHIFT)
#define NRT_TOP_BRIDGE		(3 << BRIDGE_SHIFT)
#define RT_TOP_WIDGETS		(4 << BRIDGE_SHIFT)
#define RT_TOP_BRIDGE		(5 << BRIDGE_SHIFT)
#define UPERF_WIDGETS		(6 << BRIDGE_SHIFT)
#define UPERF_BRIDGE		(7 << BRIDGE_SHIFT)
#define CDIO_WIDGETS		(8 << BRIDGE_SHIFT)
#define CDIO_BRIDGE		(9 << BRIDGE_SHIFT)

static const u_int32_t bridge_registers[] = {
	NRT_DART_WIDGETS_BASE_ADDR,	NRT_DART_PL301_BASE_ADDR,
	NRT_TOP_WIDGETS_BASE_ADDR,	NRT_TOP_PL301_BASE_ADDR,
	RT_TOP_WIDGETS_BASE_ADDR,	RT_TOP_PL301_BASE_ADDR,
	UPERF_WIDGETS_BASE_ADDR,	UPERF_PL301_BASE_ADDR,
	CDIO_WIDGETS_BASE_ADDR,		CDIO_PL301_BASE_ADDR,
};

#if APPLICATION_SECUREROM || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
// The following settings are based on H5P/Bali Tunables Revision 0.46

static const u_int32_t bridge_settings_static[] = {
	// Configure recommended transaction limits in CDIO
	CDIO_WIDGETS | CDIO_IOP_RRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_IOP_WRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_IOP_RTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_IOP_WTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_CDMA_RRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_CDMA_WRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_CDMA_RTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_CDMA_WTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_UPERF_RRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_UPERF_WRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_UPERF_RTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_UPERF_WTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_AUDIO_RRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_AUDIO_WRLIMIT,	0x00000000,
	CDIO_WIDGETS | CDIO_AUDIO_RTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_AUDIO_WTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_PIO_RTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_PIO_WTLIMIT,	0x00000404,
	CDIO_WIDGETS | CDIO_CIM_RTLIMIT,	0x00003F3F,
	CDIO_WIDGETS | CDIO_CIM_WTLIMIT, 	0x00000F0F,

	// Configure recommended bridge settings in CDIO
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x00000000,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x00000000,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x00000000,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x00000000,

	// Configure recommended features in CDIO
	CDIO_WIDGETS | CDIO_IOP_WGATHER,	0x00000100,
	CDIO_WIDGETS | CDIO_IOP_RC_REMAP(0),	0xffffbbbb,
	CDIO_WIDGETS | CDIO_IOP_RC_REMAP(1),	0xfedcba98,
	CDIO_WIDGETS | CDIO_IOP_RC_REMAP(2),	0xffffbbbb,
	CDIO_WIDGETS | CDIO_IOP_RC_REMAP(3),	0xfedcba98,
	CDIO_WIDGETS | CDIO_IOP_WC_REMAP(0),	0x76767676,
	CDIO_WIDGETS | CDIO_IOP_WC_REMAP(1),	0xfedcfedc,
	CDIO_WIDGETS | CDIO_IOP_WC_REMAP(2),	0x76767676,
	CDIO_WIDGETS | CDIO_IOP_WC_REMAP(3),	0xfedcfedc,
	CDIO_WIDGETS | CDIO_IOP_C_REMAP_EN,	0x00000003,
	CDIO_WIDGETS | CDIO_CDMA_WGATHER,	0x00000100,
	CDIO_WIDGETS | CDIO_UPERF_WGATHER,	0x00000100,
	CDIO_WIDGETS | CDIO_AUDIO_WGATHER,	0x00000100,

	// Configure recommended traffic attributes in UPERF
	//	AxCACHE = Cacheable
	//	AxUSER = enable Shared bit
	UPERF_WIDGETS | UPERF_OTG_QOS,		0x01010101,
	UPERF_WIDGETS | UPERF_OTG_CACHE,	0x0107010B,
	UPERF_WIDGETS | UPERF_EHCI_QOS,		0x01010101,
	UPERF_WIDGETS | UPERF_EHCI_CACHE,	0x0107010B,
	UPERF_WIDGETS | UPERF_OHCI0_QOS,	0x01010101,
	UPERF_WIDGETS | UPERF_OHCI0_CACHE,	0x0107010B,
	UPERF_WIDGETS | UPERF_OHCI1_QOS,	0x01010101,
	UPERF_WIDGETS | UPERF_OHCI1_CACHE,	0x0107010B,
	UPERF_WIDGETS | UPERF_USB2HOST1EHCI_QOS, 0x01010101,
	UPERF_WIDGETS | UPERF_USB2HOST1EHCI_CACHE, 0x0107010B,

	// Configure recommended bridge settings in UPERF
	UPERF_BRIDGE | UPERF_RTLIMIT,		0x00000404,
	UPERF_BRIDGE | UPERF_WTLIMIT,		0x00000404,

	0, 0,
	0, 0
};
#endif	// APPLICATION_SECUREROM || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

#if WITH_DEVICETREE || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static const u_int32_t bridge_settings_dynamic[] = {
	// Configure recommended transaction limits in HPerf-NRT DART
	NRT_DART_WIDGETS | HPERF_NRT_DART_DART_RTLIMIT,		0x00003F3F,
	NRT_DART_WIDGETS | HPERF_NRT_DART_DART_WTLIMIT,		0x00003F3F,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER0_RTLIMIT,	0x00000707,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER0_WTLIMIT,	0x00000404,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER1_RTLIMIT,	0x00000707,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER1_WTLIMIT,	0x00000404,
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG0_RTLIMIT,	0x00003F3F,
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG0_WTLIMIT,	0x00003F3F,
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG1_RTLIMIT,	0x00003F3F,
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG1_WTLIMIT,	0x00003F3F,

	// Configure recommended bridge settings in HPerf-NRT DART
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x00000000,	// JPEG0 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x01000001,	// JPEG1 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x02000002,	// Scaler0 Reduced Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x03000003,	// Sclaer1 Reduced Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x00000000,	// JPEG0 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x01000001,	// JPEG1 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x02000002,	// Scaler0 Reduced Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x03000003,	// Sclaer1 Reduced Priority

	// Configure recommended bridge settings in HPerf-NRT DART
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG0_QOS,		0x01010101,	// JPEG0 LLT Traffic for R&W
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG1_QOS,		0x01010101,	// JPEG1 LLT Traffic for R&W
	0, 0,

	// Configure recommended transaction limits in HPerf-NRT Top
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_TOP_RTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_TOP_WTLIMIT,		0x00000F0F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VENC_RTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VENC_WTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VDEC_RTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VDEC_WTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_AUX_RTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_AUX_WTLIMIT,		0x00003F3F,

	// Configure recommended bridge settings in HPerf-NRT Top
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AR_CHAN_ARB_MI,		0x00000000,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AR_CHAN_ARB_MI,		0x01000001,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AR_CHAN_ARB_MI,		0x02000002,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AR_CHAN_ARB_MI,		0x03000003,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AW_CHAN_ARB_MI,		0x00000000,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AW_CHAN_ARB_MI,		0x01000001,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AW_CHAN_ARB_MI,		0x02000002,
	NRT_TOP_BRIDGE | HPERF_NRT_TOP_AW_CHAN_ARB_MI,		0x03000003,

	// Configure recommended features in HPerf-NRT Top
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_AUX_WGATHER,		0x00000100,
	0, 0,

	// Configure recommended transaction limits in HPerf-RT Top
	RT_TOP_WIDGETS | HPERF_RT_TOP_DISP0_RTLIMIT,		0x00003F3F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_DISP1_RTLIMIT,		0x00001818,
	RT_TOP_WIDGETS | HPERF_RT_TOP_ISP_RTLIMIT,		0x00003F3F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_ISP_WTLIMIT,		0x00003F3F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_TOP_RTLIMIT,		0x00003F3F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_TOP_WTLIMIT,		0x00000F0F,

	// Configure recommended bridge settings in HPerf-RT Top
	RT_TOP_BRIDGE | HPERF_RT_TOP_AR_CHAN_ARB_MI,		0x00000000,
	RT_TOP_BRIDGE | HPERF_RT_TOP_AW_CHAN_ARB_MI,		0x00000000,
	0, 0,

	0, 0
};
#endif	// WITH_DEVICETREE || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

int miu_initialize_internal_ram(void)
{
#if APPLICATION_SECUREROM
	// Ensure that rPMGR_SCRATCH0-3 get cleared
	rPMGR_SCRATCH0 = 0;
	rPMGR_SCRATCH1 = 0;
	rPMGR_SCRATCH2 = 0;
	rPMGR_SCRATCH3 = 0;
#endif /* APPLICATION_SECUREROM */

	// Save the Security Epoch in the top byte of PMGR_SCRATCH0
	rPMGR_SCRATCH0 &= ~0xFF000000;
	rPMGR_SCRATCH0 |= (platform_get_security_epoch()) << 24;

	return 0;
}

int miu_init(void)
{
#if APPLICATION_IBOOT && !PRODUCT_IBEC
	// Verify that the Security Epoch in PMGR_SCRATCH0 matches
	if ((rPMGR_SCRATCH0 >> 24) != platform_get_security_epoch()) {
		panic("miu_init: Epoch Mismatch\n");
	}
#endif

#if APPLICATION_SECUREROM || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// Configure bridges that are statically powered
	clock_gate(CLK_UPERF, true);
	miu_configure_bridge(bridge_settings_static);
	clock_gate(CLK_UPERF, false);
#endif

#if APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC
	// Configure bridges that are dynamically powered
	clock_gate(CLK_HPERFNRT, true);
	clock_gate(CLK_HPERFRT, true);
	miu_configure_bridge(bridge_settings_dynamic);
	clock_gate(CLK_HPERFNRT, false);
	clock_gate(CLK_HPERFRT, false);
#endif

	/* remap whatever bank of ram we're in to zero */
	if (TEXT_BASE == SRAM_BASE)
		miu_select_remap(REMAP_SRAM);
	else if ((TEXT_BASE >= SDRAM_BASE) && (TEXT_BASE < SDRAM_END))
		miu_select_remap(REMAP_SDRAM);

	return 0;
}

static void miu_configure_bridge(const u_int32_t *bridge_settings)
{
	volatile u_int32_t *reg;
	u_int32_t cnt = 0, bridge, offset, data;

	while ((bridge_settings[cnt] != 0) || (bridge_settings[cnt + 1] != 0)) {
		while ((bridge_settings[cnt] != 0) || (bridge_settings[cnt + 1] != 0)) {
			bridge = bridge_settings[cnt] >> BRIDGE_SHIFT;
			offset = bridge_settings[cnt] & BRIDGE_MASK;
			data = bridge_settings[cnt + 1];
			reg = (volatile u_int32_t *)(bridge_registers[bridge] + offset);
			*reg = data;
			cnt += 2;
		}
		cnt += 2;
	}
}

void miu_suspend(void)
{
	/* nothing required for suspend */
}

int miu_initialize_dram(bool resume)
{
#if APPLICATION_IBOOT && WITH_HW_AMC
	mcu_initialize_dram(resume);
#endif

	return 0;
}

void miu_select_remap(enum remap_select sel)
{
	u_int32_t tmp;

	switch (sel) {
		case REMAP_SRAM:
			tmp = miu_read_l2cadrmap() & ~3;
			tmp |= 1; // RemapMode = 1 (L2C RAM)
			miu_write_l2cadrmap(tmp);
			break;

		case REMAP_SDRAM:
			tmp = miu_read_l2cadrmap() & ~3;
			tmp |= 2; // RemapMode = 2 (MEM)
			miu_write_l2cadrmap(tmp);
			break;
	}
}

void miu_bypass_prep(int step)
{
#if APPLICATION_IBOOT && WITH_HW_AMC
	extern void mcu_bypass_prep(int step);
	mcu_bypass_prep(step);
#endif
}

#if WITH_DEVICETREE

void miu_update_device_tree(DTNode *pmgr_node)
{
	uint32_t propSize;
	char *propName;
	void *propData;

	// Fill in the bridge-settings property
	propName = "bridge-settings";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize < sizeof(bridge_settings_dynamic)) {
			panic("miu_update_device_tree: bridge-settings property is too small (0x%x)", propSize);
		}
		memcpy(propData, bridge_settings_dynamic, sizeof(bridge_settings_dynamic));
	}
}

#endif
