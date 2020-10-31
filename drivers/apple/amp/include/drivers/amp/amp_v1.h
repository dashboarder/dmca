/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _AMP_V1_H
#define _AMP_V1_H

#include <platform/soc/hwregbase.h>

/* Apple Memory Phy */

#define AMP_NUM_DLL		(4)
#define AMP_FREQUENCY_SLOTS	(4)

#define AMP_INDEX_FOR_CHANNEL(_c)	(((_c) < 2) ? (_c) : ((_c) + 4))

#define rAMP_AMPEN(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x000))
#define rAMP_AMPCLK(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x004))
#define rAMP_AMPINIT(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x008))
#define rAMP_AMPVER(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x00C))

#define rAMP_IMPCODE(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x080))
#define rAMP_IMPOVRRD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x084))
#define rAMP_IMPAUTOCAL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x088))
#define rAMP_IMPCALPHYUPDT(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x08C))
#define rAMP_IMPCALCMD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x090))
#define rAMP_DRAMSIGDLY(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x094))
#define rAMP_DRAMSIGPH(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x098))
#define rAMP_DQDQSDS(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x09C))
#define rAMP_NONDQDS(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x0A0))
#define rAMP_DIFFMODE_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x0A4 + ((_f) * 4)))
#define rAMP_DQFLTCTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x0B4))

#define rAMP_RDCAPCFG_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x100 + ((_f) * 4)))
#define rAMP_DQSPDRES(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x110))
#define rAMP_CALDQMSK(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x114))
#define rAMP_CALPATCFG(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x118))
#define rAMP_GATETRNCMD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x11C))
#define rAMP_GATETRNRESLT(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x120))
#define rAMP_DQTRNCFG(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x124))
#define rAMP_DQTRNCMD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x128))
#define rAMP_DQTRNSTS(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x12C))

#define rAMP_DLLEN(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x180))
#define rAMP_DLLUPDTCMD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x184))
#define rAMP_MDLLCODE(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x188))
#define rAMP_DLLLOCKTIM(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x18C))
#define rAMP_DLLLOCKCTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x190))
#define rAMP_DLLUPDTINTVL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x194))
#define rAMP_DLLUPDTCTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x198))
#define rAMP_DLLSTS(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x19C))

#define rAMP_DQSINDLLCODE(_c,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x1A0 + ((_d) * 0x18)))
#define rAMP_DQSINDLLOVRRD(_c,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x1A4 + ((_d) * 0x18)))
#define rAMP_DQSINDLLSCL_FREQ(_c,_d,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x1A8 + ((_d) * 0x18) + ((_f) * 4)))

#define rAMP_DQOUTDLLCODE(_c,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x200 + ((_d) * 0x18)))
#define rAMP_DQOUTDLLOVRRD(_c,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x204 + ((_d) * 0x18)))
#define rAMP_DQOUTDLLSCL_FREQ(_c,_d,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x208 + ((_d) * 0x18) + ((_f) * 4)))

#define rAMP_CAOUTDLLCODE(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x260))
#define rAMP_CAOUTDLLOVRRD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x264))
#define rAMP_CAOUTDLLSCL_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x268 + ((_f) * 4)))

#define rAMP_CTLOUTDLLCODE(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x278))
#define rAMP_CTLOUTDLLOVRRD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x27C))
#define rAMP_CTLOUTDLLSCL_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x280 + ((_f) * 4)))

#define rAMP_CAPSTDLLCODE(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x290))
#define rAMP_CAPSTDLLOVRRD(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x294))
#define rAMP_CAPSTDLLSCL_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x298 + ((_f) * 4)))

#define rAMP_INTEN(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x400))
#define rAMP_INSTS(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x404))

struct amp_per_freq {
	uint32_t scl;
	uint32_t rdcapcfg;
};

struct amp_params {
	uint32_t flags;
	struct amp_per_freq freq[AMP_FREQUENCY_SLOTS];
};

#define FLAG_AMP_PARAM_SUPPORT_FMCLK_OFF	0x0001
#define FLAG_AMP_PARAM_FULLDIFFMODE	0x0002

#include <platform/ampconfig.h>

#endif /* ! _AMP_V1_H */
