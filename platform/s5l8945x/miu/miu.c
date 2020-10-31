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
#define UPERF1_WIDGETS          (10 << BRIDGE_SHIFT)
#define UPERF1_BRIDGE           (11 << BRIDGE_SHIFT)

static const u_int32_t bridge_registers[] = {
	NRT_DART_WIDGETS_BASE_ADDR,	NRT_DART_PL301_BASE_ADDR,
	NRT_TOP_WIDGETS_BASE_ADDR,	NRT_TOP_PL301_BASE_ADDR,
	RT_TOP_WIDGETS_BASE_ADDR,	RT_TOP_PL301_BASE_ADDR,
	UPERF_WIDGETS_BASE_ADDR,	UPERF_PL301_BASE_ADDR,
	CDIO_WIDGETS_BASE_ADDR,		CDIO_PL301_BASE_ADDR,
	UPERF1_WIDGETS_BASE_ADDR,	UPERF1_PL301_BASE_ADDR,
};

// The following settings are based on H4 Tunables Revision 0.24

#if APPLICATION_SECUREROM || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static const u_int32_t bridge_settings_static[] = {
	// Configure recommended transaction limits in CDIO
	CDIO_WIDGETS | CDIO_IOP_RTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_IOP_WTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_CDMA_RTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_CDMA_WTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_UPERF_RTLIMIT,	0x00000404,
	CDIO_WIDGETS | CDIO_UPERF_WTLIMIT,	0x00000404,
	CDIO_WIDGETS | CDIO_AUDIO_RTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_AUDIO_WTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_PIO_RTLIMIT,	0x00000808,
	CDIO_WIDGETS | CDIO_PIO_WTLIMIT,	0x00000404,
	CDIO_WIDGETS | CDIO_UPERF1_RTLIMIT,	0x00000404,
	CDIO_WIDGETS | CDIO_UPERF1_WTLIMIT,	0x00000404,

	// Configure recommended bridge settings in CDIO
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x00000000,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x01000001,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x02000002,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x03000003,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x04000004,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI0,	0x05000005,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x00000000,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x01000001,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x02000002,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x03000003,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x04000004,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI0,	0x05000005,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x00000000,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x01000001,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x02000002,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x03000003,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x04000004,
	CDIO_BRIDGE | CDIO_AR_CHAN_ARB_MI1,	0x05000005,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x00000000,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x01000001,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x02000002,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x03000003,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x04000004,
	CDIO_BRIDGE | CDIO_AW_CHAN_ARB_MI1,	0x05000005,

	// Configure recommended features in CDIO
	CDIO_WIDGETS | CDIO_IOP_WREQSP,		0x00000003,
	CDIO_WIDGETS | CDIO_CDMA_WGATHER,	0x00000100,
	CDIO_WIDGETS | CDIO_UPERF_WGATHER,	0x00000100,
	CDIO_WIDGETS | CDIO_AUDIO_WGATHER,	0x00000100,
	CDIO_WIDGETS | CDIO_UPERF1_WGATHER,	0x00000100,

	// Configure recommended traffic attributes in UPERF
	//	AxCACHE = Cacheable
	//	AxUSER = enable Shared bit
	UPERF_WIDGETS | UPERF_OTG_QOS,		0x01010101,
	UPERF_WIDGETS | UPERF_OTG_CACHE,	0x01020102,
	UPERF_WIDGETS | UPERF_EHCI_QOS,		0x01010101,
	UPERF_WIDGETS | UPERF_EHCI_CACHE,	0x01020102,
	UPERF_WIDGETS | UPERF_OHCI0_QOS,	0x01010101,
	UPERF_WIDGETS | UPERF_OHCI0_CACHE,	0x01020102,
	UPERF_WIDGETS | UPERF_OHCI1_QOS,	0x01010101,
	UPERF_WIDGETS | UPERF_OHCI1_CACHE,	0x01020102,

	UPERF1_WIDGETS | UPERF_OTG_QOS,		0x01010101,
	UPERF1_WIDGETS | UPERF_OTG_CACHE,	0x01020102,
	UPERF1_WIDGETS | UPERF_EHCI_QOS,	0x01010101,
	UPERF1_WIDGETS | UPERF_EHCI_CACHE,	0x01020102,
	UPERF1_WIDGETS | UPERF_OHCI0_QOS,	0x01010101,
	UPERF1_WIDGETS | UPERF_OHCI0_CACHE,	0x01020102,
	UPERF1_WIDGETS | UPERF_OHCI1_QOS,	0x01010101,
	UPERF1_WIDGETS | UPERF_OHCI1_CACHE,	0x01020102,

	0, 0,
	0, 0
};
#endif	// APPLICATION_SECUREROM || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

#if WITH_DEVICETREE || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static const u_int32_t bridge_settings_dynamic[] = {
	// Configure recommended transaction limits in HPerf-NRT DART
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER0_RTLIMIT,	0x00000707,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER0_WTLIMIT,	0x00000404,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER1_RTLIMIT,	0x00000707,
	NRT_DART_WIDGETS | HPERF_NRT_DART_SCALER1_WTLIMIT,	0x00000404,
	NRT_DART_WIDGETS | HPERF_NRT_DART_DART_RTLIMIT,		0x00000808,
	NRT_DART_WIDGETS | HPERF_NRT_DART_DART_WTLIMIT,		0x00000808,

	// Configure recommended bridge settings in HPerf-NRT DART
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x00000000,	// JPEG0 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x01000001,	// JPEG1 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x02000102,	// Scaler0 Reduced Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AR_CHAN_ARB_MI,	0x03000103,	// Sclaer1 Reduced Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x00000000,	// JPEG0 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x01000001,	// JPEG1 High Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x02000102,	// Scaler0 Reduced Priority
	NRT_DART_BRIDGE | HPERF_NRT_DART_AW_CHAN_ARB_MI,	0x03000103,	// Sclaer1 Reduced Priority

	// Configure recommended bridge settings in HPerf-NRT DART
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG0_QOS,		0x01010101,	// JPEG0 LLT Traffic for R&W
	NRT_DART_WIDGETS | HPERF_NRT_DART_JPEG1_QOS,		0x01010101,	// JPEG0 LLT Traffic for R&W
	0, 0,

	// Configure recommended transaction limits in HPerf-NRT Top
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VENC_RTLIMIT,		0x00002020,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VENC_WTLIMIT,		0x00000808,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VDEC_RTLIMIT,		0x00001010,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VDEC_WTLIMIT,		0x00000808,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_TOP_RTLIMIT,		0x00003F3F,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_TOP_WTLIMIT,		0x00000F0F,

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
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_DART_WGATHER,		0x00000000,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VENC_WREQSP,		0x00000003,
	NRT_TOP_WIDGETS | HPERF_NRT_TOP_VDEC_WREQSP,		0x00000003,
	0, 0,

	// Configure recommended transaction limits in HPerf-RT Top
	RT_TOP_WIDGETS | HPERF_RT_TOP_TOP_RTLIMIT,		0x00003F3F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_TOP_WTLIMIT,		0x00000F0F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_DISP0_RTLIMIT,		0x00003F3F,
	RT_TOP_WIDGETS | HPERF_RT_TOP_DISP1_RTLIMIT,		0x00001818,
	RT_TOP_WIDGETS | HPERF_RT_TOP_ISP_RTLIMIT,		0x00000C0C,
	RT_TOP_WIDGETS | HPERF_RT_TOP_ISP_WTLIMIT,		0x00001010,

	// Configure recommended bridge settings in HPerf-RT Top
	RT_TOP_BRIDGE | HPERF_RT_TOP_AR_CHAN_ARB_MI,		0x00000000,
	RT_TOP_BRIDGE | HPERF_RT_TOP_AR_CHAN_ARB_MI,		0x01000001,
	RT_TOP_BRIDGE | HPERF_RT_TOP_AR_CHAN_ARB_MI,		0x02000002,
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

#if APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC
	// Configure recommended transaction limits in CIF
	rCIF_RTLIMIT = 0x00000808;
	rCIF_WTLIMIT = 0x00000808;

	// Tunables recommendation, for SCU clock-gating
	// (will change to 0x80 for <rdar://problem/8267114>)
	rCIF_IDLETMR = 0x00000010;
#endif


#if APPLICATION_SECUREROM || (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// Configure bridges that are statically powered
	clock_gate(CLK_UPERF, true);
	clock_gate(CLK_UPERF1, true);
	miu_configure_bridge(bridge_settings_static);
	clock_gate(CLK_UPERF1, false);
	clock_gate(CLK_UPERF, false);
#endif

#if APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC
	// Configure bridges that are dynamically powered
	clock_gate(CLK_HPERFNRT, true);
	clock_gate(CLK_HPERFRT, true);
	miu_configure_bridge(bridge_settings_dynamic);
	clock_gate(CLK_HPERFNRT, false);
	clock_gate(CLK_HPERFRT, false);
	
	/* Enable the SCU.  Note that we explicitly do NOT touch the
	 * "power status" register, since the default is supposed to
	 * be correct, and the real control is in the CPU's "SMP"
	 * bit. */
	rSCU_TAG_INVAL = 0xff;	/* invalidate SCU tags for all CPUs */

	/* Enable SCU, standby, speculative linefills, and parity */
	rSCU_CONTROL = (1 << 5) | (1 << 3) | (1 << 2) | (1 << 0);
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
	/* XXX ? */
}

int miu_initialize_dram(bool resume)
{
#if APPLICATION_IBOOT
	mcu_initialize_dram(resume);
#endif
	return 0;
}

void miu_select_remap(enum remap_select sel)
{
	switch (sel) {
		case REMAP_SRAM:
			rREMAP_CTL = (rREMAP_CTL & ~3) | (1 << 0); // remap_sel = 1
			rPL310_FILTER_START = 0x1;
			break;
		case REMAP_SDRAM:
			rREMAP_CTL = (rREMAP_CTL & ~3) | (2 << 0); // remap_sel = 2
			// Leave low megabyte heading to DRAM for wake-from-sleep,
			// Otherwise send everything below DRAM to the PIO block.
			// Currently this is for testing only; it is not POR.
			rPL310_FILTER_END   = 0x80000000;
			rPL310_FILTER_START = 0x00100001;
			break;
	}

	/* read remap register to ensure it has been updated */
	rREMAP_CTL;
}

void miu_bypass_prep(void)
{
#if APPLICATION_IBOOT
	extern void mcu_bypass_prep(void);
	mcu_bypass_prep();
#endif
}

#if WITH_DEVICETREE

void miu_update_device_tree(DTNode *pmgr_node)
{
	u_int32_t propSize;
	char *propName;
	void *propData;

	// Fill in the bridge-settings property
	propName = "bridge-settings";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(bridge_settings_dynamic)) {
			memcpy(propData, bridge_settings_dynamic, sizeof(bridge_settings_dynamic));
		}
	}
}

#endif
