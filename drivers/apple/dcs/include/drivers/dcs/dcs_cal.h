/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _DCS_CAL_H
#define _DCS_CAL_H

#define AMP_FREQUENCY_SLOTS	(4)

/* _t: type, DQ0, DQ1, or CA */
/* _c: channel, 0 or 1 */
#define AMP_CA			(0)
#define AMP_DQ0			(1)
#define AMP_DQ1			(2)

#define rAMP_AMPEN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x000)
#define rAMP_AMPCLK(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x004)
#define rAMP_AMPINIT(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x008)
#define rAMP_AMPVER(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x00C)

#define rAMP_IMPCODE(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x080)
#define rAMP_IMPOVRRD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x084)
#define rAMP_IMPAUTOCAL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x088)
#define rAMP_IMPCALPHYUPDT(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x08C)
#define rAMP_IMPCALCMD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x090)
#define rAMP_DRAMSIGDLY(_t,_c,_n)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x094 + ((_n) * 4))
#define rAMP_NONDQDS(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x098)
#define rAMP_DQDQSDS(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x09C)
#define rAMP_DIFFMODE_FREQ(_t,_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x0A0 + ((_f) * 4))
#define rAMP_DQFLTCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x0B0)

#define rAMP_DQSPDENALWYSON(_t,_c) 	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x110)
#define rAMP_DQSPDRES(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x114)
#define rAMP_RDFIFOPTRSTS(_t,_c) 	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x118)
#define rAMP_CALDQMSK(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x11C)
#define rAMP_DQTRNCFG(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x124 - ((_t >> 1) * 0x10))
#define rAMP_DQTRNCMD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x128 - ((_t >> 1) * 0x10))
#define rAMP_DQTRNSTS(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x12C - ((_t >> 1) * 0x10))

#define rAMP_DLLEN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x180)
#define rAMP_MDLLOVRRD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x188 + ((_t >> 1) * 4))

#define rAMP_DLLLOCKTIM(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x198)
#define rAMP_DLLLOCKCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x19C)
#define rAMP_DLLUPDTINTVL(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x1A0)
#define rAMP_DLLUPDTCTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x1A4)
#define rAMP_DLLSTS(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x1A8)
#define rAMP_DQSINDLLSCL_FREQ(_t,_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x1C0 + ((_f) * 4))
#define rAMP_CAOUTDLLSCL_FREQ(_c,_f)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x1C0 + ((_f) * 4))
// <rdar://problem/13919568>
#define rAMP_TESTMODE(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x200 + ((_t >> 1) * 0x180))

#define rAMP_CKEDESKEW_CTRL(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0x52C)

#define rAMP_DQWRLVLDLYCHAINCTRL(_t,_c,_b)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x704 + ((_b)*0x04))
#define rAMP_DQWRLVLRUN(_t,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x714)
#define rAMP_DQWRLVLDATA(_t,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x718)


#define rAMP_CACALMASK(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x804)
#define rAMP_CACALCABYTESEL(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x808)
#define rAMP_CACALCAMAP0(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x80C)
#define rAMP_CACALCAMAP1(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x810)
#define rAMP_CACALCAMAP2(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x814)
//#define rAMP_CACALRUN(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x838 + ((_t >> 1) * 0xEC))
// CP  : Maui Change
//#define rAMP_CACALRESULT(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x83C)


#define rAMP_CAPHYUPDTREQ(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0xB00)
#define rAMP_CAPHYUPDTSTATUS(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + AMP_CA_SPACING + 0xB04)
#define rAMP_DLLUPDTCMD(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0x18C)

// Some bit definitions
// CP : Maui Change
#define CACALRUN_CACALMODE		1
#define rAMP_CACACALVREF(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x908)
#define rAMP_CACALBITSELECT(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x918)
#define CS_TRAINING 0
#define CA_TRAINING 1
#define CS_FINE_TRAINING 2
#define RUN_DLL_UPDT 0

#define CACALRUN_RUNCACALENTRY	(1 << 0)
#define CACALRUN_RUNCACALCS		(1 << 4)
#define CACALRUN_RUNCACALCA		(1 << 8)
#define CACALRUN_RUNCACALEXIT		(1 << 12)
#define CACALRUN_RUNCACALVREF		(1 << 16)
#define rAMP_CACALRESULT(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x91C)
#define rAMP_CACALRUN(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x924)
#define rAMP_CACALCTRL(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x900)
#define rAMP_CACALCTRL2(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x904)
#define rAMP_CACALCTRL3(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x928)
#define rAMP_CACALPAT(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) +  0x90C)
#define rAMP_CASDLLCTRL(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x700)
#define rAMP_CASDLLCODE(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x704)
#define rAMP_CKSDLLCTRL(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x710)
#define rAMP_CKSDLLCODE(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x714)
#define rAMP_CSSDLLCTRL(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x708)
#define rAMP_CSSDLLCODE(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x70C)
#define rAMP_AMPCACALENTRYCMD(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x910)
#define rAMP_AMPCACALEXITCMD(_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x914)
#define rAMP_MDLLCODE(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t)*AMP_DQ_SPACING) + 0x194)
#define rAMP_CKDESKEW_CTRL(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x51C)
#define rAMP_CADESKEW_CTRL(_c,_d)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x500 + ((_d)*4))
#define rAMP_CSDESKEW_CTRL(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x518)
#define rAMP_RDWRDQCALLOOPCNT(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE20)
#define rAMP_RUNRDDQCAL(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE58)
#define rAMP_CALPATCFG(_c)					(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x110)
#define rAMP_DQSDLLCTRL_RD(_b,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x608)
#define rAMP_DQSDLLCODE_RD(_b,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x60C)
#define rAMP_RDDQDESKEW_CTRL(_b,_c,_d)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x428 + ((_d)*4))
#define rAMP_RDDQSDESKEW_CTRL(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0x448)
#define rAMP_RDDMDESKEW_CTRL(_t,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0x44C)
#define rAMP_DQCALCTRL(_t,_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0x900)
#define rAMP_DQCALRUN(_c)					(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE58)
#define rAMP_DQCALRESULT(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE5C)
#define rAMP_RDCLKDLYSEL(_t,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0x618)

#define rAMP_RDDQCALPAT(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE2C)
#define rAMP_RdWrDqCalTiming_f0(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE00)
#define rAMP_RdWrDqCalTiming_f1(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE08)
#define rAMP_RdWrDqCalTiming_f2(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE10)
#define rAMP_RdWrDqCalTiming_f3(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE18)
#define rAMP_RdWrDqCalTimingCtrl2(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE28)
#define rAMP_RDDQCALVREF_f0(_t, _c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0xB00)
#define rAMP_RDDQCALVREF_f1(_t, _c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0xB04)

#define rAMP_CAWRLVLSDLLCODE(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x800)
#define rAMP_CAWRLVLENTRYCMD(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xB00)
#define rAMP_CAWRLVLEXITCMD(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xB04)
#define rAMP_CAWRLVLTIM(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xB08)
#define rAMP_CARUNWRLVL(_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xB0C)
#define rAMP_CAWRLVLRESULT(_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xB10)
#define rAMP_AMPSDQIOCFG_DQFLTCTRL(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0xA4)

#define rAMP_DQWRLVLSDLLCODE(_t,_c)	(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_t + 1)*AMP_DQ_SPACING) + 0x704)

#define rAMCX_DRAMCFG_FREQCHNGMRWCNT(_c) (DCS_BASE_ADDR + ((_c)*DCS_SPACING) + 0x44C)  

#define rAMCX_DRAMCFG_FREQCHNGCTL0_FREQ0(_c) (DCS_BASE_ADDR + ((_c)*DCS_SPACING) + 0x450)
#define rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ0(_c) (DCS_BASE_ADDR + ((_c)*DCS_SPACING) + 0x454)

#define rAMCX_DRAMCFG_FREQCHNGCTL0_FREQ1(_c) (DCS_BASE_ADDR + ((_c)*DCS_SPACING) + 0x468)
#define rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ1(_c) (DCS_BASE_ADDR + ((_c)*DCS_SPACING) + 0x46C)

#define RUNDESKEWUPD				(1 << 8)

#define rAMP_WRDQCALPAT(_c)					(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE30)
#define rAMP_WRDQSDQ_SDLLCTRL(_b,_c)	   (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x600)
#define rAMP_WRDQSDQ_SDLLCODE(_b,_c)	   (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x604)
#define rAMP_WRDQ_CAL_RUN(_c)					(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE60)
#define rAMP_WRDQ_CAL_RESULT(_c)				(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0xE64)
#define rAMP_WRDQCALVREF(_c)					(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + 0x908)
#define rAMP_WRDQDESKEW_CTRL(_b,_c,_t)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x400 + ((_t)*4))
#define rAMP_WRDQSDESKEW_CTRL(_b,_c)		(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x420)
#define rAMP_WRDMDESKEW_CTRL(_b,_c)			(AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x424)

#define rAMP_CAMDLL_VTSCL_REFCNTL(_b,_c)  (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x1D4)
#define rAMP_RDMDLL_VTSCL_REFCNTL(_b,_c) (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x1D8)
#define rAMP_WRMDLL_VTSCL_REFCNTL(_b,_c) (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x1D8)

#define rAMP_RDCAPCFG_FREQ(_c, _b, _f)	  (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x100 + ((_f) * 4))
#define rAMP_DQSPDENALWAYSON(_c, _b)	  (AMP_BASE_ADDR + ((_c)*AMP_SPACING) + ((_b + 1)*AMP_DQ_SPACING) + 0x110)

#if DCS_REG_VERSION == 2
#define rAMPH_CFGH_DQS0_WKPUPD(_c)	(AMP_H_BASE_ADDR + ((_c)*AMP_SPACING)+ 0x0C8)
#define rAMPH_CFGH_DQS1_WKPUPD(_c)	(AMP_H_BASE_ADDR + ((_c)*AMP_SPACING)+ 0x138)

#define F0_IDLE_ACTIVE_MASK	0xFFFEFFF7
#define F0_IDLE_ACTIVE_OFFSET	16	
#define F1_IDLE_ACTIVE_MASK	0xFFFDFFF7
#define F1_IDLE_ACTIVE_OFFSET	17
#define F1_PDPWK_OFFSET			3
#define DESKEW_CODE_MASK      0x3F
#endif // #if DCS_REG_VERSION == 2

#define RUNWRLVLENTRY_OFFSET			0
#define RUNWRLVLDQ_OFFSET				1
#define RUNWRLVLEXIT_OFFSET			3
#define RUNWRLVLLOOPCNT_OFFSET		8
#define RUNWRLVLMASK_OFFSET			16
#define RUNWRLVL_SDLLUPDOVR			8
#define RUNWRLVL_SDLLUPDWRRES			9
#define WRLVL_SDLLCODE_MASK			0xFFFFFF00
#define WRLVLMAX_SDLLCODE_MASK		0xFF00FFFF
#define WRLVL_SDLLCODE_OFFSET			0
#define WRLVLMAX_SDLLCODE_OFFSET		16
#define DQS_IDLE_ACTIVE_MASK			0xFFFFFFEF
#define DQS_IDLE_ACTIVE_OFFSET		4

#define WRLVL_RESULT						0
#define WRLVL_CNT0_OFFSET				8
#define WRLVL_CNT1_OFFSET				16

#define SDLLOFFSET 			16
#define RUN_SDLLUPDOVR 		1
#define RUN_SDLLUPDWRRES 	2

#define WR_DQ_SDLL_OFFSET	16
#define WR_DQS_SDLL_OFFSET	24
#define WR_SDLLUPDOVR		1	
#define WR_SDLLUPDWRRES		2	
#define WR_ALL_BITS_PASS	0xFFFF
#define WR_ALL_BITS_FAIL	0x0
#define WR_CALIB_RUN			0
#define WR_DQS_SDLL_CODE_OFFSET 8

#define VT_SCL_REF_MASK		0x3FF
#define VT_SCL_REF_BITS		10
#define VT_SCL_OVR_OFFSET  16
#define VT_SCL_REF_UPD_OFFSET 28

#define DQS_PDEN_OFFSET 24
#define DQS_PDEN_MASK   0xFEFFFFFF
#define DQSPDEN_ALWAYSON_OFFSET 0
#define DQSPDEN_ALWAYSON_MASK 0x1

#define MAX_SDLL_OFFSET 

#define TESTMODE_FORCECKELOW		(1 << 8)

#define SIGN_BIT_POS			(6)

#define AMP_MAX_RANKS_PER_CHAN		(2)

#endif /* _DCS_CAL_H */
