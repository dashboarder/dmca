/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* Apple Memory Controller */

#ifndef __PLATFORM_SOC_AMC_H
#define __PLATFORM_SOC_AMC_H

#include <drivers/amc/amc.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <sys/types.h>

#define AMC_FREQUENCY_SLOTS	(4)

#define rAMC_AMCEN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x000))
#define rAMC_BOOTCLK		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x004))
#define rAMC_CLKGATE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x008))
#define rAMC_FREQSEL		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x00C))
#define rAMC_CLKPWRGATE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x010))
#define rAMC_CLKPWRGATE2	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x014))

#define rAMC_MCSADDRBNKHASH(_n)	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x080 + ((_n) * 4)))

#define rAMC_MCSRDRTNCRD	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x100))
#define rAMC_MCSCHNLCRD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x104))
#define rAMC_MCSCHNLTM		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x108))
#define rAMC_MCSARBCFG		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x10C))

#define rAMC_SCHEN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x200))
#define rAMC_OPIDLETMR		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x204))
#define rAMC_OPTMRADJEN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x208))
#define rAMC_OPTMRADJPARAM	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x20C))
#define rAMC_MIFNRTAGE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x210))
#define rAMC_MIFRTAGE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x214))
#define rAMC_MIFPCAGE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x218))
#define rAMC_QBREN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x21C))
#define rAMC_QBRPARAM		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x220))
#define rAMC_MIFACTSCH		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x224))
#define rAMC_MIFCASSCH_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x228))
#define rAMC_PSQAFNTY		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x238))
#define rAMC_PSQRQCTL0		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x23C))
#define rAMC_PSQRQCTL1		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x240))
#define rAMC_PSQRQTIMER0	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x244))
#define rAMC_PSQRQTIMER1	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x248))
#define rAMC_PSQRQTIMER2	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x24C))
#define rAMC_PSQRQBRST		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x250))
#define rAMC_PSQRQSCHCRD	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x254))
#define rAMC_PSQWQCTL0		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x258))
#define rAMC_PSQWQCTL1		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x25C))
#define rAMC_PSQWQTHR		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x260))
#define rAMC_PSQWQBRST		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x264))
#define rAMC_PSQWQSCHCRD	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x268))
#define rAMC_MCUQOS		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x26C))
#define rAMC_MCUQOSLLT		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x270))
#define rAMC_PSQRQTIMER3	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x274))

#define rAMC_PHYRDWRTIM		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x280))

#define rAMC_PHYMRRDATADQBITMUX  (*(volatile uint32_t *)(AMC_BASE_ADDR + 0x300))
#define rAMC_PHYMRRDATADQBYTEMUX (*(volatile uint32_t *)(AMC_BASE_ADDR + 0x304))

#define rAMC_LAT		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x380))
#define rAMC_CAS_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x384))
#define rAMC_PCH_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x394))
#define rAMC_ACT_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x3A4))
#define rAMC_AUTO_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x3B4))
#define rAMC_SELF_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x3C4))
#define rAMC_PDN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3D4))
#define rAMC_MODE_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x3D8))
#define rAMC_RD			(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3E8))
#define rAMC_BUSTAT		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3EC))
#define rAMC_BUSTAT_FREQ01	rAMC_BUSTAT
#define rAMC_DERATE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3F0))
#define rAMC_TRANSGAP		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3F4))
#define rAMC_AUTOREF_PARAMS	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3F8))
#define rAMC_BUSTAT_FREQ23	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x3FC))

#define rAMC_ADDRMAP_MODE	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x400))
#define rAMC_ADDRCFG		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x404))
#define rAMC_CH0RNKCFG0		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x408))
#define rAMC_CH0RNKCFG1		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x40C))
#define rAMC_CH1RNKCFG0		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x410))
#define rAMC_CH1RNKCFG1		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x414))
#define rAMC_AREFEN_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x418))
#define rAMC_AREFPARAM		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x428))
#define rAMC_LONGSR		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x42C))
#define rAMC_ZQC		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x430))
#define rAMC_PWRMNGTEN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x434))
#define rAMC_PWRMNGTPARAM	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x438))
#define rAMC_ODTS		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x43C))
#define rAMC_ODTS_MAPPING	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x440))
#define rAMC_MCPHY_UPDTPARAM	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x444))
#define rAMC_READ_LEVELING	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x448))
#define rAMC_CH2RNKCFG0		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x44C))
#define rAMC_CH2RNKCFG1		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x450))
#define rAMC_CH3RNKCFG0		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x454))
#define rAMC_CH3RNKCFG1		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x458))
#define AMC_RNKCFG_OFFSET(_r)	((_r) * 1)

// These defs are for AREFPARAM regs for freqs 1, 2, and 3 only (freq0 already defined above as rAMC_AREFPARAM)
#define rAMC_AREFPARAM_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x460))

#define rAMC_MCPHY_UPDTPARAM1	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x478))

#define rAMC_CH0MRCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x480))
#define rAMC_CH0INITCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x484))
#define rAMC_CH1MRCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x488))
#define rAMC_CH1INITCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x48C))
#define rAMC_CH01MRSTATUS	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x490))
#define rAMC_CH01INITSTATUS	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x494))
#define rAMC_CH2MRCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x498))
#define rAMC_CH2INITCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x49C))
#define rAMC_CH3MRCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x4A0))
#define rAMC_CH3INITCMD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x4A4))
#define rAMC_CH23MRSTATUS	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x4A8))
#define rAMC_CH23INITSTATUS	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x4AC))

#define rAMC_INTEN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x700))
#define rAMC_INTSTATUS		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x704))


#define rAMC_MIFQMAXCTRL_FREQ(_f)	(*(volatile uint32_t *)(AMC_BASE_ADDR + ((_f) * 4) + 0x780))	

#define rMCC_MCCEN		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x800))
#define rMCC_CHNLDEC		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x80C))
#define rAMC_CHNLDEC		rMCC_CHNLDEC
#define rMCC_MCUCHNHASH		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x810))
#define rMCC_DRAMACCCTRL	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x81C))
#define rMCC_ALCHINT		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x850))
#define rMCC_AFWRMERGE		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x854))

#define rMCC_MCSCMDCRD		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x880))
#define rMCC_MCCAFDTHR		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x884))
#define rMCC_MCDHWMTHR		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x888))
#define rMCC_MCFTHR		(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x88C))

#define rMCC_DATASETID(_n)	(*(volatile uint32_t *)(AMC_BASE_ADDR + 0x1000 + ((_n) * 0x1000)))

#include <platform/amcconfig.h>

#endif /* __PLATFORM_SOC_AMC_H */
