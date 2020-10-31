/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* Apple Memory Controller version 3 */

#ifndef __PLATFORM_SOC_AMC_H
#define __PLATFORM_SOC_AMC_H

#include <drivers/amc/amc.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <sys/types.h>

#define AMC_FREQUENCY_SLOTS	(4)

// AMCX registers

#define rAMC_AMCEN		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x000))
#define rAMC_CLKGATE		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x004))
#define rAMC_FREQSEL		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x008))
#define rAMC_CLKPWRGATE		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x010))
#define rAMC_CLKPWRGATE2	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x014))

#define rAMC_MCSADDRBNKHASH(_n)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x080 + ((_n) * 4)))
#define rAMC_MCSCHNLDEC		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x08C))
#define rAMC_ADDRMAP_MODE	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x090))
#define rAMC_ADDRCFG		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x094))

#define rAMC_MCSRDRTNCRD	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x100))
#define rAMC_MCSCHNLCRD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x104))
#define rAMC_MCSCHNLTM		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x108))
#define rAMC_MCSARBCFG		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x10C))

#define rAMC_SCHEN		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x180))
#define rAMC_OPIDLETMR		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x184))
#define rAMC_OPTMRADJEN		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x188))
#define rAMC_OPTMRADJPARAM	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x18C))
#define rAMC_MIFNRTAGE		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x190))
#define rAMC_MIFRTAGE		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x194))
#define rAMC_MIFPCAGE		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x198))
#define rAMC_QBREN		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x19C))
#define rAMC_QBRPARAM		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1A0))
#define rAMC_MIFACTSCH		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1A4))
#define rAMC_MIFCASSCH_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x1A8))
#define rAMC_PSQAFNTY		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1B8))
#define rAMC_PSQRQCTL0		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1BC))
#define rAMC_PSQRQCTL1		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1C0))
#define rAMC_PSQRQTIMER0	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1C4))
#define rAMC_PSQRQTIMER1	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1C8))
#define rAMC_PSQRQTIMER2	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1CC))
#define rAMC_PSQRQBRST		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1D0))
#define rAMC_PSQRQSCHCRD	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1D4))
#define rAMC_PSQWQCTL0		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1D8))
#define rAMC_PSQWQCTL1		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1DC))
#define rAMC_PSQWQTHR		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1E0))
#define rAMC_PSQWQBRST		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1E4))
#define rAMC_PSQWQSCHCRD	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1E8))
#define rAMC_MCUQOS		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1EC))
#define rAMC_MCUQOSLLT		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1F0))
#define rAMC_PSQRQTIMER3	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x1F4))

#define rAMC_PHYRDWRTIM		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x200))
#define rAMC_PHYUPDATETIMERS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x280))

#define rAMC_LAT		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x300))
#define rAMC_CAS_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x304))
#define rAMC_PCH_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x314))
#define rAMC_ACT_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x324))
#define rAMC_AUTO_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x334))
#define rAMC_SELF_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x344))
#define rAMC_PDN		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x354))
#define rAMC_MODE_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x358))
#define rAMC_RD			(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x368))
#define rAMC_BUSTAT		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x36C))
#define rAMC_BUSTAT_FREQ01	rAMC_BUSTAT
#define rAMC_DERATE		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x370))
#define rAMC_TRANSGAP		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x374))
#define rAMC_AUTOREF_PARAMS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x378))
#define rAMC_BUSTAT_FREQ23	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x37C))

#define rAMC_CH0RNKCFG0		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x380))
#define rAMC_CH0RNKCFG1		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x384))
#define rAMC_CH1RNKCFG0		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x388))
#define rAMC_CH1RNKCFG1		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x38C))
#define rAMC_AREFEN_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x390))
#define rAMC_AREFPARAM		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3A0))
#define rAMC_LONGSR		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3A4))
#define rAMC_ZQC		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3A8))
#define rAMC_PWRMNGTEN		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3AC))
#define rAMC_PWRMNGTPARAM	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3B0))
#define rAMC_ODTS		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3B4))
#define rAMC_ODTS_MAPPING	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3B8))
#define rAMC_MCPHY_UPDTPARAM	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3BC))
#define rAMC_READ_LEVELING	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3C0))
#define rAMC_CH2RNKCFG0		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3C4))
#define rAMC_CH2RNKCFG1		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3C8))
#define rAMC_CH3RNKCFG0		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3CC))
#define rAMC_CH3RNKCFG1		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3D0))
#define AMC_RNKCFG_OFFSET(_r)	((_r) * 1)

// These defs are for AREFPARAM regs for freqs 1, 2, and 3 only (freq0 already defined above as rAMC_AREFPARAM)
#define rAMC_AREFPARAM_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x3D8))

#define rAMC_MCPHY_UPDTPARAM1	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x3F0))

#define rAMC_CH0MRCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x400))
#define rAMC_CH0INITCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x404))
#define rAMC_CH1MRCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x408))
#define rAMC_CH1INITCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x40C))

#if AMC_REG_VERSION == 4

#define rAMC_CH01MRSTATUS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x410))
#define rAMC_CH01INITSTATUS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x414))
#define rAMC_CH2MRCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x418))
#define rAMC_CH2INITCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x41C))
#define rAMC_CH3MRCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x420))
#define rAMC_CH3INITCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x424))

#else

#define rAMC_CH2MRCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x410))
#define rAMC_CH2INITCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x414))
#define rAMC_CH3MRCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x418))
#define rAMC_CH3INITCMD		(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x41C))
#define rAMC_CH01MRSTATUS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x420))
#define rAMC_CH01INITSTATUS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x424))

#endif

#define rAMC_CH23MRSTATUS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x428))
#define rAMC_CH23INITSTATUS	(*(volatile uint32_t *)(AMCX_BASE_ADDR + 0x42C))

#define rAMC_MIFQMAXCTRL_FREQ(_f)	(*(volatile uint32_t *)(AMCX_BASE_ADDR + ((_f) * 4) + 0x700))

// AMCC registers

#define rMCC_CHNLDEC		(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x4A4))
#define rMCC_MCUCHNHASH		(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x4A8))
#define rMCC_MCUCHNHASH2	(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x4AC))
#define rMCC_DRAMACCCTRL	(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x4B4))

#define rAMC_INTEN		(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x700))
#define rAMC_INTSTATUS		(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x704))
#define rAMC_MCCTAGPARLOG1	(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x710))
#define rAMC_MCCTAGPARLOG2      (*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x714))
#define rAMC_MCCDATERRLOG       (*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x718))
#define rAMC_MCCAFERRLOG0       (*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x71c))
#define rAMC_MCCAFERRLOG1       (*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x720))
#define rAMC_MCCAFERRLOG2       (*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x724))
#define rAMC_MCCDPERRCNTCTRL    (*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x728))

#define rMCC_MCCGEN		(*(volatile uint32_t *)(AMCC_BASE_ADDR + 0x780))

#include <platform/amcconfig.h>

#endif /* __PLATFORM_SOC_AMC_H */
