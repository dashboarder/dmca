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
#ifndef __DCS_H
#define __DCS_H

#include <sys/types.h>

/* DRAM Control Subsystem (DCS) */

#define DCS_MAX_FREQUENCY_SLOTS		4
#define DCS_NUM_FREQCHNGCTLREGS		6

#define DCS_FREQ(n)	(1 << n)

#define EXEC(_instructions)		do { _instructions ; } while(0)
#define __abs(x) 			(((x) < 0) ? -(x) : (x))

typedef enum {
	DCS_BOOT_COLDBOOT,
	DCS_BOOT_RESUME,		// "Classic Resume" is (AOP_DDR | AOP_AWAKE)
	DCS_BOOT_AOP_DDR,
	DCS_BOOT_AOP_AWAKE,
} dcs_boottype_t;

typedef enum {
	MR_READ,
	MR_WRITE,
	MR_MPC
} dcs_mrcmd_op_t;

struct dcs_per_freq {
	uint32_t	clk_period;
	uint32_t	freqchngctl[DCS_NUM_FREQCHNGCTLREGS];
	uint32_t	freqchngtim;
	uint32_t	lat;
	uint32_t	phyrdwrtim;
	uint32_t	caspch;
	uint32_t	act;
	uint32_t	autoref;
	uint32_t	selfref;
	uint32_t	pdn;
	uint32_t	derate;
	uint32_t	lat2;
	uint32_t	tat;
	uint32_t	mifqmaxctrl;
	uint32_t	nondqdspd;
	uint32_t	nondqds;
	uint32_t	cavref;
	uint32_t	odt_enable;
	uint32_t	dqds;
	uint32_t	dqdqsds;
	uint32_t	dqvref;
	uint32_t	cadficaltiming;
	uint32_t	dqdficaltiming;
	uint32_t	rddqcalwindow;
	uint32_t	wrdqcalwindow;
	uint32_t	caoutdllscl;
	uint32_t	dqsindllscl;
	uint32_t	rdcapcfg;
	uint32_t	autoref2;
};

typedef struct dcs_config_params {
	uint32_t		flags;
	uint32_t		supported_freqs;
	uint32_t		num_freqchngctl_regs;
	uint32_t		spllctrl_vco_1;
	uint32_t		spllctrl_vco_2;
	uint32_t		wrdqcalvrefcodecontrol;
	uint32_t		rddqcalvrefcodecontrol;
	uint32_t		rwcfg;
	uint32_t		phyupdatetimers;
	uint32_t		dllupdtctrl;
	uint32_t		dllupdtctrl1;
	uint32_t		pdn1;
	uint32_t		modereg;
	uint32_t		modereg1;
	uint32_t		autoref_params;
	uint32_t		autoref_params2;
	uint32_t		pdn;
	uint32_t		rnkcfg;
	uint32_t		cackcswkds;
	uint32_t		dqspdres;
	uint32_t		casdllupdatectrl;
	uint32_t		dqsdllupdatectrl;
	uint32_t		rdsdllctrl_step2;
	uint32_t		wrdqdqssdllctrl_step2;
	uint32_t		cawrlvlsdllcode;
	uint32_t		dlllocktim;
	uint32_t		dficaltiming;
	uint32_t		rdwrdqcaltiming_f0;
	uint32_t		rdwrdqcalseglen_f0;
	uint32_t		rdwrdqcaltiming_f1;
	uint32_t		rdwrdqcalseglen_f1;
	uint32_t		rdwrdqcaltiming_f3;
	uint32_t		rdwrdqcaltimingctrl1;
	uint32_t		rdwrdqcaltimingctrl2;
	uint32_t		rddqcalpatprbs4i;
	uint32_t		wrdqcalpatprbs4i;
	uint32_t		maxrddqssdllmulfactor;
	uint32_t		maxwrdqssdllmulfactor;
	uint32_t		dllupdtintvl;
	uint32_t		dcccontrol;
	uint32_t		dcccal;
	uint32_t		cbdrivestr;
	uint32_t		cbioctl;
	uint32_t		ckdrivestr;
	uint32_t		ckioctl;
	uint32_t		b0drivestr;
	uint32_t		b0ioctl;
	uint32_t		b0odt;
	uint32_t		b0odtctrl;
	uint64_t		b0dyniselasrtime;
	uint64_t		b0dynisel;
	uint64_t		b1dynisel;
	uint32_t		b1drivestr;
	uint32_t		b1ioctl;
	uint32_t		b1odt;
	uint32_t		b1odtctrl;
	uint32_t		dqs0drivestr;
	uint32_t		dqs0ioctl;
	uint32_t		dqs0odt;
	uint32_t		dqs0odtctrl;
	uint32_t		dqs0zdetbiasen;
	uint32_t		dqs1drivestr;
	uint32_t		dqs1ioctl;
	uint32_t		dqs1odt;
	uint32_t		dqs1odtctrl;
	uint32_t		dqs1zdetbiasen;
	uint32_t		zcalfsm0;
	uint32_t		zcalfsm1;
	uint32_t		spare0;
	uint32_t		hwrddqcaltvref;
	uint32_t		hwwrdqcaltvref;
	uint32_t		arefparam;
	uint32_t		odtszqc;
	uint32_t		longsr;
	uint32_t		freqchngctl_step3;
	uint32_t		mr3cmd;
	uint32_t		mr13cmd;
	uint32_t		addrcfg;
	uint32_t		aiuprtbaddr;
	uint32_t		chnldec;
	uint32_t		arefparam2;
	uint32_t		mr13cmd_step7;
	uint32_t		mr3cmd_step7;
	uint32_t		mr22cmd;
	uint32_t		mr11cmd;
	uint32_t		mr13cmd_step9;
	uint32_t		freqchngctl_step9;
	uint32_t		dqs0wkpupd;
	uint32_t		dqs1wkpupd;
	uint32_t		mr13cmd_step11;
	uint32_t		mr2cmd_step11;
	uint32_t		mr1cmd_step11;
	uint32_t		mr3cmd_step11;
	uint32_t		mr22cmd_step11;
	uint32_t		mr11cmd_step11;
	uint32_t		mr12cmd_step11;
	uint32_t		mr14cmd_step11;
	uint32_t		mr13cmd_step13;
	uint32_t		mr13cmd_step15;
	uint32_t		freqchngctl_step15;
	uint32_t		rdsdllctrl_step15;
	uint32_t		odtszqc2;
	uint32_t		qbren_step16;
	uint32_t		qbrparam;
	uint32_t		qbren;
	uint32_t		mccgen;
	uint32_t		caampclk;
	uint32_t		dqampclk;
	uint32_t		pwrmngten;
	uint32_t		odtszqc3;
	struct dcs_per_freq 	freq[DCS_MAX_FREQUENCY_SLOTS];
} dcs_config_params_t;

typedef struct dcs_tunable {
	volatile uintptr_t	reg;
	uint32_t		mask;
	uint32_t		value;
} dcs_tunable_t;

struct dcs_memory_device_info {
	uint32_t	vendor_id;
	uint32_t	rev_id;
	uint32_t	rev_id2;
	uint32_t	type;
	uint32_t	width;		// Device width in Bytes
	uint32_t	density;	// Device density in MBytes
};

// to write the same value to the register in all channels
void dcs_reg_write_all_chan(uintptr_t reg_addr, uint32_t reg_value);
bool dcs_reg_is_outside_dcs_block(uintptr_t reg_addr);
void dcs_mrcmd(dcs_mrcmd_op_t op, uint8_t channels, uint8_t ranks, int32_t reg, uintptr_t val);
void dcs_enable_slow_boot (bool enable);
void dcs_enable_autorefresh(void);
uint64_t dcs_get_memory_size(void);
const struct dcs_memory_device_info *dcs_get_memory_device_info(void);
volatile uint32_t *dcs_save_calibration_results(volatile uint32_t *save_region, uint32_t freq);
void dcs_restore_calibration_results(volatile uint32_t *save_region);
void dcs_lock_down_mcc(void);

int32_t dcs_init(dcs_boottype_t boot_type);
#endif /* ! __DCS_H */
