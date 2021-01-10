/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _DCS_S8000_H
#define _DCS_S8000_H

#include <drivers/dcs/dcs.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <sys/types.h>

/*
 * DCS_REG_VERSION mapping used in iBoot for various platforms
 * Maui:  1
 * s8003: 2
 * Elba:  3
 * Cayman: 4
 *
 * SoCs with "dcsh" block to have bit 4 set
 * M8:    0x10
 */

#ifndef DCS_REG_VERSION
#define DCS_REG_VERSION	(1)
#endif

#ifndef NUM_AMCCS
#define NUM_AMCCS		(1)
#endif

/* Macro to read a register in a particular DCS channel */
#define DCS_REG_ACCESS(reg)	(*(volatile uint32_t *)(reg))
#define DCS_REG_READ_CH(_c,reg) DCS_REG_ACCESS((reg) + ((_c)*DCS_SPACING))

// Used by Calibration Code
#define CSR_READ(addr)		DCS_REG_ACCESS(addr)
#define CSR_WRITE(addr, val)	EXEC(DCS_REG_ACCESS(addr) = val)


#define KAY				(1024)
#define MEG				(KAY*KAY)

#define DCS_FREQUENCY_SLOTS	(4)

#if DCS_REG_VERSION >= 0x10
// PIOWRAP registers
#define rAMC_PIO_ACCESS_CFG				(PIO_BASE_ADDR + 0x02C)

// AMCSYS registers
#define rAMC_AMCEN					(AMC_BASE_ADDR + 0x000)
#define rAMC_CLKGATE					(AMC_BASE_ADDR + 0x004)
#define rAMC_AIUPRT_RD_CWF				(AMC_BASE_ADDR + 0x084)
#define rAMC_AIUPRT_RD_GEN				(AMC_BASE_ADDR + 0x088)
#define rAMC_AIUPRTBADDR				(AMC_BASE_ADDR + 0x0A0)
#define rAMC_AIUCHNLDEC				(AMC_BASE_ADDR + 0x0A4)
#define rAMC_AIUADDRBANKHASH(_n)			(AMC_BASE_ADDR + (_n * 4) + 0x0B0)

#define rAMC_AIUCHNLCRD				(AMC_BASE_ADDR + 0x100)
#define rAMC_AIUCHNLTM					(AMC_BASE_ADDR + 0x104)
#define rAMC_AIURDBRSTLEN				(AMC_BASE_ADDR + 0x108)
#define rAMC_AIURDMAXCRD				(AMC_BASE_ADDR + 0x10C)
#define rAMC_AIURDMINCRD				(AMC_BASE_ADDR + 0x110)
#define rAMC_AIURDREFL					(AMC_BASE_ADDR + 0x114)
#define rAMC_AIUWRBRSTLEN				(AMC_BASE_ADDR + 0x118)
#define rAMC_AIUWRMAXCRD				(AMC_BASE_ADDR + 0x11C)
#define rAMC_AIUWRMINCRD				(AMC_BASE_ADDR + 0x120)
#define rAMC_AIUWRREFL					(AMC_BASE_ADDR + 0x124)
#define rAMC_AIUCPUSPCREQ				(AMC_BASE_ADDR + 0x128)
#define rAMC_AIUGRXSPCREQ				(AMC_BASE_ADDR + 0x12C)
#define rAMC_AIUNRTSPCREQ				(AMC_BASE_ADDR + 0x130)
#define rAMC_AIURLTSPCREQ				(AMC_BASE_ADDR + 0x134)
#define rAMC_AIUWPQSCH					(AMC_BASE_ADDR + 0x138)
#define rAMC_AIUTHOTLEN				(AMC_BASE_ADDR + 0x13C)
#define rAMC_AIUTHOTLCPUPARAM				(AMC_BASE_ADDR + 0x140)
#define rAMC_AIUTHOTLRLTPARAM				(AMC_BASE_ADDR + 0x144)
#define rAMC_AIUTHOTLNRTPARAM				(AMC_BASE_ADDR + 0x148)
#define rAMC_AIUTHOTLGRXPARAM				(AMC_BASE_ADDR + 0x14C)
#define rAMC_AIUPUSHEN					(AMC_BASE_ADDR + 0x150)
#define rAMC_AIUTHOTLCPUPARAM2				(AMC_BASE_ADDR + 0x154)
#define rAMC_AIUTHOTLRLTPARAM2				(AMC_BASE_ADDR + 0x158)
#define rAMC_AIUTHOTLNRTPARAM2				(AMC_BASE_ADDR + 0x15C)
#define rAMC_AIUTHOTLGRXPARAM2				(AMC_BASE_ADDR + 0x160)
#define rAMC_AIULLTSCHCTL				(AMC_BASE_ADDR + 0x164)
#define rAMC_AIURDMAXCRD2				(AMC_BASE_ADDR + 0x168)
#define rAMC_AIURDMINCRD2				(AMC_BASE_ADDR + 0x16C)
#define rAMC_AIURDREFL2				(AMC_BASE_ADDR + 0x170)
#define rAMC_AIUWRMAXCRD2				(AMC_BASE_ADDR + 0x174)
#define rAMC_AIUWRMINCRD2				(AMC_BASE_ADDR + 0x178)
#define rAMC_AIUWRREFL2				(AMC_BASE_ADDR + 0x17C)
#define rAMC_AIUMEDSPCREQ				(AMC_BASE_ADDR + 0x180)
#define rAMC_AIUTHOTLMEDPARAM				(AMC_BASE_ADDR + 0x184)
#define rAMC_AIUTHOTLMEDPARAM2				(AMC_BASE_ADDR + 0x188)

#define rAMC_QBREN					(AMC_BASE_ADDR + 0x200)
#define rAMC_PSQRQCTL0					(AMC_BASE_ADDR + 0x208)
#define rAMC_PSQRQCTL1					(AMC_BASE_ADDR + 0x20C)
#define rAMC_PSQRQTIMER(_n)				(AMC_BASE_ADDR + (_n * 4) + 0x210)
#define rAMC_PSQRQBRST					(AMC_BASE_ADDR + 0x21C)
#define rAMC_PSQRQSCHCRD				(AMC_BASE_ADDR + 0x220)
#define rAMC_PSQWQCTL1					(AMC_BASE_ADDR + 0x228)
#define rAMC_PSQWQBRST					(AMC_BASE_ADDR + 0x230)
#define rAMC_PSQWQSCHCRD				(AMC_BASE_ADDR + 0x234)
#define rAMC_MCUQOS					(AMC_BASE_ADDR + 0x238)
#define rAMC_MCUQOSLLT					(AMC_BASE_ADDR + 0x23C)

#define rAMC_ADDRMAPMODE				(AMC_BASE_ADDR + 0x400)
#define rAMC_ADDRCFG					(AMC_BASE_ADDR + 0x404)
#define rAMC_PWRMNGTPARAM				(AMC_BASE_ADDR + 0x408)

// AMCSYS_AON
#define rAMC_IDLE2BCG_CONTROL				(AMC_BASE_ADDR + 0x8014)

// DCSH_AON
#define rDCS_AON_IDLE2BCG_CONTROL			(DCS_BASE_ADDR + DCSH_AON_SPACING + 0x004)
#endif // #if DCS_REG_VERSION >= 0x10

#if NUM_AMCCS == 2

/* AMCC0 registers */
#define rAMCC_MCCCHNLDEC_ADDR					(AMCC_BASE_ADDR(0) + 0x4A4)
#define rAMCC_MCUCHNHASH_ADDR					(AMCC_BASE_ADDR(0) + 0x4A8)
#define rAMCC_DRAMACCCTRL_ADDR					(AMCC_BASE_ADDR(0) + 0x4B8)
#define rAMCC_ADDRCFG_ADDR					(AMCC_BASE_ADDR(0) + 0x4CC)
#define rAMCC_MCC0QPROPCTRL_ADDR				(AMCC_BASE_ADDR(0) + 0x648)
#define rAMCC_MCC1QPROPCTRL_ADDR				(AMCC_BASE_ADDR(0) + 0x64C)
#define rAMCC_MCCGEN_ADDR					(AMCC_BASE_ADDR(0) + 0x780)
#define rAMCC_MCCCFG_TAG_RD_DIS_ADDR				(AMCC_BASE_ADDR(0) + 0x794)
#define rAMCC_MCCPWRONWAYCNTSTATUS_ADDR				(AMCC_BASE_ADDR(0) + 0x79C)

#define rAMCC_MCCCHNLDEC					DCS_REG_ACCESS(rAMCC_MCCCHNLDEC_ADDR)
#define rAMCC_DRAMACCCTRL					DCS_REG_ACCESS(rAMCC_DRAMACCCTRL_ADDR)
#define rAMCC_ADDRCFG						DCS_REG_ACCESS(rAMCC_ADDRCFG_ADDR)
#define rAMCC_MCC0QPROPCTRL					DCS_REG_ACCESS(rAMCC_MCC0QPROPCTRL_ADDR)
#define rAMCC_MCC1QPROPCTRL					DCS_REG_ACCESS(rAMCC_MCC1QPROPCTRL_ADDR)
#define rAMCC_MCCGEN						DCS_REG_ACCESS(rAMCC_MCCGEN_ADDR)
#define rAMCC_MCCCFG_TAG_RD_DIS					DCS_REG_ACCESS(rAMCC_MCCCFG_TAG_RD_DIS_ADDR)
#define rAMCC_MCCPWRONWAYCNTSTATUS				DCS_REG_ACCESS(rAMCC_MCCPWRONWAYCNTSTATUS_ADDR)

/* AMCC1 registers */
#define rAMCC1_MCCCHNLDEC_ADDR					(AMCC_BASE_ADDR(1) + 0x4A4)
#define rAMCC1_MCUCHNHASH_ADDR					(AMCC_BASE_ADDR(1) + 0x4A8)
#define rAMCC1_DRAMACCCTRL_ADDR					(AMCC_BASE_ADDR(1) + 0x4B8)
#define rAMCC1_ADDRCFG_ADDR					(AMCC_BASE_ADDR(1) + 0x4CC)
#define rAMCC1_MCC0QPROPCTRL_ADDR				(AMCC_BASE_ADDR(1) + 0x648)
#define rAMCC1_MCC1QPROPCTRL_ADDR				(AMCC_BASE_ADDR(1) + 0x64C)
#define rAMCC1_MCCGEN_ADDR					(AMCC_BASE_ADDR(1) + 0x780)
#define rAMCC1_MCCCFG_TAG_RD_DIS_ADDR				(AMCC_BASE_ADDR(1) + 0x794)
#define rAMCC1_MCCPWRONWAYCNTSTATUS_ADDR			(AMCC_BASE_ADDR(1) + 0x79C)

#define rAMCC1_MCCCHNLDEC					DCS_REG_ACCESS(rAMCC1_MCCCHNLDEC_ADDR)
#define rAMCC1_DRAMACCCTRL					DCS_REG_ACCESS(rAMCC1_DRAMACCCTRL_ADDR)
#define rAMCC1_ADDRCFG						DCS_REG_ACCESS(rAMCC1_ADDRCFG_ADDR)
#define rAMCC1_MCC0QPROPCTRL					DCS_REG_ACCESS(rAMCC1_MCC0QPROPCTRL_ADDR)
#define rAMCC1_MCC1QPROPCTRL					DCS_REG_ACCESS(rAMCC1_MCC1QPROPCTRL_ADDR)
#define rAMCC1_MCCGEN						DCS_REG_ACCESS(rAMCC1_MCCGEN_ADDR)
#define rAMCC1_MCCCFG_TAG_RD_DIS				DCS_REG_ACCESS(rAMCC1_MCCCFG_TAG_RD_DIS_ADDR)
#define rAMCC1_MCCPWRONWAYCNTSTATUS				DCS_REG_ACCESS(rAMCC1_MCCPWRONWAYCNTSTATUS_ADDR)

#elif NUM_AMCCS == 1

/* AMCC registers */
#define rAMCC_MCCCHNLDEC_ADDR					(AMCC_BASE_ADDR(0) + 0x4A4)
#define rAMCC_DRAMACCCTRL_ADDR					(AMCC_BASE_ADDR(0) + 0x4B4)
#define rAMCC_ADDRCFG_ADDR					(AMCC_BASE_ADDR(0) + 0x4C8)
#define rAMCC_MCC0QPROPCTRL_ADDR				(AMCC_BASE_ADDR(0) + 0x648)
#define rAMCC_MCC1QPROPCTRL_ADDR				(AMCC_BASE_ADDR(0) + 0x64C)
#define rAMCC_MCCGEN_ADDR					(AMCC_BASE_ADDR(0) + 0x780)
#define rAMCC_MCCCFG_TAG_RD_DIS_ADDR				(AMCC_BASE_ADDR(0) + 0x794)
#define rAMCC_MCCPWRONWAYCNTSTATUS_ADDR				(AMCC_BASE_ADDR(0) + 0x79C)

#define rAMCC_MCCCHNLDEC					DCS_REG_ACCESS(rAMCC_MCCCHNLDEC_ADDR)
#define rAMCC_DRAMACCCTRL					DCS_REG_ACCESS(rAMCC_DRAMACCCTRL_ADDR)
#define rAMCC_ADDRCFG						DCS_REG_ACCESS(rAMCC_ADDRCFG_ADDR)
#define rAMCC_MCC0QPROPCTRL					DCS_REG_ACCESS(rAMCC_MCC0QPROPCTRL_ADDR)
#define rAMCC_MCC1QPROPCTRL					DCS_REG_ACCESS(rAMCC_MCC1QPROPCTRL_ADDR)
#define rAMCC_MCCGEN						DCS_REG_ACCESS(rAMCC_MCCGEN_ADDR)
#define rAMCC_MCCCFG_TAG_RD_DIS					DCS_REG_ACCESS(rAMCC_MCCCFG_TAG_RD_DIS_ADDR)
#define rAMCC_MCCPWRONWAYCNTSTATUS				DCS_REG_ACCESS(rAMCC_MCCPWRONWAYCNTSTATUS_ADDR)

#endif // #if NUM_AMCCS == 2

#define AMCC_MCCCFG_TAG_RD_DIS_ADDR(i)				(AMCC_BASE_ADDR(i) + 0x794)

#define AMCC_MCCGEN_MCC_RAM_EN					(1 << 1)
#define AMCC_MCCGEN_MCC_RAM_EN_LOCK				(1 << 16)

#define DCS_NUM_MEG_IN_DRAM_BLK					(128)

/* MCU registers */
#define rDCS_MCU_AMCCTRL					(DCS_BASE_ADDR + 0x000)
#define rDCS_MCU_CLKGATE					(DCS_BASE_ADDR + 0x004)
#define rDCS_MCU_AMCCLKPWRGATE					(DCS_BASE_ADDR + 0x010)
#define rDCS_MCU_AMCCLKPWRGATE2				(DCS_BASE_ADDR + 0x014)

#if DCS_REG_VERSION >= 0x10
#define rDCS_MCU_IDLE_EXTEND_CONTROL				(DCS_BASE_ADDR + 0x018)
#define rDCS_MCU_DIV_CLK_SELECT				(DCS_BASE_ADDR + 0x020)
#endif

#define rDCS_MCU_OPIDLETMR					(DCS_BASE_ADDR + 0x180)
#define rDCS_MCU_OPTMRADJEN					(DCS_BASE_ADDR + 0x184)
#define rDCS_MCU_OPTMRADJPARAM					(DCS_BASE_ADDR + 0x188)
#define rDCS_MCU_MIFNRTAGE					(DCS_BASE_ADDR + 0x18C)
#define rDCS_MCU_MIFRTAGE					(DCS_BASE_ADDR + 0x190)
#define rDCS_MCU_MIFPCAGE					(DCS_BASE_ADDR + 0x194)
#define rDCS_MCU_QBREN						(DCS_BASE_ADDR + 0x198)
#define rDCS_MCU_QBRPARAM					(DCS_BASE_ADDR + 0x19C)

#if DCS_REG_VERSION >= 0x10
#define rDCS_MCU_QBRPARAM2					(DCS_BASE_ADDR + 0x1A0)
#endif

#define rDCS_MCU_PSQRQCTL0					(DCS_BASE_ADDR + 0x1A4)

#define rDCS_MCU_PSQCPU0LLT_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 0xC) + 0x1A8)
#define rDCS_MCU_PSQCPU1LLT_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 0xC) + 0x1AC)
#define rDCS_MCU_PSQSOCLLT_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 0xC) + 0x1B0)

#define rDCS_MCU_PSQRQTIMER0					(DCS_BASE_ADDR + 0x1D8)
#define rDCS_MCU_PSQRQTIMER1					(DCS_BASE_ADDR + 0x1DC)
#define rDCS_MCU_PSQRQBRST					(DCS_BASE_ADDR + 0x1E0)
#define rDCS_MCU_PSQRQSCHCRD					(DCS_BASE_ADDR + 0x1E4)

#define rDCS_MCU_PSQWQ_CTL(_n)					(DCS_BASE_ADDR + ((_n) * 4) + 0x1E8)
#define rDCS_MCU_PSQWQCTL0					rDCS_MCU_PSQWQ_CTL(0)
#define rDCS_MCU_PSQWQCTL1					rDCS_MCU_PSQWQ_CTL(1)
#define rDCS_MCU_PSQWQCTL2					rDCS_MCU_PSQWQ_CTL(2)
#define rDCS_MCU_PSQWQTHR					(DCS_BASE_ADDR + 0x1F4)
#define rDCS_MCU_PSQWQBRST					(DCS_BASE_ADDR + 0x1F8)
#define rDCS_MCU_PSQWQSCHCRD					(DCS_BASE_ADDR + 0x1FC)

#define rDCS_MCU_QOSLLT						(DCS_BASE_ADDR + 0x200)
#define rDCS_MCU_SBQOSUPGCTL					(DCS_BASE_ADDR + 0x204)
#define rDCS_MCU_SBQOSUPGISPAIDS				(DCS_BASE_ADDR + 0x208)

#define rDCS_MCU_PHYRDWRTIM_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x280)
#define rDCS_MCU_PHYUPDATETIMERS				(DCS_BASE_ADDR + 0x300)

#if (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)
#define rDCS_MCU_LAT_FREQ(_f)					(DCS_BASE_ADDR +  ((_f) * 4) + 0x340)
#define rDCS_MCU_CASPCH_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x350)
#define rDCS_MCU_ACT_FREQ(_f)					(DCS_BASE_ADDR + ((_f) * 4) + 0x360)
#define rDCS_MCU_AUTOREF_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x370)
#define rDCS_MCU_SELFREF_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x380)
#define rDCS_MCU_PDN						(DCS_BASE_ADDR + 0x390)
#define rDCS_MCU_MODEREG					(DCS_BASE_ADDR + 0x394)
#define rDCS_MCU_DERATE_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x398)
#define rDCS_MCU_TAT_FREQ(_f)					(DCS_BASE_ADDR + ((_f) * 4) + 0x3A8)
#define rDCS_MCU_AUTOREF_PARAMS					(DCS_BASE_ADDR + 0x3BC)
#define rDCS_MCU_PDN1						(DCS_BASE_ADDR + 0x3C0)
#define rDCS_MCU_MODEREG1					(DCS_BASE_ADDR + 0x3C4)
#define rDCS_MCU_PDN_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x3C8)
#else
#define rDCS_MCU_LAT_FREQ(_f)					(DCS_BASE_ADDR +  ((_f) * 4) + 0x380)
#define rDCS_MCU_CASPCH_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x390)
#define rDCS_MCU_ACT_FREQ(_f)					(DCS_BASE_ADDR + ((_f) * 4) + 0x3A0)
#define rDCS_MCU_AUTOREF_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x3B0)
#define rDCS_MCU_SELFREF_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x3C0)
#define rDCS_MCU_PDN						(DCS_BASE_ADDR + 0x3D0)
#define rDCS_MCU_MODEREG					(DCS_BASE_ADDR + 0x3D4)
#define rDCS_MCU_DERATE_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x3D8)
#define rDCS_MCU_TAT_FREQ(_f)					(DCS_BASE_ADDR + ((_f) * 4) + 0x3E8)
#define rDCS_MCU_AUTOREF_PARAMS					(DCS_BASE_ADDR + 0x3FC)
#endif

#define rDCS_MCU_RWCFG						(DCS_BASE_ADDR + 0x400)
#define rDCS_MCU_RNKCFG						(DCS_BASE_ADDR + 0x404)
#define rDCS_MCU_AREFEN_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x408)
#define rDCS_MCU_AREFPARAM					(DCS_BASE_ADDR + 0x418)

#if (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)
#define rDCS_MCU_LONGSR						(DCS_BASE_ADDR + 0x420)
#define rDCS_MCU_ODTSZQC					(DCS_BASE_ADDR + 0x424)
#define rDCS_MCU_PWRMNGTEN					(DCS_BASE_ADDR + 0x428)
#define rDCS_MCU_PWRMNGTPARAM_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x42C)
#define rDCS_MCU_MCPHYUPDTPARAM					(DCS_BASE_ADDR + 0x440)
#define rDCS_MCU_READLEVELING					(DCS_BASE_ADDR + 0x444)
#define rDCS_MCU_FREQCHNGCTL					(DCS_BASE_ADDR + 0x450)
#define rDCS_MCU_FREQCHNGCTL_FREQ(_n,_f)			(DCS_BASE_ADDR + ((_n) * 4) + ((_f) * 0x18) + 0x454)
#define rDCS_MCU_FREQCHNGTIM_FREQ(_n)					(DCS_BASE_ADDR + ((_n) * 4) + 0x4B4)
#define rDCS_MCU_LS3B_QUIES_CTRL					(DCS_BASE_ADDR + 0x838)

#else
#define rDCS_MCU_LONGSR						(DCS_BASE_ADDR + 0x41C)
#define rDCS_MCU_ODTSZQC					(DCS_BASE_ADDR + 0x420)
#define rDCS_MCU_PWRMNGTEN					(DCS_BASE_ADDR + 0x424)
#define rDCS_MCU_PWRMNGTPARAM_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x428)
#define rDCS_MCU_MCPHYUPDTPARAM					(DCS_BASE_ADDR + 0x43C)
#define rDCS_MCU_READLEVELING					(DCS_BASE_ADDR + 0x440)
#define rDCS_MCU_FREQCHNGCTL					(DCS_BASE_ADDR + 0x44C)
#define rDCS_MCU_ROWHAM_CTL(_n)					(DCS_BASE_ADDR + ((_n) * 4) + 0x4B0)
#define rDCS_MCU_FREQCHNGCTL_FREQ(_n,_f)			(DCS_BASE_ADDR + ((_n) * 4) + ((_f) * 0x18) + 0x450)
#if DCS_REG_VERSION < 3
#define rDCS_MCU_FREQCHNGTIM_FREQ(_n)					(DCS_BASE_ADDR + ((_n) * 4) + 0x4C0)
#else
#define rDCS_MCU_FREQCHNGTIM_FREQ(_n)					(DCS_BASE_ADDR + ((_n) * 4) + 0x4C4)
#endif // #if DCS_REG_VERSION < 3
#endif // #if (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)

#define rDCS_MCU_MRINITCMD_0					(DCS_BASE_ADDR + 0x500)
#define rDCS_MCU_MRINITSTS_0					(DCS_BASE_ADDR + 0x504)
#define rDCS_MCU_INTSTS_0					(DCS_BASE_ADDR + 0x708)
#define rDCS_MCU_MIFQMAXCTRL_FREQ(_f)				(DCS_BASE_ADDR + ((_f) * 4) + 0x780)
#define rDCS_MCU_MRINITCMD(_c)					DCS_REG_READ_CH(_c,rDCS_MCU_MRINITCMD_0)
#define rDCS_MCU_MRINITSTS(_c)					DCS_REG_READ_CH(_c,rDCS_MCU_MRINITSTS_0)
#define rDCS_MCU_INTSTS(_c)					DCS_REG_READ_CH(_c,rDCS_MCU_INTSTS_0)

#define MRCMD_POLL_BIT						(0x100)

#define MRCMD_RANK_SHIFT					(13)
#define MRCMD_CMDTYPE_SHIFT					(8)
#define MRCMD_REG_SHIFT						(16)
#define MRCMD_DATA_SHIFT					(24)

#define MRCMD_TYPE(_h)						(((_h) & 0xFF) << MRCMD_CMDTYPE_SHIFT)
#define MRCMD_TYPE_READ						MRCMD_TYPE(0x11)
#define MRCMD_TYPE_WRITE					MRCMD_TYPE(0x01)
#define MRCMD_TYPE_MPC						MRCMD_TYPE(0x41)

/* SPLLCTRL registers */
#define rDCS_SPLLCTRL_MODEREG					(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x000)
#define rDCS_SPLLCTRL_CHARGEPUMP				(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x01C)
#define rDCS_SPLLCTRL_VCO					(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x020)
#define rDCS_SPLLCTRL_LDO					(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x024)
#define rDCS_SPLLCTRL_SPLLPWRDNCFG				(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x05C)
#define rDCS_SPLLCTRL_SPLLDIVCFG				(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x060)

#if DCS_REG_VERSION >= 2
#define rDCS_SPLLCTRL_MDLLPWRDNCFG(_n)				(DCS_BASE_ADDR + SPLLCTRL_SPACING + ((_n) * 4) + 0x064)
#endif

#if DCS_REG_VERSION >= 0x10
#define rDCS_SPLLCTRL_RECONFIGSWRESETTRIGGER			(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x074)
#define rDCS_SPLLCTRL_RECONFIGSWRESETCONTROL			(DCS_BASE_ADDR + SPLLCTRL_SPACING + 0x078)
#endif

/* AMP registers */

/* _t: type, DQ0, DQ1, or CA */
/* _c: channel, 0 or 1 */
#define AMP_CA			(0)
#define AMP_DQ0			(1)
#define AMP_DQ1			(2)
#define DCS_AMP_NUM_PHYS	(3)

#define AMP_IFF(__t,_t,_off)	(((__t) == (_t))? _off:0)

#define rDCS_AMP_AMPEN(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x000)
#define rDCS_AMP_AMPCLK(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x004)
#define rDCS_AMP_AMPINIT(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x008)
#define rDCS_AMP_DRAM_RESETN(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x00C)	// Only for AMPCA
#define rDCS_AMP_AMPVER(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + AMP_IFF(_t,0,4) + 0x00C)

// for AMPCA Only
#define rDCS_AMP_IMPCODE					(AMP_BASE_ADDR + 0x080)

#if (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)
#define rDCS_AMP_IMPAUTOCAL					(AMP_BASE_ADDR + 0x084)
#define rDCS_AMP_IMPCALCONTROL					(AMP_BASE_ADDR + 0x088)
#define rDCS_AMP_IMPCALPHYUPDT					(AMP_BASE_ADDR + 0x08C)
#define rDCS_AMP_IMPCALCMD					(AMP_BASE_ADDR + 0x090)

#else
#define rDCS_AMP_IMPOVRRD					(AMP_BASE_ADDR + 0x084)
#define rDCS_AMP_IMPAUTOCAL					(AMP_BASE_ADDR + 0x088)
#define rDCS_AMP_IMPCALCONTROL					(AMP_BASE_ADDR + 0x08C)
#define rDCS_AMP_IMPCALPHYUPDT					(AMP_BASE_ADDR + 0x090)
#define rDCS_AMP_IMPCALCMD					(AMP_BASE_ADDR + 0x094)
#define rDCS_AMP_NONDQDSPD_F(_f)				(AMP_BASE_ADDR + 0x09C + ((_f) * 4))
#define rDCS_AMP_NONDQDS_F(_f)					(AMP_BASE_ADDR + 0x0AC + ((_f) * 4))
#define rDCS_AMP_CACKCSWKPD_F(_f)				(AMP_BASE_ADDR + 0x0BC + ((_f) * 4))
#define rDCS_AMP_CACKCSWKDS					(AMP_BASE_ADDR + 0x0CC)
#define rDCS_AMP_BISTRXMODE					(AMP_BASE_ADDR + 0x0D4)
#endif

#if (DCS_REG_VERSION == 3 || DCS_REG_VERSION >= 0x10)
#define rDCS_AMP_DCCCONTROL					(AMP_BASE_ADDR + 0x0DC)
#define rDCS_AMP_DCCTIMER					(AMP_BASE_ADDR + 0x0E0)
#define rDCS_AMP_IMPCALCONTROL2				(AMP_BASE_ADDR + 0x0E8)
#if DCS_REG_VERSION >= 0x10
#define rDCS_AMP_IMPCALCONTROL3				(AMP_BASE_ADDR + 0x0EC)
#endif // #if DCS_REG_VERSION >= 0x10

#elif (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)
#define rDCS_AMP_DCCCONTROL					(AMP_BASE_ADDR + 0x098)
#define rDCS_AMP_DCCTIMER					(AMP_BASE_ADDR + 0x09C)
#endif

// For AMPDQn
#define rDCS_AMP_DRAMSIGDLY(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x080 + AMP_IFF(_t,0,0x18))
#define rDCS_AMP_DQDQSDS_F(_t,_f)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x084 + ((_f) * 4))
#define rDCS_AMP_DIFFMODE_FREQ(_t,_f)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x094 + ((_f) * 4))
#define rDCS_AMP_DQFLTCTRL(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x0A4)			// For AMPDQn only
#define rDCS_AMP_DCC(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x0A8 + AMP_IFF(_t,0,0x28))

#if DCS_REG_VERSION < 4
#define rDCS_AMP_DQDS_F(_t,_f)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x0AC + ((_f) * 4))
#endif

#define rDCS_AMP_RDCAPCFG_FREQ(_t,_f)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x100 + ((_f) * 4))
#define rDCS_AMP_DQSPDENALWYSON(_t) 				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x110)
#define rDCS_AMP_DQSPDRES(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x114)
#define rDCS_AMP_RDFIFOPTRSTS(_t) 				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x118)
#define rDCS_AMP_CALDQMSK(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x11C)
#define rDCS_AMP_CALPATCFG(_t)	 				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x120 - AMP_IFF(_t,0,0x10))
#define rDCS_AMP_DQTRNCFG(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x124 - AMP_IFF(_t,0,0x10))
#define rDCS_AMP_DQTRNCMD(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x128 - AMP_IFF(_t,0,0x10))
#define rDCS_AMP_DQTRNSTS(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x12C - AMP_IFF(_t,0,0x10))

#define rDCS_AMP_DLLEN(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x180)
#define rDCS_AMP_MDLLFREQBINDIS(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x184)
#define rDCS_AMP_MDLLFREQSCALE(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x188)
#define rDCS_AMP_DLLUPDTCMD(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x18C)
#define rDCS_AMP_MDLLOVRRD(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x190 + AMP_IFF(_t,0,4))
#define rDCS_AMP_MDLLCODE(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x194 - AMP_IFF(_t,0,4))
#define rDCS_AMP_DLLLOCKTIM(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x198)
#define rDCS_AMP_DLLLOCKCTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x19C)

#if (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)
#define rDCS_AMP_DLLUPDTINTVL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1A0 - AMP_IFF(_t,0,4))
#define rDCS_AMP_DLLUPDTCTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1A4 - AMP_IFF(_t,0,4))
#define rDCS_AMP_CAOUTDLLSCL_FREQ(_f)				(AMP_BASE_ADDR + 0x1A4 + ((_f) * 4))				// For AMPCA Only
#define rDCS_AMP_MDLLCODE_CAP_CNTL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1BC)
#define rDCS_AMP_MDLL_VTSCALE_CNTL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1C0)
#define rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1C4)
#define rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1C8)
#define rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1CC)
#define rDCS_AMP_CA_VTSCALE_FACTOR_STAT(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1D0)
#define rDCS_AMP_RD_VTSCALE_FACTOR_STAT(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1D4)
#define rDCS_AMP_WR_VTSCALE_FACTOR_STAT(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1D8)

#else
#define rDCS_AMP_DLLUPDTINTVL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1A0)
#define rDCS_AMP_DLLUPDTCTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1A4)
#define rDCS_AMP_DQSINDLLSCL_FREQ(_t,_f)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1BC + ((_f) * 4))
#define rDCS_AMP_CAOUTDLLSCL_FREQ(_f)				(AMP_BASE_ADDR + 0x1C0 + ((_f) * 4))				// For AMPCA Only
#define rDCS_AMP_MDLLCODE_CAP_CNTL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1CC)
#define rDCS_AMP_MDLL_VTSCALE_CNTL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1D0)
#define rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1D4)
#define rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1D8)
#define rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1DC)
#define rDCS_AMP_CA_VTSCALE_FACTOR_STAT(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1E0)
#define rDCS_AMP_RD_VTSCALE_FACTOR_STAT(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1E4)
#define rDCS_AMP_WR_VTSCALE_FACTOR_STAT(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x1E8)
#endif

#define rDCS_AMP_TESTMODE(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x200 + AMP_IFF(_t,0,0x180))

// For AMPDQn Only
#define rDCS_AMP_WRDQDESKEW_CTRL(_t,_d)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x400 + ((_d) * 4))
#define rDCS_AMP_WRDQSDESKEW_CTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x420)
#define rDCS_AMP_WRDMDESKEW_CTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x424)
#define rDCS_AMP_RDDQDESKEW_CTRL(_t,_d)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x428 + ((_d) * 4))
#define rDCS_AMP_RDDQSDESKEW_CTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x448)
#define rDCS_AMP_RDDMDESKEW_CTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x44C)

#define rDCS_AMP_WRDQDQSSDLLCTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x600)
#define rDCS_AMP_WRDQDQSSDLLCODE(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x604)
#define rDCS_AMP_RDSDLLCTRL(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x608)
#define rDCS_AMP_RDSDLLCODE(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x60C)
#define rDCS_AMP_RDCAPSDLLCTRL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x610)
#define rDCS_AMP_RDCAPSDLLCODE(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x614)
#define rDCS_AMP_RDDQSDLL_DLYSEL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x618)
#define rDCS_AMP_SDLL_UPDATE_CNTL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x61c + AMP_IFF(_t,0,0xFC))
#define rDCS_AMP_DQSDQ_SKEWCTL(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x620)
#define rDCS_AMP_DQSDQ_SKEW(_t)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x624)
#define rDCS_AMP_SDLL_UPDATE_DEFER_EN(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x628 + AMP_IFF(_t,0,0xF4))

#define rDCS_AMP_WRLVL_DLYCHAIN_CTRL(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x700)
#define rDCS_AMP_WRLVL_SDLL_CODE0(_t)				(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0x704)

// For AMPCA Only
#define rDCS_AMP_CADESKEW_CTRL(_d)				(AMP_BASE_ADDR + 0x500 + ((_d) * 4))
#define rDCS_AMP_CSDESKEW_CTRL					(AMP_BASE_ADDR + 0x518)
#define rDCS_AMP_CKDESKEW_CTRL					(AMP_BASE_ADDR + 0x51C)

#define rDCS_AMP_CASDLLCTRL					(AMP_BASE_ADDR + 0x700)
#define rDCS_AMP_CASDLLCODE					(AMP_BASE_ADDR + 0x704)
#define rDCS_AMP_CSSDLLCTRL					(AMP_BASE_ADDR + 0x708)
#define rDCS_AMP_CSSDLLCODE					(AMP_BASE_ADDR + 0x70C)
#define rDCS_AMP_CKSDLLCTRL					(AMP_BASE_ADDR + 0x710)
#define rDCS_AMP_CKSDLLCODE					(AMP_BASE_ADDR + 0x714)

// Some bit definitions
#define RUNSDLLUPDWRRESULT					(1 << 2)
#define SDLLOVRVAL_MASK					(0x00FF0000)
#define WRLVLRUNUPDWRRESULT					(1 << 9)
#define WRLVLSDLLCODE_MASK					(0x000000FF)
#define RUNDESKEWUPD						(1 << 8)
#define DESKEWCODE_MASK					(0x0000003F)
#define VREFSEL_MASK						(0x000000FF)
#define DQSVREFSEL_MASK					(0x00FF0000)
#define VTSCALEREFUPDATE					(1 << 28)
#define VTSCALEREFSTATUS_MASK					(0x000003FF)
#define VTSCALEREFSTATUS_SHIFT					(0)
#define VTSCALEREFOVERRIDEVAL_MASK				(0x03FF0000)
#define VTSCALEREFOVERRIDEVAL_SHIFT				(16)

// #define rDCS_AMP_SDLL_UPDATE_CNTL				rDCS_AMP_SDLL_UPDATE_CNTL(0)		// 0x718
// #define rDCS_AMP_SDLL_UPDATE_DEFER_EN			rDCS_AMP_SDLL_UPDATE_DEFER_EN(0)	// 0x71C
#define rDCS_AMP_MCCPWRONWAYCNTCTRL				(AMP_BASE_ADDR + 0x798)
#define rDCS_AMP_MCCPWRONWAYCNTSTAT				(AMP_BASE_ADDR + 0x79c)
#define rDCS_AMP_MCCALCHINT					(AMP_BASE_ADDR + 0x7c8)

#define rDCS_AMP_CAWRLVLSDLLCODE				(AMP_BASE_ADDR + 0x800)

#define rDCS_AMP_ODTENABLE_F(_f)				(AMP_BASE_ADDR + 0xD00 + ((_f) * 4))

#if (DCS_REG_VERSION >= 4 && DCS_REG_VERSION < 0x10)
#define rDCS_AMP_VREF_F(_t,_f)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0xB00 + AMP_IFF(_t,0,0x200) + ((_f) * 4))
#else
#define rDCS_AMP_VREF_F(_t,_f)					(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0xB00 + AMP_IFF(_t,0,0x210) + ((_f) * 4))
#endif

#define rDCS_AMPDQ_RDDQSMULFACTOR(_t)			(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0xD00)
#define rDCS_AMPDQ_DFICALTIMING_F(_t,_f)		(AMP_BASE_ADDR + ((_t)*AMP_DQ_SPACING) + 0xD80 + ((_f) * 4))
#define rDCS_AMP_RDWRDQCALTIMING_F(_f)				(AMP_BASE_ADDR + 0xE00 + ((_f) * 8))
#define rDCS_AMP_RDWRDQCALSEGLEN_F(_f)				(AMP_BASE_ADDR + 0xE04 + ((_f) * 8))
#define rDCS_AMP_HWRDWRDQCALTIMINGCTRL(_n)			(AMP_BASE_ADDR + 0xE20 + ((_n) * 4))
#define rDCS_AMP_HWRDDQCALPATPRBS4I				(AMP_BASE_ADDR + 0xE2C)
#define rDCS_AMP_HWWRDQCALPATPRBS4I				(AMP_BASE_ADDR + 0xE30)
#define rDCS_AMP_HWRDWRDQCALFULLSCANEN				(AMP_BASE_ADDR + 0xE34)
#define rDCS_AMP_RDDQCALWINDOW_F(_f)				(AMP_BASE_ADDR + 0xE38 + ((_f) * 8))
#define rDCS_AMP_WRDQCALWINDOW_F(_f)				(AMP_BASE_ADDR + 0xE3C + ((_f) * 8))

#define rDCS_AMP_DFICALTIMING					(AMP_BASE_ADDR + 0xE74)
#define rDCS_AMP_MAXWRDQ_SDLL_MULFACTOR				(AMP_BASE_ADDR + 0xE78)
#define rDCS_AMP_MAXRDDQS_SDLL_MULFACTOR			(AMP_BASE_ADDR + 0xE7C)
#define rDCS_AMP_MAXWRDQS_SDLL_MULFACTOR			(AMP_BASE_ADDR + 0xE80)
#define rDCS_AMPCA_DFICALTIMING_F(_f)			(AMP_BASE_ADDR + 0xF80 + ((_f - 1) * 4))
#define rDCS_AMPCA_DFICALTIMING_RDDQCAL2			(AMP_BASE_ADDR + 0xF90 )

#if DCS_REG_VERSION >= 3
#define rDCS_AMP_HWWRDQCALPATPRBS7(_n)			(AMP_BASE_ADDR + 0xEA0 + ((_n) * 4))
#define rDCS_AMP_HWWRDQCALPATINVERTMASK		(AMP_BASE_ADDR + 0xEB0)
#define rDCS_AMP_HWWRDQCALDYNHALFCLKDLYCTRL		(AMP_BASE_ADDR + 0xECC)
#define rDCS_AMP_HWRDDQCALVREFCONTROL                  (AMP_BASE_ADDR + 0xED8)
#define rDCS_AMP_HWRDDQCALTVREF                        (AMP_BASE_ADDR + 0xEDC)
#define rDCS_AMP_HWRDDQCALVREFOFFSETCONTROL(_n)        (AMP_BASE_ADDR + 0xEE0 + ((_n - 1) * 4))
#define rDCS_AMP_HWWRDQCALVREFCONTROL                  (AMP_BASE_ADDR + 0xEE8)
#define rDCS_AMP_HWWRDQCALTVREF                        (AMP_BASE_ADDR + 0xEEC)
#define rDCS_AMP_HWWRDQCALVREFOFFSETCONTROL(_n)        (AMP_BASE_ADDR + 0xEF0 + ((_n - 1) * 4))
#define rDCS_AMP_DQCALBURSTLENMODE			(AMP_BASE_ADDR + 0xF10)
#define rDCS_AMP_RDDQCALVREFCODECONTROL		(AMP_BASE_ADDR + 0xF18)
#define rDCS_AMP_RDDQCALVREFCODESTATUS			(AMP_BASE_ADDR + 0xF1C)
#define rDCS_AMP_WRDQCALVREFCODECONTROL		(AMP_BASE_ADDR + 0xF20)
#define rDCS_AMP_WRDQCALVREFCODESTATUS			(AMP_BASE_ADDR + 0xF24)
#endif

#if DCS_REG_VERSION >= 2
// AMPH
#define rDCS_AMPH_ZCAL_FSM(_n)			(AMPH_BASE_ADDR + (_n * 4) + 0x008)
#define rDCS_AMPH_DBG_REG0			(AMPH_BASE_ADDR + 0x024)
#define rDCS_AMPH_CB_DRIVESTR			(AMPH_BASE_ADDR + 0x028)
#define rDCS_AMPH_CB_IMPCTL			(AMPH_BASE_ADDR + 0x02C)
#define rDCS_AMPH_CB_WKPUPD			(AMPH_BASE_ADDR + 0x030)
#define rDCS_AMPH_CB_IOCTL			(AMPH_BASE_ADDR + 0x044)
#define rDCS_AMPH_CK_DRIVESTR			(AMPH_BASE_ADDR + 0x050)
#define rDCS_AMPH_CK_WKPUPD			(AMPH_BASE_ADDR + 0x058)
#define rDCS_AMPH_CK_IOCTL			(AMPH_BASE_ADDR + 0x060)
#define rDCS_AMPH_CK_ZDETBIASEN		(AMPH_BASE_ADDR + 0x068)
#define rDCS_AMPH_B0_DRIVESTR			(AMPH_BASE_ADDR + 0x078)
#define rDCS_AMPH_B0_IMPCTL			(AMPH_BASE_ADDR + 0x07C)
#define rDCS_AMPH_B0_WKPUPD			(AMPH_BASE_ADDR + 0x080)
#define rDCS_AMPH_B0_DYN_ISEL			(AMPH_BASE_ADDR + 0x098)
#define rDCS_AMPH_B0_DYN_ISEL_ASRTIME		(AMPH_BASE_ADDR + 0x09C)
#define rDCS_AMPH_B0_IOCTL			(AMPH_BASE_ADDR + 0x0A0)
#define rDCS_AMPH_B0_ODT			(AMPH_BASE_ADDR + 0x0B0)
#define rDCS_AMPH_B0_ODTCTRL			(AMPH_BASE_ADDR + 0x0B4)
#define rDCS_AMPH_DQS0_DRIVESTR		(AMPH_BASE_ADDR + 0x0C0)
#define rDCS_AMPH_DQS0_IMPCTL			(AMPH_BASE_ADDR + 0x0C4)
#define rDCS_AMPH_DQS0_WKPUPD			(AMPH_BASE_ADDR + 0x0C8)
#define rDCS_AMPH_DQS0_DCD_IOCTL		(AMPH_BASE_ADDR + 0x0CC)
#define rDCS_AMPH_DQS0_IOCTL			(AMPH_BASE_ADDR + 0x0D0)
#define rDCS_AMPH_DQS0_ODT			(AMPH_BASE_ADDR + 0x0D8)
#define rDCS_AMPH_DQS0_ZDETBIASEN		(AMPH_BASE_ADDR + 0x0DC)
#define rDCS_AMPH_DQS0_ODTCTRL			(AMPH_BASE_ADDR + 0x0E0)
#define rDCS_AMPH_B1_DRIVESTR			(AMPH_BASE_ADDR + 0x0E8)
#define rDCS_AMPH_B1_IMPCTL			(AMPH_BASE_ADDR + 0x0EC)
#define rDCS_AMPH_B1_WKPUPD			(AMPH_BASE_ADDR + 0x0F0)
#define rDCS_AMPH_B1_DYN_ISEL			(AMPH_BASE_ADDR + 0x108)
#define rDCS_AMPH_B1_IOCTL			(AMPH_BASE_ADDR + 0x10C)
#define rDCS_AMPH_B1_ODT			(AMPH_BASE_ADDR + 0x11C)
#define rDCS_AMPH_B1_ODTCTRL			(AMPH_BASE_ADDR + 0x120)
#define rDCS_AMPH_DQS1_DRIVESTR		(AMPH_BASE_ADDR + 0x130)
#define rDCS_AMPH_DQS1_IMPCTL			(AMPH_BASE_ADDR + 0x134)
#define rDCS_AMPH_DQS1_WKPUPD			(AMPH_BASE_ADDR + 0x138)
#define rDCS_AMPH_DQS1_DCD_IOCTL		(AMPH_BASE_ADDR + 0x13C)
#define rDCS_AMPH_DQS1_IOCTL			(AMPH_BASE_ADDR + 0x140)
#define rDCS_AMPH_DQS1_ODT			(AMPH_BASE_ADDR + 0x148)
#define rDCS_AMPH_DQS1_ODTCTRL			(AMPH_BASE_ADDR + 0x14C)
#define rDCS_AMPH_DQS1_ZDETBIASEN		(AMPH_BASE_ADDR + 0x150)
#define rDCS_AMPH_DCC_CAL			(AMPH_BASE_ADDR + 0x290)
#define rDCS_AMPH_SPARE(_n)			(AMPH_BASE_ADDR + (_n * 4) + 0x2FC)
#define rDCS_AMPH_DCC_BYPCA			(AMPH_BASE_ADDR + 0x29C)
#define rDCS_AMPH_DCC_BYPCK			(AMPH_BASE_ADDR + 0x2A0)
#define rDCS_AMPH_DCC_BYPCS			(AMPH_BASE_ADDR + 0x2A4) 
#define rDCS_AMPH_DCC_BYPB0			(AMPH_BASE_ADDR + 0x2A8)
#define rDCS_AMPH_DCC_BYPDQS0			(AMPH_BASE_ADDR + 0x2AC) 
#define rDCS_AMPH_DCC_BYPB1			(AMPH_BASE_ADDR + 0x2B0) 
#define rDCS_AMPH_DCC_BYPDQS1			(AMPH_BASE_ADDR + 0x2B4) 
 
#else
#define rDCS_AMPH_SLC_REG(_n)			(AMPH_BASE_ADDR + (_n * 4) + 0x080)
#endif // #if DCS_REG_VERSION >= 2

#define AMP_MAX_RANKS_PER_CHAN		(2)
#define DQ_NUM_BITS_PER_BYTE	8

// Global Timer Registers
#define rGLBTIMER_MDLLTIMER                        		(GLBTIMER_BASE_ADDR + 0x000)
#define rGLBTIMER_MDLLVOLTRAMPTIMER                		(GLBTIMER_BASE_ADDR + 0x004)
#define rGLBTIMER_CTRLUPDMASKTIMER                 		(GLBTIMER_BASE_ADDR + 0x008)
#define rGLBTIMER_IDTTIMER                         		(GLBTIMER_BASE_ADDR + 0x00c)
#define rGLBTIMER_RDCALTIMER                       		(GLBTIMER_BASE_ADDR + 0x010)
#define rGLBTIMER_WRCALTIMER                       		(GLBTIMER_BASE_ADDR + 0x014)
#define rGLBTIMER_ZQCTIMER                         		(GLBTIMER_BASE_ADDR + 0x018)

#if DCS_REG_VERSION >= 0x10
#define rGLBTIMER_FREQCHNG_ACTIVE                   		(GLBTIMER_BASE_ADDR + 0x094)
#endif

#if DCS_REG_VERSION >= 3

#define rGLBTIMER_DCCTIMER					(GLBTIMER_BASE_ADDR + 0x01c)
#define rGLBTIMER_PERCAL_FREQCHNGTIMER             		(GLBTIMER_BASE_ADDR + 0x020)
#define rGLBTIMER_VOLTRAMPTIMER                    		(GLBTIMER_BASE_ADDR + 0x024)
#define rGLBTIMER_IMPCALTIMER                      		(GLBTIMER_BASE_ADDR + 0x028)
#define rGLBTIMER_PERWRCALCFG                      		(GLBTIMER_BASE_ADDR + 0x02c)
#define rGLBTIMER_PERRDCALCFG                      		(GLBTIMER_BASE_ADDR + 0x030)
#define rGLBTIMER_VOLTRAMPCFG                      		(GLBTIMER_BASE_ADDR + 0x034)
#define rGLBTIMER_FREQCHNGCFG                      		(GLBTIMER_BASE_ADDR + 0x038)
#define rGLBTIMER_PMGRWAKEUPCFG                    		(GLBTIMER_BASE_ADDR + 0x03c)
#define rGLBTIMER_FREQBIN3CALCFG                   		(GLBTIMER_BASE_ADDR + 0x040)
#define rGLBTIMER_CHEN                             		(GLBTIMER_BASE_ADDR + 0x044)
#define rGLBTIMER_CAL2PREFREQCHNGDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x048)
#define rGLBTIMER_PREFREQ2ALLBANKDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x050)
#define rGLBTIMER_PREFREQCHNG2FREQCHNGDLY(_n)                  (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x058)
#define rGLBTIMER_FREQCHNG2PSTCALDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x060)
#define rGLBTIMER_VOLTRAMP2ALLBANKDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x068)
#define rGLBTIMER_ALLBANK2PMGRACKDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x070)
#define rGLBTIMER_PRECALWAITPERIOD                   		(GLBTIMER_BASE_ADDR + 0x078)
#define rGLBTIMER_CALSEG2ALLBANK(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x07C)
#define rGLBTIMER_ALLBANK2CALSEG(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x084)
#define rGLBTIMER_VREFCNTRL                   		(GLBTIMER_BASE_ADDR + 0x08C)
#define rGLBTIMER_PERVREFCALCFG                   		(GLBTIMER_BASE_ADDR + 0x090)

#else

#define rGLBTIMER_PERCAL_FREQCHNGTIMER             		(GLBTIMER_BASE_ADDR + 0x01c)
#define rGLBTIMER_VOLTRAMPTIMER                    		(GLBTIMER_BASE_ADDR + 0x020)
#define rGLBTIMER_IMPCALTIMER                      		(GLBTIMER_BASE_ADDR + 0x024)
#define rGLBTIMER_PERWRCALCFG                      		(GLBTIMER_BASE_ADDR + 0x028)
#define rGLBTIMER_PERRDCALCFG                      		(GLBTIMER_BASE_ADDR + 0x02c)
#define rGLBTIMER_VOLTRAMPCFG                      		(GLBTIMER_BASE_ADDR + 0x030)
#define rGLBTIMER_FREQCHNGCFG                      		(GLBTIMER_BASE_ADDR + 0x034)
#define rGLBTIMER_PMGRWAKEUPCFG                    		(GLBTIMER_BASE_ADDR + 0x038)
#define rGLBTIMER_FREQBIN3CALCFG                   		(GLBTIMER_BASE_ADDR + 0x03c)
#define rGLBTIMER_CHEN                             		(GLBTIMER_BASE_ADDR + 0x040)
#define rGLBTIMER_CAL2PREFREQCHNGDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x044)
#define rGLBTIMER_PREFREQ2ALLBANKDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x04C)
#define rGLBTIMER_PREFREQCHNG2FREQCHNGDLY(_n)                  (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x054)
#define rGLBTIMER_FREQCHNG2PSTCALDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x05C)
#define rGLBTIMER_VOLTRAMP2ALLBANKDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x064)
#define rGLBTIMER_ALLBANK2PMGRACKDLY(_n)                       (GLBTIMER_BASE_ADDR + ((_n) * 4) + 0x06C)

#endif // #if DCS_REG_VERSION >= 3

#endif /* _DCS_S8000_H */