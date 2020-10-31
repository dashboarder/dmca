/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _AMP_V2_H
#define _AMP_V2_H

#include <platform/soc/hwregbase.h>

#define AMP_FREQUENCY_SLOTS	(4)

/* _t: type, CA or DQ */
/* _c: channel, 0 or 1 */
#define AMP_DQ			(0)
#define AMP_CA			(1)

#ifndef AMP_REG_OFFSET
# define AMP_REG_OFFSET		(0)
#endif

#define rAMP_AMPEN(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x000))
#define rAMP_AMPCLK(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x004))
#define rAMP_AMPINIT(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x008))
#define rAMP_AMPVER(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x00C))

#define rAMP_IMPCODE(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x080))
#define rAMP_IMPOVRRD(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x084))
#define rAMP_IMPAUTOCAL(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x088))
#define rAMP_IMPCALPHYUPDT(_t,_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x08C))
#define rAMP_IMPCALCMD(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x090))
#define rAMP_DRAMSIGDLY(_t,_c,_n)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x094 + ((_n) * 4)))
#define rAMP_NONDQDS(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x098))
#define rAMP_DQDQSDS(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x0A4))
#define rAMP_DIFFMODE_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x0A8 + ((_f) * 4)))
#define rAMP_DQFLTCTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x0B8))

#define rAMP_RDCAPCFG_FREQ(_t,_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x100 + ((_f) * 4)))
#define rAMP_DQSPDENALWYSON(_c) 	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x110))
#define rAMP_DQSPDRES(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x114))
#define rAMP_RDFIFOPTRSTS(_c) 		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x118))
#define rAMP_CALDQMSK(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x11C))
#define rAMP_CALPATCFG(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x120) - ((_t)*0xC))
#define rAMP_DQTRNCFG(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x124) - ((_t)*0xC))
#define rAMP_DQTRNCMD(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x128) - ((_t)*0xC))
#define rAMP_DQTRNSTS(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x12C) - ((_t)*0xC))

#define rAMP_DLLEN(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x180))
#if SUB_PLATFORM_S7002
#define rAMP_MDLLFREQBINDISABLE(_t,_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x184))
#endif
#define rAMP_DLLUPDTCMD(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x184 + AMP_REG_OFFSET))
#define rAMP_MDLLOVRRD(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x04) + 0x188 + AMP_REG_OFFSET))
#define rAMP_MDLLCODE(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) - ((_t)*0x04) + 0x18C + AMP_REG_OFFSET))
#define rAMP_DLLLOCKTIM(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x190 + AMP_REG_OFFSET))
#define rAMP_DLLLOCKCTRL(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x194 + AMP_REG_OFFSET))
#define rAMP_DLLUPDTINTVL(_t,_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x198 + AMP_REG_OFFSET))
#define rAMP_DLLUPDTCTRL(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x19C + AMP_REG_OFFSET))
#define rAMP_DLLSTS(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + 0x1A0 + AMP_REG_OFFSET))
#define rAMP_DQSINDLLSCL_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x1B8 + ((_f) * 4) + AMP_REG_OFFSET))
#define rAMP_CAOUTDLLSCL_FREQ(_c,_f)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x1B8 + ((_f) * 4) + AMP_REG_OFFSET))

#define rAMP_TESTMODE(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x180) + 0x200))

#define rAMP_WRDQDESKEW_CTRL(_c,_r,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x400 + ((_r)*0x28) + ((_d)*4)))
#define rAMP_WRDQSDESKEW_CTRL(_c,_r)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x420 + ((_r)*0x28)))
#define rAMP_WRDMDESKEW_CTRL(_c,_b)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x424 + ((_b)*0x28)))
#define rAMP_RDDQDESKEW_CTRL(_c,_r,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x4A0 + ((_r)*0x28) + ((_d)*4)))
#define rAMP_RDDQSDESKEW_CTRL(_c,_r)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x4C0 + ((_r)*0x28)))
#define rAMP_RDDMDESKEW_CTRL(_c,_r)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x4C4 + ((_r)*0x28)))

#define rAMP_CADESKEW_CTRL(_c,_d)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + ((_d)*4) + 0x500))
#define rAMP_CSDESKEW_CTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x528))
#define rAMP_CKEDESKEW_CTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x52C))
#define rAMP_CKDESKEW_CTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x530))

#define rAMP_DQSDLLCTRL_WR(_c, _b)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b)*0x08) + 0x600))
#define rAMP_DQSDLLCTRL_RD(_c, _b)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b)*0x08) + 0x620))
#define rAMP_DQWRLVLTIM(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x700))
#define rAMP_DQWRLVLDLYCHAINCTRL(_c, _b)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b)*0x04) + 0x704))
#define rAMP_DQWRLVLSDLLCODE(_c, _b)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b)*0x04) + 0x714))
#define rAMP_DQWRLVLRUN(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x724))
#define rAMP_DQWRLVLDATA(_c)			(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x728))

#define rAMP_CASDLLCTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x700))
#define rAMP_CAWRLVLSDLLCODE(_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x800))
#define rAMP_CAWRLVLCLKDLYSEL(_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x804))

#define rAMP_CACALCTRL(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x100) + 0x800))
#define rAMP_CACALMASK(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x100) + 0x804))
#define rAMP_CACALBYTESEL(_t,_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x100) + 0x808))
#define rAMP_CACALBITSELMAP(_t,_c,_n)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x100) + ((_n)*0x04) + 0x80C))
#define rAMP_CACALPAT(_t,_c, _p)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x100) + ((_p)*0x04) + 0x818))
#define rAMP_CACALRUN(_t,_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_CA_SPACING) + ((_t)*0x100) + 0x838))
#define rAMP_CACALRESULT(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x83C))

#define rAMP_DQCALCTRL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x900))
#define rAMP_DQCALRUN(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x908))
#define rAMP_DQCALRESULT(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x90C))

#define rAMP_CAPHYUPDTCRTL(_c)		(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0xb00))
#define rAMP_CAPHYUPDTSTATUS(_c)	(*(volatile u_int32_t *)(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0xb04))

// Some bit definitions
#define CACALRUN_CACALMODE		(1 << 16)
#define CACALRUN_RUNCACAL		(1 << 0)
#define TESTMODE_FORCECKELOW		(1 << 8)

#define SIGN_BIT_POS			(6)

#define AMP_MAX_RD			(4)
#define AMP_MAX_DQ			(8)
#define AMP_MAX_RANKS_PER_CHAN		(2)

struct amp_per_freq {
	uint32_t caoutdllscl;
	uint32_t dqsindllscl;
	uint32_t rdcapcfg;
};

struct amp_params {
	struct amp_per_freq freq[AMP_FREQUENCY_SLOTS];
	uint32_t drive_strength;
	uint32_t cacalib_hw_loops;
	uint32_t cacalib_sw_loops;
	bool wrlvl_togglephyupdt;
	uint32_t imp_auto_cal;
};

#include <platform/ampconfig.h>

#endif /* _AMP_V2_H */
