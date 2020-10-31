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
#ifndef _AMP_T7000_H
#define _AMP_T7000_H

#define AMP_FREQUENCY_SLOTS	(4)

/* _t: type, DQ0, DQ1, or CA */
/* _c: channel, 0 or 1 */
#define AMP_DQ0			(0)
#define AMP_DQ1			(1)
#define AMP_CA			(2)

#define rAMP_AMPEN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x000)
#define rAMP_AMPCLK(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x004)
#define rAMP_AMPINIT(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x008)
#define rAMP_AMPVER(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x00C)

#define rAMP_IMPCODE(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x080)
#define rAMP_IMPOVRRD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x084)
#define rAMP_IMPAUTOCAL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x088)
#define rAMP_IMPCALPHYUPDT(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x08C)
#define rAMP_IMPCALCMD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x090)
#define rAMP_DRAMSIGDLY(_t,_c,_n)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x094 + ((_n) * 4))
#define rAMP_NONDQDS(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x098)
#define rAMP_DQDQSDS(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x09C)
#define rAMP_DIFFMODE_FREQ(_t,_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x0A0 + ((_f) * 4))
#define rAMP_DQFLTCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x0B0)

#define rAMP_RDCAPCFG_FREQ(_t,_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x100 + ((_f) * 4))
#define rAMP_DQSPDENALWYSON(_t,_c) 	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x110)
#define rAMP_DQSPDRES(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x114)
#define rAMP_RDFIFOPTRSTS(_t,_c) 	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x118)
#define rAMP_CALDQMSK(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x11C)
#define rAMP_CALPATCFG(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x120 - ((_t >> 1) * 0x10))
#define rAMP_DQTRNCFG(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x124 - ((_t >> 1) * 0x10))
#define rAMP_DQTRNCMD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x128 - ((_t >> 1) * 0x10))
#define rAMP_DQTRNSTS(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x12C - ((_t >> 1) * 0x10))

#define rAMP_DLLEN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x180)
#define rAMP_DLLUPDTCMD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x184)
#define rAMP_MDLLOVRRD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x188 + ((_t >> 1) * 4))
#define rAMP_MDLLCODE(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x18C - ((_t >> 1) * 4))
#define rAMP_DLLLOCKTIM(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x190)
#define rAMP_DLLLOCKCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x194)
#define rAMP_DLLUPDTINTVL(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x198)
#define rAMP_DLLUPDTCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x19C)
#define rAMP_DLLSTS(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x1A0)
#define rAMP_DQSINDLLSCL_FREQ(_t,_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x1B8 + ((_f) * 4))
#define rAMP_CAOUTDLLSCL_FREQ(_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x1B8 + ((_f) * 4))
// <rdar://problem/13919568>
#define rAMP_TESTMODE(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x200 + ((_t >> 1) * 0x180))

#define rAMP_WRDQDESKEW_CTRL(_t,_c,_r,_d)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x400 + ((_r)*0x28) + ((_d)*4))
#define rAMP_WRDQSDESKEW_CTRL(_t,_c,_r)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x420 + ((_r)*0x28))
#define rAMP_WRDMDESKEW_CTRL(_t,_c,_b)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x424 + ((_b)*0x28))
#define rAMP_RDDQDESKEW_CTRL(_t,_c,_r,_d)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x450 + ((_r)*0x28) + ((_d)*4))
#define rAMP_RDDQSDESKEW_CTRL(_t,_c,_r)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x470 + ((_r)*0x28))
#define rAMP_RDDMDESKEW_CTRL(_t,_c,_r)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x474 + ((_r)*0x28))

#define rAMP_CADESKEW_CTRL(_c,_d)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x500 + ((_d)*4))
#define rAMP_CSDESKEW_CTRL(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x528)
#define rAMP_CKEDESKEW_CTRL(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x52C)
#define rAMP_CKDESKEW_CTRL(_c,_d)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x530 + ((_d)*4))

#define rAMP_DQSDLLCTRL_WR(_t,_c,_b)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x600 + ((_b)*0x08))
#define rAMP_DQSDLLCTRL_RD(_t,_c,_b)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x610 + ((_b)*0x08))
#define rAMP_DQWRLVLTIM(_t,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x700)
#define rAMP_DQWRLVLDLYCHAINCTRL(_t,_c,_b)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x704 + ((_b)*0x04))
#define rAMP_DQWRLVLSDLLCODE(_t,_c,_b)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x70C + ((_b)*0x04))
#define rAMP_DQWRLVLRUN(_t,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x714)
#define rAMP_DQWRLVLDATA(_t,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x718)

#define rAMP_CASDLLCTRL(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x700)
#define rAMP_CAWRLVLSDLLCODE(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x800)
#define rAMP_CAWRLVLCLKDLYSEL(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x804)

#define rAMP_CACALCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x800 + ((_t >> 1) * 0x100))
#define rAMP_CACALMASK(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x804)
#define rAMP_CACALCABYTESEL(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x808)
#define rAMP_CACALCAMAP0(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x80C)
#define rAMP_CACALCAMAP1(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x810)
#define rAMP_CACALCAMAP2(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x814)
#define rAMP_CACALPAT(_t,_c,_p)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x818 + ((_t >> 1) * 0xEC) + ((_p)*0x04))
#define rAMP_CACALRUN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x838 + ((_t >> 1) * 0xEC))
#define rAMP_CACALRESULT(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x83C)

#define rAMP_DQCALCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x900)
#define rAMP_DQCALRUN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x908)
#define rAMP_DQCALRESULT(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0x90C)

#define rAMP_DQODTVREF_F(_t,_c,_f)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQCA_SPACING) + 0xB00 + ((_f)*0x04))

#define rAMP_CAPHYUPDTREQ(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0xB00)
#define rAMP_CAPHYUPDTSTATUS(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0xB04)

// Some bit definitions
#define CACALRUN_CACALMODE		(1 << 16)
#define CACALRUN_RUNCACAL		(1 << 0)
#define TESTMODE_FORCECKELOW		(1 << 8)

#define SIGN_BIT_POS			(6)

#define AMP_MAX_RANKS_PER_CHAN		(2)

#endif /* _AMP_T7000_H */
