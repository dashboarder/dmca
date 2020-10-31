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
#ifndef __RECONFIG_H
#define __RECONFIG_H

#include <sys/types.h>
#include <sys/boot.h>

typedef enum {
	AWAKE_AOP_DDR_PRE = 0,
	AWAKE_AOP_DDR_POST,
	AOP_DDR_S2R_AOP_PRE,
	AOP_DDR_S2R_AOP_POST,
	S2R_AOP_AOP_DDR_PRE,
	S2R_AOP_AOP_DDR_POST,
	AOP_DDR_AWAKE_PRE,
	AOP_DDR_AWAKE_POST,
	MAX_STAGE
} reconfig_stage_t;

enum {
	END_COMMAND = 0x0,
	WRITE_COMMAND = 0x1,
	READ_COMMAND = 0x2,
	DELAY_COMMAND = 0x4,
	NOP_COMMAND = 0x4,
};

typedef struct write_command_t {
	uint32_t in_progress;
	uint64_t address;
	uint32_t is_reg64;
	uint32_t offsets[4];
	uint32_t value_count;
	uint64_t values[16];

} write_command_t;

typedef enum {
	REGION_ONLY,
	COMMAND_ONLY,
	REGION_AND_COMMAND,
} reconfig_dump_t;

void reconfig_init(enum boot_target target);

void reconfig_command_write(reconfig_stage_t stage, uint64_t addr, uint64_t value, uint32_t is_reg64);

void reconfig_command_read(reconfig_stage_t stage, uint64_t addr, uint64_t value, uint64_t mask, uint32_t retry_cnt, uint32_t is_reg64);

void reconfig_command_delay(reconfig_stage_t stage, uint32_t delay);

void reconfig_command_nop(reconfig_stage_t stage);

void reconfig_command_raw(reconfig_stage_t stage, const uint32_t *cmd, uint32_t cmdItems);

void reconfig_commit(reconfig_stage_t stage);

void reconfig_lock(enum boot_target target);

void dump_reconfig(reconfig_dump_t option);

static void validate_stage(reconfig_stage_t stage);
static void commit_write_command(reconfig_stage_t stage, write_command_t *wc);
static void bounds_check_stage(reconfig_stage_t stage);
static void reconfig_write_pending(reconfig_stage_t stage);

// For 64 bit registers the data/mask etc.. need to be aligned to 64 bits
// count * 4 is the register offset from the 64 bit aligned base. So the following should 
// tell if we are aligned or not
#define ALIGNED_64(count)		((count) % 2 == 0)
#define ALIGNMENT_MARKER		0xDEADBEEF
#define SEQUENCE_END			0xDEADDEAD

#endif /* __RECONFIG_H */
