/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef AOP_CONFIG_SEQUENCES_A0_H
#define AOP_CONFIG_SEQUENCES_A0_H


#if DCS_NUM_CHANNELS == 8
#define AMC_PARAM_EIGHT_CH_ONE_RANK
#include "reconfig_sequences_mcu_8ch_s8001_a0.h"
#else
#include "reconfig_sequences_mcu_4ch_s8001_a0.h"
#endif

//     _         _                                        _           _ 
//    / \  _   _| |_ ___   __ _  ___ _ __   ___ _ __ __ _| |_ ___  __| |
//   / _ \| | | | __/ _ \ / _` |/ _ \ '_ \ / _ \ '__/ _` | __/ _ \/ _` |
//  / ___ \ |_| | || (_) | (_| |  __/ | | |  __/ | | (_| | ||  __/ (_| |
// /_/   \_\__,_|\__\___/ \__, |\___|_| |_|\___|_|  \__,_|\__\___|\__,_|
//  ____                  |___/     _____    _ _ _                      
// |  _ \  ___    _ __   ___ | |_  | ____|__| (_) |_                    
// | | | |/ _ \  | '_ \ / _ \| __| |  _| / _` | | __|                   
// | |_| | (_) | | | | | (_) | |_  | |__| (_| | | |_                    
// |____/ \___/  |_| |_|\___/ \__| |_____\__,_|_|\__|                   
//

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP SOC CONFIG SEQUENCES */
/* Comment: // ****************************************************************************************** */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
/* Comment: // ** RELEASE NOTES */
/* Comment: // **  */
/* Comment: // **    -- 2/8/2015 -- */
/* Comment: // **    Added "verbatim" command which will print the specified text verbatim in the final */
/* Comment: // **    output file.  This is used for putting #defines into the final header file for SW. */
/* Comment: // **  */
/* Comment: // **    -- 2/5/2015 -- */
/* Comment: // **    Introduced ifdef command for 4ch vs 8ch mode.  Moved 4ch and 8ch differences back */
/* Comment: // **    into aop_soc_cfg.seq. */
/* Comment: // **  */
/* Comment: // **    Note: Nested ifdefs are not supported! */
/* Comment: // **  */
/* Comment: // **    -- 2/4/2015 -- */
/* Comment: // **    Ported stitch sequences to Elba.  Moved 4ch and 8ch differences into stitched files. */
/* Comment: // **    Deleted 4ch version of aop_soc_cfg.seq. */
/* Comment: // **  */
/* Comment: // **    -- 1/29/2015 -- */
/* Comment: // **    Updated sequences based on CoreOS feedback from radars 19132562 and 19148279. */
/* Comment: // **    Added "stitch" command to stitch in sequences from external files.  Intended to */
/* Comment: // **    explicitly annotate which sequences are meant to be overridden by SW. */
/* Comment: // **  */
/* Comment: // **    -- 1/26/2015 -- */
/* Comment: // **    Disable DynPwrDn in AOP_DDR (see radar 19338346) */
/* Comment: // **  */
/* Comment: // **    -- 11/12/2014 -- */
/* Comment: // **    Added S2R_AOP->AOP_DDR preamble which polls for proxy_osc_clk to be equal to lpo/8. */
/* Comment: // **    Activating the S2R_AOP->AOP_DDR preamble forces the LPO to be on prior to the */
/* Comment: // **    AOP-PMU handshake.  See radar 18563566 for details. */
/* Comment: // **  */
/* Comment: // **    -- 7/22/2014 -- */
/* Comment: // **    Config engine changes the current perf state to 0 prior to polling. */
/* Comment: // **  */
/* Comment: // **    -- 7/17/2014 -- */
/* Comment: // **    Removed MCU bucket 1 transitions per radar 17699704. */
/* Comment: // **  */
/* Comment: // **    -- 6/26/2014 -- */
/* Comment: // **    Instead of implicitly stitching the 48->800 MCU sequence at the beginning of the */
/* Comment: // **    AOP_DDR->AWAKE sequence, it is now inserted at the comment token labeled  */
/* Comment: // **    "INSERT_MCU_AWAKE_SEQ".  This will make the sequence more flexible in case commands */
/* Comment: // **    are to be inserted before the MCU sequence, e.g., changing tunables. */
/* Comment: // **     */
/* Comment: // **    -- 6/20/2014 -- */
/* Comment: // **    Added more documentation to aop_soc_cfg.seq. */
/* Comment: // **     */
/* Comment: // **    -- 6/4/2014 -- */
/* Comment: // **    In this update, we removed the need to specify reg_addr in the write/write64/poll */
/* Comment: // **    commands.  The reg_addr was prone to human error because it was manually written, */
/* Comment: // **    so there is no guarantee that the address matches the register name. Furthermore, */
/* Comment: // **    manually specifying the register address is not future-proof because addresses of */
/* Comment: // **    registers may change from chip to chip. */
/* Comment: // **     */
/* Comment: // **    Now, the register address are calculated based on the register name and the include */
/* Comment: // **    files.  The user must ensure that all of the header files needed are specified */
/* Comment: // **    using the "include:" command.  In the DV environment, these header files were */
/* Comment: // **    generated from SPDS. */
/* Comment: // ** */
/* Comment: // **    Note: For legacy reasons, we still support manually specifying the reg_addr field. */
/* Comment: // **    If the reg_addr field is specified, the reg_name field is ignored during the */
/* Comment: // **    generation of the .h file. */
/* Comment: // ** */
/* Comment: // **    We also renamed the file type of this file from .h to .seq. */
/* Comment: // ** */
/* Comment: // ****************************************************************************************** */
/* Comment: // **  */
/* Comment: // ** This file contains the AOP SoC config sequences verified by DV.  It is written in a */
/* Comment: // ** pseudo-language that is parsed by the scripts described in the "Related files" section */
/* Comment: // ** below.  All commands must be contained within a single line. */
/* Comment: // **  */
/* Comment: // ** The supported commands are: */
/* Comment: // **  */
/* Comment: // **    include: <file_name> */
/* Comment: // **       Header file to include for looking up register addresses.  The path to the include */
/* Comment: // **       files is provided as an argument to the aop_func_gen.pl script described below. */
/* Comment: // ** */
/* Comment: // **    seq_name: <sequence_name> */
/* Comment: // **       Designates the start of a sequence.  The supported sequences are: */
/* Comment: // **          s2r_aop_to_aop_ddr_a0_pre */
/* Comment: // **          s2r_aop_to_aop_ddr_a0_post */
/* Comment: // **          aop_ddr_to_s2r_aop_a0_pre */
/* Comment: // **          aop_ddr_to_s2r_aop_a0_post */
/* Comment: // **          aop_ddr_to_awake_a0_pre */
/* Comment: // **          aop_ddr_to_awake_a0_post */
/* Comment: // **          awake_to_aop_ddr_a0_pre */
/* Comment: // **          awake_to_aop_ddr_a0_post */
/* Comment: // **       Note that s2r_aop_to_aop_ddr_a0_pre and aop_ddr_to_s2r_aop_a0_post are currently unused. */
/* Comment: // ** */
/* Comment: // **    endseq */
/* Comment: // **       Designates the end of a sequence. */
/* Comment: // ** */
/* Comment: // **    write: reg_name=<register_name> [reg_addr=<address>] reg_value=<value> */
/* Comment: // **       32-bit write command to the register at 36-bit hex <address> with 32-bit hex <value>. */
/* Comment: // **       If reg_addr is specified, reg_name is ignored (except for generation of .c file). */
/* Comment: // ** */
/* Comment: // **    write64: reg_name=<register_name> [reg_addr=<address>] reg_value=<upper_lower> */
/* Comment: // **       64-bit write command to the 36-bit hex <address> with 64-bit value <upper_lower>, */
/* Comment: // **       where "upper" is the upper 32-bit hex value, and "lower" is the lower 32-bit hex */
/* Comment: // **       value. */
/* Comment: // **       If reg_addr is specified, reg_name is ignored (except for generation of .c file). */
/* Comment: // ** */
/* Comment: // **    write_start */
/* Comment: // **       Designates the start of a write command with multiple register writes. Refer to */
/* Comment: // **       AOP spec section 6.1 "Configuration Write Command" for details.  Because the */
/* Comment: // **       annotation with "write_start" and "write_end" is manually done, it is the */
/* Comment: // **       responsibility of the author to ensure that the combined writes are within the */
/* Comment: // **       same 1KB block, and a max of 16 writes is included. */
/* Comment: // **  */
/* Comment: // **    write_end */
/* Comment: // **       Designates the end of a multi-register write command. */
/* Comment: // ** */
/* Comment: // **    poll: reg_name=<register_name> [reg_addr=<address>] reg_value=<value> reg_mask=<mask> retry_en=<retry_en> retry_cnt=<retry_cnt> */
/* Comment: // **       Reads 36-bit <address> until (read_data & mask == value). If <retry_en> is 0, the */
/* Comment: // **       poll repeats indefinitely.  If <retry_en> is set, the read is retried up to */
/* Comment: // **       <retry_cnt> times. As with "write" and "write64", <register_name> is not used. */
/* Comment: // **       If reg_addr is specified, reg_name is ignored (except for generation of .c file). */
/* Comment: // ** */
/* Comment: // **    cfg_cnt: <delay> */
/* Comment: // **       <delay> designates the number of aop_clk cycles to wait before executing the next  */
/* Comment: // **       command. */
/* Comment: // ** */
/* Comment: // **    cfg_end */
/* Comment: // **       Designates the Configuration End Command. */
/* Comment: // ** */
/* Comment: // **    comment: <text> */
/* Comment: // **       Designates a comment. */
/* Comment: // ** */
/* Comment: // **    stitch: <file.seq> */
/* Comment: // **       Stitches file.seq into the sequence inline. */
/* Comment: // ** */
/* Comment: // **    stitch_h: <file.h> */
/* Comment: // **       In the generated .h file, an explicit #include "file.h" will appear at this line. */
/* Comment: // ** */
/* Comment: // ** Related files: */
/* Comment: // **   tb_lib/cpu/armv7/aop_lib/aop_func_gen.pl */
/* Comment: // **      This script parses this file (aop_soc_cfg.h) and converts it into an intermediate */
/* Comment: // **      C header format.  The output file contains C arrays whose contents are in a format */
/* Comment: // **      understood by the SOC config engine.  In the DV environment, the intermediate C */
/* Comment: // **      header file is called aop_soc_engine.h and is placed in the testgen* directory of */
/* Comment: // **      the test results directory.  The intermediate file and MCU config sequences are */
/* Comment: // **      combined using the powgen_common.pl script, described below. */
/* Comment: // **       */
/* Comment: // **   tb_lib/cpu/armv8/mcu/mcu_init_aop.h */
/* Comment: // **      Provided by the MCU team, mcu_init_aop.h contains the two sequences for configuring */
/* Comment: // **      the DDR to operate at 48MHz and >= 800MHz.  The 48MHz sequence is stitched into the */
/* Comment: // **      S2R_AOP->AOP_DDR postamble at the comment token "INSERT_MCU_AOP_DDR_SEQ".  The */
/* Comment: // **      >= 800 MHz sequence is stitched into the AOP_DDR->AWAKE postamble at the comment */
/* Comment: // **      token "INSERT_MCU_AWAKE_SEQ". */
/* Comment: // **       */
/* Comment: // **   chip/tb/cpu/bin/powgen_common.pl */
/* Comment: // **      This script stitches together the aop_soc_engine.h file generated by aop_func_gen.pl */
/* Comment: // **      and mcu_init_aop.h into the same AP EHEX file that is loaded into memory. */
/* Comment: // **       */
/* Comment: // ****************************************************************************************** */
/* Comment: // ****************************************************************************************** */
/* Comment: // ** S2R_AOP->AOP_DDR PREAMBLE */
/* Comment: // **  */
/* Comment: // ** Poll for the proxy_osc clock to source LPO/8 before initiating minipmgr reset sequence */
/* Comment: // ****************************************************************************************** */
static const uint32_t s2r_aop_to_aop_ddr_a0_pre_0[] = {
0x21024002, /* Poll:MINIPMGR_MINI_CLKCFG_PROXY_OSC_CLK_CFG ADDR=0x210240304 Retry_en=0 Retry_cnt=0*/
0x000000c1, /* Offset */
0x40300000, /* Mask */
0x00200000, /* Data */
/* Comment: // Do a dummy read to the AOP CPU revision register to cause a functional wakeup */
0x21080002, /* Poll:AKF_AKFAPB_AKF_REV(GBI_AOP_AKF) ADDR=0x210800000 Retry_en=1 Retry_cnt=597*/
0x00035500, /* Offset */
0x00000000, /* Mask */
0x00000000, /* Data */
/* Comment: // for radar 20547317: AOP-CPU powering up around the time AOP-DDR partitions are undergoing BIRA repair */
0x21029c01, /* WR:MINIPMGR_MINI_PWRGATE_PWR_AOP_CPU_DBG ADDR=0x21029c00c */
0x00000003, /* Offset */
0x00000004, /* Data */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

static const uint32_t s2r_aop_to_aop_ddr_a0_pre_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** S2R_AOP->AOP_DDR POSTAMBLE */
/* Comment: // ****************************************************************************************** */
static const uint32_t s2r_aop_to_aop_ddr_a0_post_0[] = {
/* Comment: // for radar 20547317: AOP-CPU powering up around the time AOP-DDR partitions are undergoing BIRA repair */
0x21029c01, /* WR:MINIPMGR_MINI_PWRGATE_PWR_AOP_CPU_DBG ADDR=0x21029c00c */
0x00000003, /* Offset */
0x00000000, /* Data */
/* Comment: // Make the proxy fabric clock run at 192MHz to speed up AOP_DDR transition */
0x21024002, /* Poll:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 Retry_en=1 Retry_cnt=255*/
0x0001ff06, /* Offset */
0x04000000, /* Mask */
0x00000000, /* Data */
0x21024001, /* WR:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 */
0x00000006, /* Offset */
0x81100000, /* Data */
/* Comment: // Security sequence  */
/* Comment: // Write mem cache data and tag enable/lock bits to prevent backdoor access as RAM attacks */
0x20000041, /* WR:AMCC_MCCCFG_MCCGEN(0) ADDR=0x200000780 */
0x000000e0, /* Offset */
0x00010120, /* Data */
0x20020041, /* WR:AMCC_MCCCFG_MCCGEN(1) ADDR=0x200200780 */
0x000000e0, /* Offset */
0x00010120, /* Data */
/* Comment: // Write lockable DRAM config regs */
/* Comment: // INSERT_MCU_DDR_LOCKED */
};

/* Comment: // The MCU LockRegion programming must occur before the security/TZ programming (s2r_aop_to_aop_ddr_a0_post_security) */
#define S2D_POST_MCU_LOCKED_AFTER  0
#define S2D_POST_MCU_LOCKED_BEFORE 1
/* Comment: // The security/TZ programming must occur after the MCU LockRegion programming */
#define S2D_POST_SECURITY_AFTER  0
#define S2D_POST_SECURITY_BEFORE 1
/* Comment: // ****************************************************************************************** */
/* Comment: // ** SOC DV Version of S2R_AOP->AOP_DDR security postamble */
/* Comment: // ** */
/* Comment: // ** DO NOT USE THIS FILE VERBATIM -- THIS IS FOR DV TESTING PURPOSES ONLY */
/* Comment: // ****************************************************************************************** */
static const uint32_t s2r_aop_to_aop_ddr_a0_post_security[] = {
/* Comment: // Enable AMCC Trust Zone 0 for SEP. Lock range = 0x8_3D00_0000 => 0x8_3d01_0fff */
0x20000049, /* 3-word write */
0x00242120, /* Offset */
0x0003d000, /* Data WR:AMCC_MCCLOCKREGION_TZ0BASEADDR(0) ADDR=0x200000480 */
0x0003d010, /* Data WR:AMCC_MCCLOCKREGION_TZ0ENDADDR(0) ADDR=0x200000484 */
0x00000001, /* Data WR:AMCC_MCCLOCKREGION_TZ0LOCK(0) ADDR=0x200000490 */
0x20000042, /* Poll:AMCC_MCCLOCKREGION_TZ0LOCK(0) ADDR=0x200000490 Retry_en=1 Retry_cnt=255*/
0x0001ff24, /* Offset */
0x00000001, /* Mask */
0x00000001, /* Data */
/* Comment: // enable AMCC Trust Zone 1 for AP. Lock range = 0x8_3C00_0000 => 0x8_3C01_0fff */
0x20000049, /* 3-word write */
0x00252322, /* Offset */
0x0003c000, /* Data WR:AMCC_MCCLOCKREGION_TZ1BASEADDR(0) ADDR=0x200000488 */
0x0003c010, /* Data WR:AMCC_MCCLOCKREGION_TZ1ENDADDR(0) ADDR=0x20000048c */
0x00000001, /* Data WR:AMCC_MCCLOCKREGION_TZ1LOCK(0) ADDR=0x200000494 */
0x20000042, /* Poll:AMCC_MCCLOCKREGION_TZ1LOCK(0) ADDR=0x200000494 Retry_en=1 Retry_cnt=255*/
0x0001ff25, /* Offset */
0x00000001, /* Mask */
0x00000001, /* Data */
/* Comment: // enable AMCC Trust Zone 0 for SEP. Lock range = 0x8_3D00_0000 => 0x8_3d01_0fff */
0x20020049, /* 3-word write */
0x00242120, /* Offset */
0x0003d000, /* Data WR:AMCC_MCCLOCKREGION_TZ0BASEADDR(1) ADDR=0x200200480 */
0x0003d010, /* Data WR:AMCC_MCCLOCKREGION_TZ0ENDADDR(1) ADDR=0x200200484 */
0x00000001, /* Data WR:AMCC_MCCLOCKREGION_TZ0LOCK(1) ADDR=0x200200490 */
0x20020042, /* Poll:AMCC_MCCLOCKREGION_TZ0LOCK(1) ADDR=0x200200490 Retry_en=1 Retry_cnt=255*/
0x0001ff24, /* Offset */
0x00000001, /* Mask */
0x00000001, /* Data */
/* Comment: // enable AMCC Trust Zone 1 for AP. Lock range = 0x8_3C00_0000 => 0x8_3C01_0fff */
0x20020049, /* 3-word write */
0x00252322, /* Offset */
0x0003c000, /* Data WR:AMCC_MCCLOCKREGION_TZ1BASEADDR(1) ADDR=0x200200488 */
0x0003c010, /* Data WR:AMCC_MCCLOCKREGION_TZ1ENDADDR(1) ADDR=0x20020048c */
0x00000001, /* Data WR:AMCC_MCCLOCKREGION_TZ1LOCK(1) ADDR=0x200200494 */
0x20020042, /* Poll:AMCC_MCCLOCKREGION_TZ1LOCK(1) ADDR=0x200200494 Retry_en=1 Retry_cnt=255*/
0x0001ff25, /* Offset */
0x00000001, /* Mask */
0x00000001 /* Data */
};

static const uint32_t s2r_aop_to_aop_ddr_a0_post_1[] = {
/* Comment: // Disable AP_GID0 */
0x2102d001, /* WR:MINIPMGR_SECURITY_SIO_AES_DISABLE ADDR=0x2102d0000 */
0x00000000, /* Offset */
0x00000002, /* Data */
/* Comment: // SRAM region that must be locked ends here. */
/* Comment: // ****************************************************************************************** */
/* Comment: // ** Note for SW: */
/* Comment: // ** */
/* Comment: // ** Locked region is specified in 64B granularity.  If the last security entry does not */
/* Comment: // ** land neatly at the end of a 64B boundary, some non-security entries may be locked if */
/* Comment: // ** no additional padding is used.  */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
/* Comment: // Disable voltage changes (see radar 16038537) */
0x20e0a001, /* WR:PMGR_VOLMAN_VOLMAN_CTL ADDR=0x20e0a0000 */
0x00000000, /* Offset */
0x00001f00, /* Data */
/* Comment: // ****************************************************************************************** */
/* Comment: // ** Note for SiVal/SW: */
/* Comment: // ** */
/* Comment: // ** Per Manu, perf state entry 0 has been reserved for use by the config engine. SW agreed */
/* Comment: // ** to use the perf state entries starting from the higher indices. */
/* Comment: // ****************************************************************************************** */
/* Comment: // transition the fabric and DRAM to work off the proxy clocks */
0x20e06809, /* 3-word write */
0x00000504, /* Offset */
0x0000bb39, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0A ADDR=0x20e068010 */
0x90000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0B ADDR=0x20e068014 */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 */
0x20e06802, /* Poll:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x80000000, /* Mask */
0x00000000, /* Data */
/* Comment: // Clear the CSYSPWRUPREQ_LOOPBACK bit.  See PMGR spec section 11.4.4.1 */
0x2102cc01, /* WR:MINIPMGR_MINI_PWR_STATE_TRANS_STATE_TRANS_CTL ADDR=0x2102cc000 */
0x00000000, /* Offset */
0x00000000 /* Data */
};

/* Comment: // Enable automatic power management for the fabric and memory controller */
#define S2D_POST_PWRGATE_AFTER  1
#define S2D_POST_PWRGATE_BEFORE 2
/* Comment: // ****************************************************************************************** */
/* Comment: // ** SOC DV Version of S2R_AOP->AOP_DDR pwrgate sequence */
/* Comment: // ** */
/* Comment: // ** The primary purpose of this sequence is to set PWR_DOM_EN for the blocks which need */
/* Comment: // ** to be powered down in AOP_DDR to save power.  Since the same register is used for  */
/* Comment: // ** some tunable fields, the tunable values for these registers are also applied here. */
/* Comment: // ** */
/* Comment: // ** These values are hand-coded by the SOC DV team based on the tunable values on */
/* Comment: // ** 2/2/2015, and should not be used directly by the CoreOS team in case tunable values */
/* Comment: // ** change. */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
static const uint32_t s2r_aop_to_aop_ddr_a0_post_pwrgate[] = {
0x20e09c2d, /* 12-word write */
0x382a2928, /* Offset */
0x2d2c3a39, /* Offset */
0x3231302e, /* Offset */
0x80030309, /* Data WR:PMGR_PWRGATE_PWR_MCC_CFG0 ADDR=0x20e09c0a0 */
0x000f0003, /* Data WR:PMGR_PWRGATE_PWR_MCC_CFG1 ADDR=0x20e09c0a4 */
0x01060600, /* Data WR:PMGR_PWRGATE_PWR_MCC_CFG2 ADDR=0x20e09c0a8 */
0x80030303, /* Data WR:PMGR_PWRGATE_PWR_ACS_CFG0 ADDR=0x20e09c0e0 */
0x001d0000, /* Data WR:PMGR_PWRGATE_PWR_ACS_CFG1 ADDR=0x20e09c0e4 */
0x01030301, /* Data WR:PMGR_PWRGATE_PWR_ACS_CFG2 ADDR=0x20e09c0e8 */
0x80030301, /* Data WR:PMGR_PWRGATE_PWR_DCS0123_CFG0 ADDR=0x20e09c0b0 */
0x00190003, /* Data WR:PMGR_PWRGATE_PWR_DCS0123_CFG1 ADDR=0x20e09c0b4 */
0x010a0808, /* Data WR:PMGR_PWRGATE_PWR_DCS0123_CFG2 ADDR=0x20e09c0b8 */
0x80030301, /* Data WR:PMGR_PWRGATE_PWR_DCS4567_CFG0 ADDR=0x20e09c0c0 */
0x00150004, /* Data WR:PMGR_PWRGATE_PWR_DCS4567_CFG1 ADDR=0x20e09c0c4 */
0x010a0808 /* Data WR:PMGR_PWRGATE_PWR_DCS4567_CFG2 ADDR=0x20e09c0c8 */
};

static const uint32_t s2r_aop_to_aop_ddr_a0_post_2[] = {
/* Comment: // Per <rdar://problem/20676275>, DV will set PWR_DOM_EN for SIO/USB/PCIE/PMP with default values for other fields */
0x20e09c0d, /* 4-word write */
0x50603424, /* Offset */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_SIO_CFG0 ADDR=0x20e09c090 */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_USB_CFG0 ADDR=0x20e09c0d0 */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_PCIE_CFG0 ADDR=0x20e09c180 */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_PMP_CFG0 ADDR=0x20e09c140 */

#ifdef AMC_PARAM_FOUR_CH_ONE_RANK
0x20e08031, /* 13-word write */
0xb0b24048, /* Offset */
0x92908e8c, /* Offset */
0x9a989694, /* Offset */
0x0000009c, /* Offset */
0x1000000f, /* Data WR:PMGR_PS_PMS_PS ADDR=0x20e080120 */
0x1000000f, /* Data WR:PMGR_PS_SBR_PS ADDR=0x20e080100 */
0x1000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x1000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x1000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x1000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x1000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x1000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x1000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x00000000, /* Data WR:PMGR_PS_DCS4_PS ADDR=0x20e080258 */
0x00000000, /* Data WR:PMGR_PS_DCS5_PS ADDR=0x20e080260 */
0x00000000, /* Data WR:PMGR_PS_DCS6_PS ADDR=0x20e080268 */
0x00000000, /* Data WR:PMGR_PS_DCS7_PS ADDR=0x20e080270 */
#endif


#ifdef AMC_PARAM_EIGHT_CH_ONE_RANK
0x20e08031, /* 13-word write */
0xb0b24048, /* Offset */
0x92908e8c, /* Offset */
0x9a989694, /* Offset */
0x0000009c, /* Offset */
0x1000000f, /* Data WR:PMGR_PS_PMS_PS ADDR=0x20e080120 */
0x1000000f, /* Data WR:PMGR_PS_SBR_PS ADDR=0x20e080100 */
0x1000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x1000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x1000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x1000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x1000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x1000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x1000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x1000000f, /* Data WR:PMGR_PS_DCS4_PS ADDR=0x20e080258 */
0x1000000f, /* Data WR:PMGR_PS_DCS5_PS ADDR=0x20e080260 */
0x1000000f, /* Data WR:PMGR_PS_DCS6_PS ADDR=0x20e080268 */
0x1000000f, /* Data WR:PMGR_PS_DCS7_PS ADDR=0x20e080270 */
#endif

0x20e40001, /* WR:PMSCSR_PMSCSR_PMP_PMS_CPG_CTRL ADDR=0x20e40000c */
0x00000003, /* Offset */
0x00010100, /* Data */
0x20400001, /* WR:SOCBUSMUX_SOCBUSMUX_REGS_CPG_CNTL ADDR=0x204000090 */
0x00000024, /* Offset */
0x80012c04, /* Data */
0x20080001, /* WR:SWITCHFABRIC_SWITCHFABRIC_REGS_CPG_CNTL ADDR=0x200800094 */
0x00000025, /* Offset */
0x80017c04, /* Data */
0x20f00001, /* WR:AFC_AIU_SB_AFC_AIU_SB_SELF_REGS_CPG_CNTL ADDR=0x20f00000c */
0x00000003, /* Offset */
0x80011004, /* Data */
0x20f1c001, /* WR:DYNAMIC_CLK_GATING_SB_GLUE_DYNAMIC_CLK_GATING ADDR=0x20f1c0000 */
0x00000000, /* Offset */
0x0100001f, /* Data */
0x20000001, /* WR:AMCC_AMCCGEN_AMCCLKPWRGATE(0) ADDR=0x200000008 */
0x00000002, /* Offset */
0x10c80064, /* Data */
0x20020001, /* WR:AMCC_AMCCGEN_AMCCLKPWRGATE(1) ADDR=0x200200008 */
0x00000002, /* Offset */
0x10c80064, /* Data */
/* Comment: // enable ACG for the PMGR clock tree */
0x20e0dc01, /* WR:PMGR_MISC_CFG_ACG ADDR=0x20e0dc000 */
0x00000000, /* Offset */
0x00000001, /* Data */
/* Comment: // Powering down SIO, PCIE, USB, AIC, PMP */
0x20e0803d, /* 16-word write */
0x5e5c5a58, /* Offset */
0x66646260, /* Offset */
0x6e6c6a68, /* Offset */
0x76747270, /* Offset */
0x00000000, /* Data WR:PMGR_PS_MCA0_PS ADDR=0x20e080160 */
0x00000000, /* Data WR:PMGR_PS_MCA1_PS ADDR=0x20e080168 */
0x00000000, /* Data WR:PMGR_PS_MCA2_PS ADDR=0x20e080170 */
0x00000000, /* Data WR:PMGR_PS_MCA3_PS ADDR=0x20e080178 */
0x00000000, /* Data WR:PMGR_PS_MCA4_PS ADDR=0x20e080180 */
0x00000000, /* Data WR:PMGR_PS_PWM0_PS ADDR=0x20e080188 */
0x00000000, /* Data WR:PMGR_PS_I2C0_PS ADDR=0x20e080190 */
0x00000000, /* Data WR:PMGR_PS_I2C1_PS ADDR=0x20e080198 */
0x00000000, /* Data WR:PMGR_PS_I2C2_PS ADDR=0x20e0801a0 */
0x00000000, /* Data WR:PMGR_PS_I2C3_PS ADDR=0x20e0801a8 */
0x00000000, /* Data WR:PMGR_PS_SPI0_PS ADDR=0x20e0801b0 */
0x00000000, /* Data WR:PMGR_PS_SPI1_PS ADDR=0x20e0801b8 */
0x00000000, /* Data WR:PMGR_PS_SPI2_PS ADDR=0x20e0801c0 */
0x00000000, /* Data WR:PMGR_PS_SPI3_PS ADDR=0x20e0801c8 */
0x00000000, /* Data WR:PMGR_PS_UART0_PS ADDR=0x20e0801d0 */
0x00000000, /* Data WR:PMGR_PS_UART1_PS ADDR=0x20e0801d8 */
0x20e0803d, /* 16-word write */
0x7e7c7a78, /* Offset */
0x86848280, /* Offset */
0xd4525654, /* Offset */
0xdcdad8d6, /* Offset */
0x00000000, /* Data WR:PMGR_PS_UART2_PS ADDR=0x20e0801e0 */
0x00000000, /* Data WR:PMGR_PS_UART3_PS ADDR=0x20e0801e8 */
0x00000000, /* Data WR:PMGR_PS_UART4_PS ADDR=0x20e0801f0 */
0x00000000, /* Data WR:PMGR_PS_UART5_PS ADDR=0x20e0801f8 */
0x00000000, /* Data WR:PMGR_PS_UART6_PS ADDR=0x20e080200 */
0x00000000, /* Data WR:PMGR_PS_UART7_PS ADDR=0x20e080208 */
0x00000000, /* Data WR:PMGR_PS_UART8_PS ADDR=0x20e080210 */
0x00000000, /* Data WR:PMGR_PS_AES0_PS ADDR=0x20e080218 */
0x00000000, /* Data WR:PMGR_PS_SIO_P_PS ADDR=0x20e080150 */
0x00000000, /* Data WR:PMGR_PS_SIO_PS ADDR=0x20e080158 */
0x00000000, /* Data WR:PMGR_PS_SIO_BUSIF_PS ADDR=0x20e080148 */
0x00000000, /* Data WR:PMGR_PS_PCIE_AUX_PS ADDR=0x20e080350 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK0_PS ADDR=0x20e080358 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK1_PS ADDR=0x20e080360 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK2_PS ADDR=0x20e080368 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK3_PS ADDR=0x20e080370 */
0x20e08035, /* 14-word write */
0xaca8a4d2, /* Offset */
0xa0aaa6a2, /* Offset */
0xc8429eae, /* Offset */
0x00005044, /* Offset */
0x00000000, /* Data WR:PMGR_PS_PCIE_PS ADDR=0x20e080348 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST0_OHCI_PS ADDR=0x20e080290 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST1_OHCI_PS ADDR=0x20e0802a0 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST2_OHCI_PS ADDR=0x20e0802b0 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST0_PS ADDR=0x20e080288 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST1_PS ADDR=0x20e080298 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST2_PS ADDR=0x20e0802a8 */
0x00000000, /* Data WR:PMGR_PS_USBCTLREG_PS ADDR=0x20e080280 */
0x00000000, /* Data WR:PMGR_PS_USB_OTG_PS ADDR=0x20e0802b8 */
0x00000000, /* Data WR:PMGR_PS_USB_PS ADDR=0x20e080278 */
0x00000000, /* Data WR:PMGR_PS_AIC_PS ADDR=0x20e080108 */
0x00000000, /* Data WR:PMGR_PS_PMP_PS ADDR=0x20e080320 */
0x00000000, /* Data WR:PMGR_PS_DWI_PS ADDR=0x20e080110 */
0x00000000, /* Data WR:PMGR_PS_PCIE_REF_PS ADDR=0x20e080140 */
/* Comment: // Ensure CoherencePoint DupTag ways will power off */
0x20e09d01, /* WR:PMGR_PWRGATE_PWR_DUP_TAGS ADDR=0x20e09d000 */
0x00000000, /* Offset */
0x00000001 /* Data */
};

#define S2D_POST_MCU_DDR_AFTER  2
#define S2D_POST_MCU_DDR_BEFORE 3
static const uint32_t s2r_aop_to_aop_ddr_a0_post_3[] = {
/* Comment: // INSERT_MCU_AOP_DDR_SEQ */
/* Comment: // Set the proxy fabric clock frequency back to 96MHz to save power in AOP_DDR */
0x21024002, /* Poll:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 Retry_en=1 Retry_cnt=255*/
0x0001ff06, /* Offset */
0x04000000, /* Mask */
0x00000000, /* Data */
0x21024001, /* WR:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 */
0x00000006, /* Offset */
0x82100000, /* Data */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

static const uint32_t s2r_aop_to_aop_ddr_a0_post_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP_DDR->S2R_AOP PREAMBLE */
/* Comment: // ****************************************************************************************** */
static const uint32_t aop_ddr_to_s2r_aop_a0_pre_0[] = {
/* Comment: // Note - We do not wait for the memcache ways to reach 0 before yanking power.  See radar 16051416 */
/* Comment: // Set NO_HANDSHAKE bit prior to restoring the perf state table to its defaults, radar 16219268 */
0x20e04081, /* WR:PMGR_CLKCTL_MCU_CLK_DEBUG ADDR=0x20e040810 */
0x00000004, /* Offset */
0x00000001, /* Data */
/* Comment: // Set all SRC_SEL for SOC clocks to 0 so there will be no clock glitches, radar 16409004 */
0x20e06809, /* 3-word write */
0x00000504, /* Offset */
0x00000030, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0A ADDR=0x20e068010 */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0B ADDR=0x20e068014 */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 */
0x20e06802, /* Poll:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x80000000, /* Mask */
0x00000000, /* Data */
/* Comment: // power up PCIe/SIO/USB domain to bypass <rdar://problem/16219268> NLP:: Pcie2phy pwr clamp enable is going to X after warmboot in UPF simulations */
0x20e08001, /* WR:PMGR_PS_SIO_BUSIF_PS ADDR=0x20e080148 */
0x00000052, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_SIO_BUSIF_PS ADDR=0x20e080148 Retry_en=1 Retry_cnt=255*/
0x0001ff52, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08035, /* 14-word write */
0x9e56b0b2, /* Offset */
0x908e8cd2, /* Offset */
0x98969492, /* Offset */
0x00009c9a, /* Offset */
0x0000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x0000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x0000000f, /* Data WR:PMGR_PS_SIO_PS ADDR=0x20e080158 */
0x0000000f, /* Data WR:PMGR_PS_USB_PS ADDR=0x20e080278 */
0x0000000f, /* Data WR:PMGR_PS_PCIE_PS ADDR=0x20e080348 */
0x0000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x0000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x0000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x0000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x0000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x0000000f, /* Data WR:PMGR_PS_DCS4_PS ADDR=0x20e080258 */
0x0000000f, /* Data WR:PMGR_PS_DCS5_PS ADDR=0x20e080260 */
0x0000000f, /* Data WR:PMGR_PS_DCS6_PS ADDR=0x20e080268 */
0x0000000f, /* Data WR:PMGR_PS_DCS7_PS ADDR=0x20e080270 */
0x20e08002, /* Poll:PMGR_PS_SF_PS ADDR=0x20e0802c8 Retry_en=0 Retry_cnt=0*/
0x000000b2, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SMX_PS ADDR=0x20e0802c0 Retry_en=0 Retry_cnt=0*/
0x000000b0, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SIO_PS ADDR=0x20e080158 Retry_en=0 Retry_cnt=0*/
0x00000056, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB_PS ADDR=0x20e080278 Retry_en=0 Retry_cnt=0*/
0x0000009e, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_PS ADDR=0x20e080348 Retry_en=0 Retry_cnt=0*/
0x000000d2, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCC_PS ADDR=0x20e080230 Retry_en=0 Retry_cnt=0*/
0x0000008c, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS0_PS ADDR=0x20e080238 Retry_en=0 Retry_cnt=0*/
0x0000008e, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS1_PS ADDR=0x20e080240 Retry_en=0 Retry_cnt=0*/
0x00000090, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS2_PS ADDR=0x20e080248 Retry_en=0 Retry_cnt=0*/
0x00000092, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS3_PS ADDR=0x20e080250 Retry_en=0 Retry_cnt=0*/
0x00000094, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS4_PS ADDR=0x20e080258 Retry_en=0 Retry_cnt=0*/
0x00000096, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS5_PS ADDR=0x20e080260 Retry_en=0 Retry_cnt=0*/
0x00000098, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS6_PS ADDR=0x20e080268 Retry_en=0 Retry_cnt=0*/
0x0000009a, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS7_PS ADDR=0x20e080270 Retry_en=0 Retry_cnt=0*/
0x0000009c, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x2102cc01, /* WR:MINIPMGR_MINI_PWR_STATE_TRANS_STATE_TRANS_CTL ADDR=0x2102cc000 */
0x00000000, /* Offset */
0x00000001, /* Data */
/* Comment: // wait 100us to delay retention reset per radar 18765287 */
0x0012c004 /* Cfg Count(Delay.Wait) */
};

static const uint32_t aop_ddr_to_s2r_aop_a0_pre_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AWAKE->AOP_DDR PREAMBLE */
/* Comment: // **  */
/* Comment: // ** See PMGR spec section 11.14.4 (Software Teardown for Awake Exit) */
/* Comment: // ****************************************************************************************** */
static const uint32_t awake_to_aop_ddr_a0_pre_0[] = {
/* Comment: // Restore the GPIO to its default configuration by setting GPIO_PS.RESET for 1us while setting MANUAL_PS to RUN_MAX */
0x20e08001, /* WR:PMGR_PS_GPIO_PS ADDR=0x20e080118 */
0x00000046, /* Offset */
0x8000000f, /* Data */
0x00003004, /* Cfg Count(Delay.Wait) */
0x20e08001, /* WR:PMGR_PS_GPIO_PS ADDR=0x20e080118 */
0x00000046, /* Offset */
0x00000004, /* Data */
/* Comment: // Disable voltage changes */
0x20e0a001, /* WR:PMGR_VOLMAN_VOLMAN_CTL ADDR=0x20e0a0000 */
0x00000000, /* Offset */
0x00001f00, /* Data */
0x20e06811, /* 5-word write */
0x07060504, /* Offset */
0x00000000, /* Offset */
0x0000bb39, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0A ADDR=0x20e068010 */
0x90000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0B ADDR=0x20e068014 */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0C ADDR=0x20e068018 */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0D ADDR=0x20e06801c */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 */
0x20e06802, /* Poll:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x80000000, /* Mask */
0x00000000, /* Data */
/* Comment: // Per radar 19338346, disable DynPwrDnEn after switch to bucket 3 */

#ifdef AMC_PARAM_FOUR_CH_ONE_RANK
0x20040041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(0) ADDR=0x200400424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20044041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(1) ADDR=0x200440424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20048041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(2) ADDR=0x200480424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x2004c041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(3) ADDR=0x2004c0424 */
0x00000009, /* Offset */
0x00000132, /* Data */
#endif


#ifdef AMC_PARAM_EIGHT_CH_ONE_RANK
0x20040041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(0) ADDR=0x200400424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20044041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(1) ADDR=0x200440424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20048041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(2) ADDR=0x200480424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x2004c041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(3) ADDR=0x2004c0424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20050041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(4) ADDR=0x200500424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20054041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(5) ADDR=0x200540424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x20058041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(6) ADDR=0x200580424 */
0x00000009, /* Offset */
0x00000132, /* Data */
0x2005c041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(7) ADDR=0x2005c0424 */
0x00000009, /* Offset */
0x00000132, /* Data */
#endif

/* Comment: // Power off PLLs that software could not turn off (ENABLE == 0 and BYPASS == 1) */
0x20e00001, /* WR:PMGR_PLL0_CTL ADDR=0x20e000000 */
0x00000000, /* Offset */
0x280c8030, /* Data */
0x20e00401, /* WR:PMGR_PLL1_CTL ADDR=0x20e004000 */
0x00000000, /* Offset */
0x2803c010, /* Data */
0x20e01801, /* WR:PMGR_PLL6_CTL ADDR=0x20e018000 */
0x00000000, /* Offset */
0x28032011, /* Data */
0x20e01c01, /* WR:PMGR_PLL7_CTL ADDR=0x20e01c000 */
0x00000000, /* Offset */
0x2806f021, /* Data */
0x20e00c01, /* WR:PMGR_PLL3_CTL ADDR=0x20e00c000 */
0x00000000, /* Offset */
0x2818d06b, /* Data */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

static const uint32_t awake_to_aop_ddr_a0_pre_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AWAKE->AOP_DDR POSTAMBLE */
/* Comment: // ** */
/* Comment: // ** Note: */
/* Comment: // ** According to the PMGR spec section 11.14.4 (Software Teardown for Awake Exit), either */
/* Comment: // ** SW or the config engine may turn off SIO/PCIE.  For DV simulations, we chose to power */
/* Comment: // ** power down ALL of the unneeded blocks in the config engine rather than from the AP. This */
/* Comment: // ** provided functionally equivalent behavior and allowed the AWAKE->AOP_DDR and */
/* Comment: // ** S2R_AOP->AOP_DDR to appear symmetric.  SiVal/SW may move parts of this sequence */
/* Comment: // ** to AP software if desired. */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
static const uint32_t awake_to_aop_ddr_a0_post_0[] = {
/* Comment: // Ensure CoherencePoint DupTag ways will power off */
0x20e09d01, /* WR:PMGR_PWRGATE_PWR_DUP_TAGS ADDR=0x20e09d000 */
0x00000000, /* Offset */
0x00000001, /* Data */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // Enable automatic power management for the fabric and memory controller */
#define A2D_POST_PWRGATE_AFTER  0
#define A2D_POST_PWRGATE_BEFORE 1
/* Comment: // ****************************************************************************************** */
/* Comment: // ** SOC DV Version of AWAKE->AOP_DDR pwrgate sequence */
/* Comment: // ** */
/* Comment: // ** The primary purpose of this sequence is to set PWR_DOM_EN for the blocks which need */
/* Comment: // ** to be powered down in AOP_DDR to save power.  Since the same register is used for  */
/* Comment: // ** some tunable fields, the tunable values for these registers are also applied here. */
/* Comment: // ** */
/* Comment: // ** These values are hand-coded by the SOC DV team based on the tunable values on */
/* Comment: // ** 2/2/2015, and should not be used directly by the CoreOS team in case tunable values */
/* Comment: // ** change. */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
static const uint32_t awake_to_aop_ddr_a0_post_pwrgate[] = {
0x20e09c2d, /* 12-word write */
0x382a2928, /* Offset */
0x2d2c3a39, /* Offset */
0x3231302e, /* Offset */
0x80030309, /* Data WR:PMGR_PWRGATE_PWR_MCC_CFG0 ADDR=0x20e09c0a0 */
0x000f0003, /* Data WR:PMGR_PWRGATE_PWR_MCC_CFG1 ADDR=0x20e09c0a4 */
0x01060600, /* Data WR:PMGR_PWRGATE_PWR_MCC_CFG2 ADDR=0x20e09c0a8 */
0x80030303, /* Data WR:PMGR_PWRGATE_PWR_ACS_CFG0 ADDR=0x20e09c0e0 */
0x001d0000, /* Data WR:PMGR_PWRGATE_PWR_ACS_CFG1 ADDR=0x20e09c0e4 */
0x01030301, /* Data WR:PMGR_PWRGATE_PWR_ACS_CFG2 ADDR=0x20e09c0e8 */
0x80030301, /* Data WR:PMGR_PWRGATE_PWR_DCS0123_CFG0 ADDR=0x20e09c0b0 */
0x00190003, /* Data WR:PMGR_PWRGATE_PWR_DCS0123_CFG1 ADDR=0x20e09c0b4 */
0x010a0808, /* Data WR:PMGR_PWRGATE_PWR_DCS0123_CFG2 ADDR=0x20e09c0b8 */
0x80030301, /* Data WR:PMGR_PWRGATE_PWR_DCS4567_CFG0 ADDR=0x20e09c0c0 */
0x00150004, /* Data WR:PMGR_PWRGATE_PWR_DCS4567_CFG1 ADDR=0x20e09c0c4 */
0x010a0808 /* Data WR:PMGR_PWRGATE_PWR_DCS4567_CFG2 ADDR=0x20e09c0c8 */
};

static const uint32_t awake_to_aop_ddr_a0_post_1[] = {
/* Comment: // Per <rdar://problem/20676275>, DV will set PWR_DOM_EN for SIO/USB/PCIE/PMP with default values for other fields */
0x20e09c0d, /* 4-word write */
0x50603424, /* Offset */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_SIO_CFG0 ADDR=0x20e09c090 */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_USB_CFG0 ADDR=0x20e09c0d0 */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_PCIE_CFG0 ADDR=0x20e09c180 */
0x80080860, /* Data WR:PMGR_PWRGATE_PWR_PMP_CFG0 ADDR=0x20e09c140 */

#ifdef AMC_PARAM_FOUR_CH_ONE_RANK
0x20e08031, /* 13-word write */
0xb0b24048, /* Offset */
0x92908e8c, /* Offset */
0x9a989694, /* Offset */
0x0000009c, /* Offset */
0x1000000f, /* Data WR:PMGR_PS_PMS_PS ADDR=0x20e080120 */
0x1000000f, /* Data WR:PMGR_PS_SBR_PS ADDR=0x20e080100 */
0x1000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x1000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x1000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x1000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x1000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x1000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x1000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x00000000, /* Data WR:PMGR_PS_DCS4_PS ADDR=0x20e080258 */
0x00000000, /* Data WR:PMGR_PS_DCS5_PS ADDR=0x20e080260 */
0x00000000, /* Data WR:PMGR_PS_DCS6_PS ADDR=0x20e080268 */
0x00000000, /* Data WR:PMGR_PS_DCS7_PS ADDR=0x20e080270 */
#endif


#ifdef AMC_PARAM_EIGHT_CH_ONE_RANK
0x20e08031, /* 13-word write */
0xb0b24048, /* Offset */
0x92908e8c, /* Offset */
0x9a989694, /* Offset */
0x0000009c, /* Offset */
0x1000000f, /* Data WR:PMGR_PS_PMS_PS ADDR=0x20e080120 */
0x1000000f, /* Data WR:PMGR_PS_SBR_PS ADDR=0x20e080100 */
0x1000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x1000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x1000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x1000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x1000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x1000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x1000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x1000000f, /* Data WR:PMGR_PS_DCS4_PS ADDR=0x20e080258 */
0x1000000f, /* Data WR:PMGR_PS_DCS5_PS ADDR=0x20e080260 */
0x1000000f, /* Data WR:PMGR_PS_DCS6_PS ADDR=0x20e080268 */
0x1000000f, /* Data WR:PMGR_PS_DCS7_PS ADDR=0x20e080270 */
#endif

0x20e40001, /* WR:PMSCSR_PMSCSR_PMP_PMS_CPG_CTRL ADDR=0x20e40000c */
0x00000003, /* Offset */
0x00010100, /* Data */
0x20400001, /* WR:SOCBUSMUX_SOCBUSMUX_REGS_CPG_CNTL ADDR=0x204000090 */
0x00000024, /* Offset */
0x80012c04, /* Data */
0x20080001, /* WR:SWITCHFABRIC_SWITCHFABRIC_REGS_CPG_CNTL ADDR=0x200800094 */
0x00000025, /* Offset */
0x80017c04, /* Data */
0x20f00001, /* WR:AFC_AIU_SB_AFC_AIU_SB_SELF_REGS_CPG_CNTL ADDR=0x20f00000c */
0x00000003, /* Offset */
0x80011004, /* Data */
0x20f1c001, /* WR:DYNAMIC_CLK_GATING_SB_GLUE_DYNAMIC_CLK_GATING ADDR=0x20f1c0000 */
0x00000000, /* Offset */
0x0100001f, /* Data */
0x20000001, /* WR:AMCC_AMCCGEN_AMCCLKPWRGATE(0) ADDR=0x200000008 */
0x00000002, /* Offset */
0x10c80064, /* Data */
0x20020001, /* WR:AMCC_AMCCGEN_AMCCLKPWRGATE(1) ADDR=0x200200008 */
0x00000002, /* Offset */
0x10c80064, /* Data */
/* Comment: // enable ACG for the PMGR clock tree */
0x20e0dc01, /* WR:PMGR_MISC_CFG_ACG ADDR=0x20e0dc000 */
0x00000000, /* Offset */
0x00000001, /* Data */
/* Comment: // Powering down SIO, PCIE, USB, AIC, PMP */
0x20e0803d, /* 16-word write */
0x5e5c5a58, /* Offset */
0x66646260, /* Offset */
0x6e6c6a68, /* Offset */
0x76747270, /* Offset */
0x00000000, /* Data WR:PMGR_PS_MCA0_PS ADDR=0x20e080160 */
0x00000000, /* Data WR:PMGR_PS_MCA1_PS ADDR=0x20e080168 */
0x00000000, /* Data WR:PMGR_PS_MCA2_PS ADDR=0x20e080170 */
0x00000000, /* Data WR:PMGR_PS_MCA3_PS ADDR=0x20e080178 */
0x00000000, /* Data WR:PMGR_PS_MCA4_PS ADDR=0x20e080180 */
0x00000000, /* Data WR:PMGR_PS_PWM0_PS ADDR=0x20e080188 */
0x00000000, /* Data WR:PMGR_PS_I2C0_PS ADDR=0x20e080190 */
0x00000000, /* Data WR:PMGR_PS_I2C1_PS ADDR=0x20e080198 */
0x00000000, /* Data WR:PMGR_PS_I2C2_PS ADDR=0x20e0801a0 */
0x00000000, /* Data WR:PMGR_PS_I2C3_PS ADDR=0x20e0801a8 */
0x00000000, /* Data WR:PMGR_PS_SPI0_PS ADDR=0x20e0801b0 */
0x00000000, /* Data WR:PMGR_PS_SPI1_PS ADDR=0x20e0801b8 */
0x00000000, /* Data WR:PMGR_PS_SPI2_PS ADDR=0x20e0801c0 */
0x00000000, /* Data WR:PMGR_PS_SPI3_PS ADDR=0x20e0801c8 */
0x00000000, /* Data WR:PMGR_PS_UART0_PS ADDR=0x20e0801d0 */
0x00000000, /* Data WR:PMGR_PS_UART1_PS ADDR=0x20e0801d8 */
0x20e0803d, /* 16-word write */
0x7e7c7a78, /* Offset */
0x86848280, /* Offset */
0xd4525654, /* Offset */
0xdcdad8d6, /* Offset */
0x00000000, /* Data WR:PMGR_PS_UART2_PS ADDR=0x20e0801e0 */
0x00000000, /* Data WR:PMGR_PS_UART3_PS ADDR=0x20e0801e8 */
0x00000000, /* Data WR:PMGR_PS_UART4_PS ADDR=0x20e0801f0 */
0x00000000, /* Data WR:PMGR_PS_UART5_PS ADDR=0x20e0801f8 */
0x00000000, /* Data WR:PMGR_PS_UART6_PS ADDR=0x20e080200 */
0x00000000, /* Data WR:PMGR_PS_UART7_PS ADDR=0x20e080208 */
0x00000000, /* Data WR:PMGR_PS_UART8_PS ADDR=0x20e080210 */
0x00000000, /* Data WR:PMGR_PS_AES0_PS ADDR=0x20e080218 */
0x00000000, /* Data WR:PMGR_PS_SIO_P_PS ADDR=0x20e080150 */
0x00000000, /* Data WR:PMGR_PS_SIO_PS ADDR=0x20e080158 */
0x00000000, /* Data WR:PMGR_PS_SIO_BUSIF_PS ADDR=0x20e080148 */
0x00000000, /* Data WR:PMGR_PS_PCIE_AUX_PS ADDR=0x20e080350 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK0_PS ADDR=0x20e080358 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK1_PS ADDR=0x20e080360 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK2_PS ADDR=0x20e080368 */
0x00000000, /* Data WR:PMGR_PS_PCIE_LINK3_PS ADDR=0x20e080370 */
0x20e08031, /* 13-word write */
0xaca8a4d2, /* Offset */
0xa0aaa6a2, /* Offset */
0x44c89eae, /* Offset */
0x00000050, /* Offset */
0x00000000, /* Data WR:PMGR_PS_PCIE_PS ADDR=0x20e080348 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST0_OHCI_PS ADDR=0x20e080290 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST1_OHCI_PS ADDR=0x20e0802a0 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST2_OHCI_PS ADDR=0x20e0802b0 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST0_PS ADDR=0x20e080288 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST1_PS ADDR=0x20e080298 */
0x00000000, /* Data WR:PMGR_PS_USB2HOST2_PS ADDR=0x20e0802a8 */
0x00000000, /* Data WR:PMGR_PS_USBCTLREG_PS ADDR=0x20e080280 */
0x00000000, /* Data WR:PMGR_PS_USB_OTG_PS ADDR=0x20e0802b8 */
0x00000000, /* Data WR:PMGR_PS_USB_PS ADDR=0x20e080278 */
0x00000000, /* Data WR:PMGR_PS_PMP_PS ADDR=0x20e080320 */
0x00000000, /* Data WR:PMGR_PS_DWI_PS ADDR=0x20e080110 */
0x00000000, /* Data WR:PMGR_PS_PCIE_REF_PS ADDR=0x20e080140 */
0x20e0b83d, /* 16-word write */
0x03020100, /* Offset */
0x07060504, /* Offset */
0x13121110, /* Offset */
0x17161514, /* Offset */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH0 ADDR=0x20e0b8000 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH1 ADDR=0x20e0b8004 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH2 ADDR=0x20e0b8008 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH3 ADDR=0x20e0b800c */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH4 ADDR=0x20e0b8010 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH5 ADDR=0x20e0b8014 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH6 ADDR=0x20e0b8018 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH7 ADDR=0x20e0b801c */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH16 ADDR=0x20e0b8040 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH17 ADDR=0x20e0b8044 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH18 ADDR=0x20e0b8048 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH19 ADDR=0x20e0b804c */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH20 ADDR=0x20e0b8050 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH21 ADDR=0x20e0b8054 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH22 ADDR=0x20e0b8058 */
0x00000000, /* Data WR:PMGR_SCRATCH_SCRATCH23 ADDR=0x20e0b805c */
/* Comment: // Reset and then clock gate the AIC */
0x20e08001, /* WR:PMGR_PS_AIC_PS ADDR=0x20e080108 */
0x00000042, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_AIC_PS ADDR=0x20e080108 Retry_en=1 Retry_cnt=255*/
0x0001ff42, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e10001, /* WR:AIC_GLOBALS_AICRST ADDR=0x20e10000c */
0x00000003, /* Offset */
0x00000001, /* Data */
0x20e10002, /* Poll:AIC_GLOBALS_AICRST ADDR=0x20e10000c Retry_en=1 Retry_cnt=255*/
0x0001ff03, /* Offset */
0x00000001, /* Mask */
0x00000000, /* Data */
0x20e08001, /* WR:PMGR_PS_AIC_PS ADDR=0x20e080108 */
0x00000042, /* Offset */
0x00000000, /* Data */
0x2102b005, /* 2-word write */
0x00000400, /* Offset */
0x00000000, /* Data WR:MINIPMGR_WATCHDOG_CHIP_WATCHDOG_TIMER ADDR=0x2102b0000 */
0x00000000 /* Data WR:MINIPMGR_WATCHDOG_SYS_WATCHDOG_TIMER ADDR=0x2102b0010 */
};

static const uint32_t awake_to_aop_ddr_a0_post_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP_DDR->AWAKE PREAMBLE */
/* Comment: // ****************************************************************************************** */
static const uint32_t aop_ddr_to_awake_a0_pre_0[] = {
/* Comment: // Make the proxy fabric clock run at 192MHz to speed up AOP_DDR->AWAKE transition */
0x21024002, /* Poll:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 Retry_en=1 Retry_cnt=255*/
0x0001ff06, /* Offset */
0x04000000, /* Mask */
0x00000000, /* Data */
0x21024001, /* WR:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 */
0x00000006, /* Offset */
0x81100000, /* Data */
/* Comment: // Powering up SIO partition and SIO devices */
0x20e08001, /* WR:PMGR_PS_SIO_BUSIF_PS ADDR=0x20e080148 */
0x00000052, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_SIO_BUSIF_PS ADDR=0x20e080148 Retry_en=1 Retry_cnt=255*/
0x0001ff52, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08001, /* WR:PMGR_PS_SIO_PS ADDR=0x20e080158 */
0x00000056, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_SIO_PS ADDR=0x20e080158 Retry_en=1 Retry_cnt=255*/
0x0001ff56, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08001, /* WR:PMGR_PS_SIO_P_PS ADDR=0x20e080150 */
0x00000054, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_SIO_P_PS ADDR=0x20e080150 Retry_en=1 Retry_cnt=255*/
0x0001ff54, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e0803d, /* 16-word write */
0x5e5c5a58, /* Offset */
0x66646260, /* Offset */
0x6e6c6a68, /* Offset */
0x76747270, /* Offset */
0x00000004, /* Data WR:PMGR_PS_MCA0_PS ADDR=0x20e080160 */
0x00000004, /* Data WR:PMGR_PS_MCA1_PS ADDR=0x20e080168 */
0x00000004, /* Data WR:PMGR_PS_MCA2_PS ADDR=0x20e080170 */
0x00000004, /* Data WR:PMGR_PS_MCA3_PS ADDR=0x20e080178 */
0x00000004, /* Data WR:PMGR_PS_MCA4_PS ADDR=0x20e080180 */
0x00000004, /* Data WR:PMGR_PS_PWM0_PS ADDR=0x20e080188 */
0x00000004, /* Data WR:PMGR_PS_I2C0_PS ADDR=0x20e080190 */
0x00000004, /* Data WR:PMGR_PS_I2C1_PS ADDR=0x20e080198 */
0x00000004, /* Data WR:PMGR_PS_I2C2_PS ADDR=0x20e0801a0 */
0x00000004, /* Data WR:PMGR_PS_I2C3_PS ADDR=0x20e0801a8 */
0x00000004, /* Data WR:PMGR_PS_SPI0_PS ADDR=0x20e0801b0 */
0x00000004, /* Data WR:PMGR_PS_SPI1_PS ADDR=0x20e0801b8 */
0x00000004, /* Data WR:PMGR_PS_SPI2_PS ADDR=0x20e0801c0 */
0x00000004, /* Data WR:PMGR_PS_SPI3_PS ADDR=0x20e0801c8 */
0x00000004, /* Data WR:PMGR_PS_UART0_PS ADDR=0x20e0801d0 */
0x00000004, /* Data WR:PMGR_PS_UART1_PS ADDR=0x20e0801d8 */
0x20e0801d, /* 8-word write */
0x7e7c7a78, /* Offset */
0x86848280, /* Offset */
0x00000004, /* Data WR:PMGR_PS_UART2_PS ADDR=0x20e0801e0 */
0x00000004, /* Data WR:PMGR_PS_UART3_PS ADDR=0x20e0801e8 */
0x00000004, /* Data WR:PMGR_PS_UART4_PS ADDR=0x20e0801f0 */
0x00000004, /* Data WR:PMGR_PS_UART5_PS ADDR=0x20e0801f8 */
0x00000004, /* Data WR:PMGR_PS_UART6_PS ADDR=0x20e080200 */
0x00000004, /* Data WR:PMGR_PS_UART7_PS ADDR=0x20e080208 */
0x00000004, /* Data WR:PMGR_PS_UART8_PS ADDR=0x20e080210 */
0x00000004, /* Data WR:PMGR_PS_AES0_PS ADDR=0x20e080218 */
/* Comment: // It might not be required to do all of the following polls since they should take the same amount of time to power up. */
/* Comment: // One possible optimization is to only do the poll for the last write */
0x20e08002, /* Poll:PMGR_PS_MCA0_PS ADDR=0x20e080160 Retry_en=1 Retry_cnt=255*/
0x0001ff58, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCA1_PS ADDR=0x20e080168 Retry_en=1 Retry_cnt=255*/
0x0001ff5a, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCA2_PS ADDR=0x20e080170 Retry_en=1 Retry_cnt=255*/
0x0001ff5c, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCA3_PS ADDR=0x20e080178 Retry_en=1 Retry_cnt=255*/
0x0001ff5e, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCA4_PS ADDR=0x20e080180 Retry_en=1 Retry_cnt=255*/
0x0001ff60, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PWM0_PS ADDR=0x20e080188 Retry_en=1 Retry_cnt=255*/
0x0001ff62, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_I2C0_PS ADDR=0x20e080190 Retry_en=1 Retry_cnt=255*/
0x0001ff64, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_I2C1_PS ADDR=0x20e080198 Retry_en=1 Retry_cnt=255*/
0x0001ff66, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_I2C2_PS ADDR=0x20e0801a0 Retry_en=1 Retry_cnt=255*/
0x0001ff68, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_I2C3_PS ADDR=0x20e0801a8 Retry_en=1 Retry_cnt=255*/
0x0001ff6a, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_SPI0_PS ADDR=0x20e0801b0 Retry_en=1 Retry_cnt=255*/
0x0001ff6c, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_SPI1_PS ADDR=0x20e0801b8 Retry_en=1 Retry_cnt=255*/
0x0001ff6e, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_SPI2_PS ADDR=0x20e0801c0 Retry_en=1 Retry_cnt=255*/
0x0001ff70, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_SPI3_PS ADDR=0x20e0801c8 Retry_en=1 Retry_cnt=255*/
0x0001ff72, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART0_PS ADDR=0x20e0801d0 Retry_en=1 Retry_cnt=255*/
0x0001ff74, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART1_PS ADDR=0x20e0801d8 Retry_en=1 Retry_cnt=255*/
0x0001ff76, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART2_PS ADDR=0x20e0801e0 Retry_en=1 Retry_cnt=255*/
0x0001ff78, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART3_PS ADDR=0x20e0801e8 Retry_en=1 Retry_cnt=255*/
0x0001ff7a, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART4_PS ADDR=0x20e0801f0 Retry_en=1 Retry_cnt=255*/
0x0001ff7c, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART5_PS ADDR=0x20e0801f8 Retry_en=1 Retry_cnt=255*/
0x0001ff7e, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART6_PS ADDR=0x20e080200 Retry_en=1 Retry_cnt=255*/
0x0001ff80, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART7_PS ADDR=0x20e080208 Retry_en=1 Retry_cnt=255*/
0x0001ff82, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_UART8_PS ADDR=0x20e080210 Retry_en=1 Retry_cnt=255*/
0x0001ff84, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_AES0_PS ADDR=0x20e080218 Retry_en=1 Retry_cnt=255*/
0x0001ff86, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
/* Comment: // Powering up PCIE partition and PCIE devices/link */
0x20e08001, /* WR:PMGR_PS_PCIE_PS ADDR=0x20e080348 */
0x000000d2, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_PS ADDR=0x20e080348 Retry_en=1 Retry_cnt=255*/
0x0001ffd2, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08011, /* 5-word write */
0xdad8d6d4, /* Offset */
0x000000dc, /* Offset */
0x00000004, /* Data WR:PMGR_PS_PCIE_AUX_PS ADDR=0x20e080350 */
0x00000004, /* Data WR:PMGR_PS_PCIE_LINK0_PS ADDR=0x20e080358 */
0x00000004, /* Data WR:PMGR_PS_PCIE_LINK1_PS ADDR=0x20e080360 */
0x00000004, /* Data WR:PMGR_PS_PCIE_LINK2_PS ADDR=0x20e080368 */
0x00000004, /* Data WR:PMGR_PS_PCIE_LINK3_PS ADDR=0x20e080370 */
0x20e08002, /* Poll:PMGR_PS_PCIE_AUX_PS ADDR=0x20e080350 Retry_en=1 Retry_cnt=255*/
0x0001ffd4, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_LINK0_PS ADDR=0x20e080358 Retry_en=1 Retry_cnt=255*/
0x0001ffd6, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_LINK1_PS ADDR=0x20e080360 Retry_en=1 Retry_cnt=255*/
0x0001ffd8, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_LINK2_PS ADDR=0x20e080368 Retry_en=1 Retry_cnt=255*/
0x0001ffda, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_LINK3_PS ADDR=0x20e080370 Retry_en=1 Retry_cnt=255*/
0x0001ffdc, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
/* Comment: // Powering up USB partition and USB devices */
0x20e08001, /* WR:PMGR_PS_USB_PS ADDR=0x20e080278 */
0x0000009e, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB_PS ADDR=0x20e080278 Retry_en=1 Retry_cnt=255*/
0x0001ff9e, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08011, /* 5-word write */
0xa6a2a0ae, /* Offset */
0x000000aa, /* Offset */
0x00000004, /* Data WR:PMGR_PS_USB_OTG_PS ADDR=0x20e0802b8 */
0x00000004, /* Data WR:PMGR_PS_USBCTLREG_PS ADDR=0x20e080280 */
0x00000004, /* Data WR:PMGR_PS_USB2HOST0_PS ADDR=0x20e080288 */
0x00000004, /* Data WR:PMGR_PS_USB2HOST1_PS ADDR=0x20e080298 */
0x00000004, /* Data WR:PMGR_PS_USB2HOST2_PS ADDR=0x20e0802a8 */
0x20e08002, /* Poll:PMGR_PS_USB_OTG_PS ADDR=0x20e0802b8 Retry_en=1 Retry_cnt=255*/
0x0001ffae, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_USBCTLREG_PS ADDR=0x20e080280 Retry_en=1 Retry_cnt=255*/
0x0001ffa0, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB2HOST0_PS ADDR=0x20e080288 Retry_en=1 Retry_cnt=255*/
0x0001ffa2, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB2HOST1_PS ADDR=0x20e080298 Retry_en=1 Retry_cnt=255*/
0x0001ffa6, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB2HOST2_PS ADDR=0x20e0802a8 Retry_en=1 Retry_cnt=255*/
0x0001ffaa, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08009, /* 3-word write */
0x00aca8a4, /* Offset */
0x00000004, /* Data WR:PMGR_PS_USB2HOST0_OHCI_PS ADDR=0x20e080290 */
0x00000004, /* Data WR:PMGR_PS_USB2HOST1_OHCI_PS ADDR=0x20e0802a0 */
0x00000004, /* Data WR:PMGR_PS_USB2HOST2_OHCI_PS ADDR=0x20e0802b0 */
0x20e08002, /* Poll:PMGR_PS_USB2HOST0_OHCI_PS ADDR=0x20e080290 Retry_en=1 Retry_cnt=255*/
0x0001ffa4, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB2HOST1_OHCI_PS ADDR=0x20e0802a0 Retry_en=1 Retry_cnt=255*/
0x0001ffa8, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_USB2HOST2_OHCI_PS ADDR=0x20e0802b0 Retry_en=1 Retry_cnt=255*/
0x0001ffac, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
/* Comment: // Powering up AIC */
0x20e08001, /* WR:PMGR_PS_AIC_PS ADDR=0x20e080108 */
0x00000042, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_AIC_PS ADDR=0x20e080108 Retry_en=1 Retry_cnt=255*/
0x0001ff42, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_PMP_PS ADDR=0x20e080320 */
0x000000c8, /* Offset */
0x00000004, /* Data */
0x20e08002, /* Poll:PMGR_PS_PMP_PS ADDR=0x20e080320 Retry_en=1 Retry_cnt=255*/
0x0001ffc8, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
/* Comment: // Restore PMS, SBR, SF, SMX, MCC, and DCS* PS to reset values per CoreOS request from radar 19132562 */
0x00000044, /* NOP to align binary to 64-bit boundary before ifdef */

#ifdef AMC_PARAM_FOUR_CH_ONE_RANK
0x20e0802d, /* 12-word write */
0xb0b24048, /* Offset */
0x92908e8c, /* Offset */
0x50444694, /* Offset */
0x0000000f, /* Data WR:PMGR_PS_PMS_PS ADDR=0x20e080120 */
0x0000000f, /* Data WR:PMGR_PS_SBR_PS ADDR=0x20e080100 */
0x0000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x0000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x0000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x0000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x0000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x0000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x0000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x0000000f, /* Data WR:PMGR_PS_GPIO_PS ADDR=0x20e080118 */
0x00000004, /* Data WR:PMGR_PS_DWI_PS ADDR=0x20e080110 */
0x00000004, /* Data WR:PMGR_PS_PCIE_REF_PS ADDR=0x20e080140 */
0x20e08002, /* Poll:PMGR_PS_PMS_PS ADDR=0x20e080120 Retry_en=1 Retry_cnt=255*/
0x0001ff48, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SBR_PS ADDR=0x20e080100 Retry_en=1 Retry_cnt=255*/
0x0001ff40, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SF_PS ADDR=0x20e0802c8 Retry_en=1 Retry_cnt=255*/
0x0001ffb2, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SMX_PS ADDR=0x20e0802c0 Retry_en=1 Retry_cnt=255*/
0x0001ffb0, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCC_PS ADDR=0x20e080230 Retry_en=1 Retry_cnt=255*/
0x0001ff8c, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS0_PS ADDR=0x20e080238 Retry_en=1 Retry_cnt=255*/
0x0001ff8e, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS1_PS ADDR=0x20e080240 Retry_en=1 Retry_cnt=255*/
0x0001ff90, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS2_PS ADDR=0x20e080248 Retry_en=1 Retry_cnt=255*/
0x0001ff92, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS3_PS ADDR=0x20e080250 Retry_en=1 Retry_cnt=255*/
0x0001ff94, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_GPIO_PS ADDR=0x20e080118 Retry_en=1 Retry_cnt=255*/
0x0001ff46, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DWI_PS ADDR=0x20e080110 Retry_en=1 Retry_cnt=255*/
0x0001ff44, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_REF_PS ADDR=0x20e080140 Retry_en=1 Retry_cnt=255*/
0x0001ff50, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
#endif


#ifdef AMC_PARAM_EIGHT_CH_ONE_RANK
0x20e0803d, /* 16-word write */
0xb0b24048, /* Offset */
0x92908e8c, /* Offset */
0x9a989694, /* Offset */
0x5044469c, /* Offset */
0x0000000f, /* Data WR:PMGR_PS_PMS_PS ADDR=0x20e080120 */
0x0000000f, /* Data WR:PMGR_PS_SBR_PS ADDR=0x20e080100 */
0x0000000f, /* Data WR:PMGR_PS_SF_PS ADDR=0x20e0802c8 */
0x0000000f, /* Data WR:PMGR_PS_SMX_PS ADDR=0x20e0802c0 */
0x0000000f, /* Data WR:PMGR_PS_MCC_PS ADDR=0x20e080230 */
0x0000000f, /* Data WR:PMGR_PS_DCS0_PS ADDR=0x20e080238 */
0x0000000f, /* Data WR:PMGR_PS_DCS1_PS ADDR=0x20e080240 */
0x0000000f, /* Data WR:PMGR_PS_DCS2_PS ADDR=0x20e080248 */
0x0000000f, /* Data WR:PMGR_PS_DCS3_PS ADDR=0x20e080250 */
0x0000000f, /* Data WR:PMGR_PS_DCS4_PS ADDR=0x20e080258 */
0x0000000f, /* Data WR:PMGR_PS_DCS5_PS ADDR=0x20e080260 */
0x0000000f, /* Data WR:PMGR_PS_DCS6_PS ADDR=0x20e080268 */
0x0000000f, /* Data WR:PMGR_PS_DCS7_PS ADDR=0x20e080270 */
0x0000000f, /* Data WR:PMGR_PS_GPIO_PS ADDR=0x20e080118 */
0x00000004, /* Data WR:PMGR_PS_DWI_PS ADDR=0x20e080110 */
0x00000004, /* Data WR:PMGR_PS_PCIE_REF_PS ADDR=0x20e080140 */
0x20e08002, /* Poll:PMGR_PS_PMS_PS ADDR=0x20e080120 Retry_en=1 Retry_cnt=255*/
0x0001ff48, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SBR_PS ADDR=0x20e080100 Retry_en=1 Retry_cnt=255*/
0x0001ff40, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SF_PS ADDR=0x20e0802c8 Retry_en=1 Retry_cnt=255*/
0x0001ffb2, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_SMX_PS ADDR=0x20e0802c0 Retry_en=1 Retry_cnt=255*/
0x0001ffb0, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_MCC_PS ADDR=0x20e080230 Retry_en=1 Retry_cnt=255*/
0x0001ff8c, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS0_PS ADDR=0x20e080238 Retry_en=1 Retry_cnt=255*/
0x0001ff8e, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS1_PS ADDR=0x20e080240 Retry_en=1 Retry_cnt=255*/
0x0001ff90, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS2_PS ADDR=0x20e080248 Retry_en=1 Retry_cnt=255*/
0x0001ff92, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS3_PS ADDR=0x20e080250 Retry_en=1 Retry_cnt=255*/
0x0001ff94, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS4_PS ADDR=0x20e080258 Retry_en=1 Retry_cnt=255*/
0x0001ff96, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS5_PS ADDR=0x20e080260 Retry_en=1 Retry_cnt=255*/
0x0001ff98, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS6_PS ADDR=0x20e080268 Retry_en=1 Retry_cnt=255*/
0x0001ff9a, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DCS7_PS ADDR=0x20e080270 Retry_en=1 Retry_cnt=255*/
0x0001ff9c, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_GPIO_PS ADDR=0x20e080118 Retry_en=1 Retry_cnt=255*/
0x0001ff46, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08002, /* Poll:PMGR_PS_DWI_PS ADDR=0x20e080110 Retry_en=1 Retry_cnt=255*/
0x0001ff44, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x20e08002, /* Poll:PMGR_PS_PCIE_REF_PS ADDR=0x20e080140 Retry_en=1 Retry_cnt=255*/
0x0001ff50, /* Offset */
0x000000f0, /* Mask */
0x00000040, /* Data */
0x00000044, /* NOP to align binary to 64-bit boundary before endif */
#endif

0x20e04001, /* WR:PMGR_CLKCFG_UVD_CLK_CFG ADDR=0x20e0400a0 */
0x00000028, /* Offset */
0x80101000, /* Data */
/* Comment: // wait 1us */
0x00003004 /* Cfg Count(Delay.Wait) */
};

static const uint32_t aop_ddr_to_awake_a0_pre_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP_DDR->AWAKE POSTAMBLE */
/* Comment: // ** */
/* Comment: // ** Note: */
/* Comment: // ** According to the PMGR spec section 11.14.3 (Software Init for AOP_DDR to AWAKE), the */
/* Comment: // ** config engine restores the state of the chip in this step.  The specific use cases where */
/* Comment: // ** the config engine is the owner of state restoration are not yet defined. */
/* Comment: // ** */
/* Comment: // ** The sequence below contains the minimum set of commands to re-boot the AP and restore */
/* Comment: // ** DRAM operation at bucket 1.  AP may bring DRAM up to bucket 0. */
/* Comment: // ** */
/* Comment: // ** It is assumed that SW will add extensively to this sequence for state restoration based */
/* Comment: // ** on future use cases. */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
#define D2A_POST_MCU_AWAKE_AFTER  -1
#define D2A_POST_MCU_AWAKE_BEFORE 0
static const uint32_t aop_ddr_to_awake_a0_post_0[] = {
/* Comment: // Wait 900us as first step of AOP_DDR->AWAKE per radar 20181763 to ensure BIRA loading is complete */
/* Comment: // prior to powering on any non-AOP power domain */
0x00a8c004, /* Cfg Count(Delay.Wait) */
/* Comment: // INSERT_MCU_AWAKE_SEQ */
/* Comment: // Restore the state of the chip */
0x20e04001, /* WR:PMGR_CLKCFG_UVD_CLK_CFG ADDR=0x20e0400a0 */
0x00000028, /* Offset */
0x80100000, /* Data */
/* Comment: // wait 1us */
0x00003004, /* Cfg Count(Delay.Wait) */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

#define D2A_POST_RESTORE_AFTER  0
#define D2A_POST_RESTORE_BEFORE 1
/* Comment: // ****************************************************************************************** */
/* Comment: // ** SOC DV Version of AOP_DDR->AWAKE system restore postamble */
/* Comment: // ** */
/* Comment: // ** DO NOT USE THIS FILE VERBATIM -- THIS IS FOR DV TESTING PURPOSES ONLY */
/* Comment: // ****************************************************************************************** */
static const uint32_t aop_ddr_to_awake_a0_post_restore[] = {
/* Comment: // DV writes small values for PLL_RESET_*TIME and LOCK_TIME to speed up simulation */
/* Comment: // SW/SiVal should use real values here */
0x20e00001, /* WR:PMGR_PLL0_PLL_DELAY_CTL1 ADDR=0x20e000020 */
0x00000008, /* Offset */
0x0001a019, /* Data */
0x20e00005, /* 2-word write */
0x00000001, /* Offset */
0x80480030, /* Data WR:PMGR_PLL0_CFG ADDR=0x20e000004 */
0xa00c8030, /* Data WR:PMGR_PLL0_CTL ADDR=0x20e000000 */
0x20e00002, /* Poll:PMGR_PLL0_CTL ADDR=0x20e000000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x02000000, /* Mask */
0x00000000, /* Data */
0x20e00401, /* WR:PMGR_PLL1_PLL_DELAY_CTL1 ADDR=0x20e004020 */
0x00000008, /* Offset */
0x0001a019, /* Data */
0x20e00405, /* 2-word write */
0x00000001, /* Offset */
0x80480030, /* Data WR:PMGR_PLL1_CFG ADDR=0x20e004004 */
0xa003c010, /* Data WR:PMGR_PLL1_CTL ADDR=0x20e004000 */
0x20e00402, /* Poll:PMGR_PLL1_CTL ADDR=0x20e004000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x02000000, /* Mask */
0x00000000, /* Data */
0x20e01801, /* WR:PMGR_PLL6_PLL_DELAY_CTL1 ADDR=0x20e018020 */
0x00000008, /* Offset */
0x0001a019, /* Data */
0x20e01805, /* 2-word write */
0x00000001, /* Offset */
0x80480030, /* Data WR:PMGR_PLL6_CFG ADDR=0x20e018004 */
0xa0032011, /* Data WR:PMGR_PLL6_CTL ADDR=0x20e018000 */
0x20e01802, /* Poll:PMGR_PLL6_CTL ADDR=0x20e018000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x02000000, /* Mask */
0x00000000, /* Data */
0x20e01c01, /* WR:PMGR_PLL7_PLL_DELAY_CTL1 ADDR=0x20e01c020 */
0x00000008, /* Offset */
0x0001a019, /* Data */
0x20e01c05, /* 2-word write */
0x00000001, /* Offset */
0x80480030, /* Data WR:PMGR_PLL7_CFG ADDR=0x20e01c004 */
0xa006f021, /* Data WR:PMGR_PLL7_CTL ADDR=0x20e01c000 */
0x20e01c02, /* Poll:PMGR_PLL7_CTL ADDR=0x20e01c000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x02000000, /* Mask */
0x00000000, /* Data */
0x20e00c01, /* WR:PMGR_PLL3_PLL_DELAY_CTL1 ADDR=0x20e00c020 */
0x00000008, /* Offset */
0x0001a019, /* Data */
0x20e00c05, /* 2-word write */
0x00000001, /* Offset */
0x80480030, /* Data WR:PMGR_PLL3_CFG ADDR=0x20e00c004 */
0xa018d06b, /* Data WR:PMGR_PLL3_CTL ADDR=0x20e00c000 */
0x20e00c02, /* Poll:PMGR_PLL3_CTL ADDR=0x20e00c000 Retry_en=1 Retry_cnt=255*/
0x0001ff00, /* Offset */
0x02000000, /* Mask */
0x00000000, /* Data */
/* Comment: // Bring MCU up to bucket0 */
/* Comment: // write: reg_name=PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0A  reg_value=55555539 */
0x20e06819, /* 7-word write */
0x10060504, /* Offset */
0x00001211, /* Offset */
0x55555505, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0A ADDR=0x20e068010 */
0x55055666, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0B ADDR=0x20e068014 */
0x00000555, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_0C ADDR=0x20e068018 */
0x5555bb39, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_3A ADDR=0x20e068040 */
0x55055666, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_3B ADDR=0x20e068044 */
0x00000555, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_ENTRY_3C ADDR=0x20e068048 */
0x00000000, /* Data WR:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 */
0x20e06802, /* Poll:PMGR_SOC_PERF_STATE_SOC_PERF_STATE_CTL ADDR=0x20e068000 Retry_en=0 Retry_cnt=0*/
0x00000000, /* Offset */
0x80000000, /* Mask */
0x00000000, /* Data */
/* Comment: // Tunables may be re-applied here */
0x20e08001, /* WR:PMGR_PS_DWI_PS ADDR=0x20e080110 */
0x00000044, /* Offset */
0x0000024f, /* Data */
0x20e0dc01, /* WR:PMGR_MISC_CFG_ACG ADDR=0x20e0dc000 */
0x00000000, /* Offset */
0x00000001, /* Data */
0x00000044, /* CFG Count to aling 64 bit write to 16 bytes address:*/
0x20205003, /* WR:ACC_CPU0_IMPL_IO_RVBAR ADDR=0x202050000 */
0x00000000, /* Offset */
0x00000000, /* Data */
0x00000008 /* Data */
};

static const uint32_t aop_ddr_to_awake_a0_post_1[] = {

#ifdef AMC_PARAM_FOUR_CH_ONE_RANK
0x20040041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(0) ADDR=0x200400424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20044041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(1) ADDR=0x200440424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20048041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(2) ADDR=0x200480424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x2004c041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(3) ADDR=0x2004c0424 */
0x00000009, /* Offset */
0x00000133, /* Data */
#endif


#ifdef AMC_PARAM_EIGHT_CH_ONE_RANK
0x20040041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(0) ADDR=0x200400424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20044041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(1) ADDR=0x200440424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20048041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(2) ADDR=0x200480424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x2004c041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(3) ADDR=0x2004c0424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20050041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(4) ADDR=0x200500424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20054041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(5) ADDR=0x200540424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x20058041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(6) ADDR=0x200580424 */
0x00000009, /* Offset */
0x00000133, /* Data */
0x2005c041, /* WR:AMCX_DRAMCFG_PWRMNGTEN(7) ADDR=0x2005c0424 */
0x00000009, /* Offset */
0x00000133, /* Data */
#endif

/* Comment: // Per radar 20183074, turn the following partitions on one at a time */
0x20e08001, /* WR:PMGR_PS_RTMUX_PS ADDR=0x20e0802d0 */
0x000000b4, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_RTMUX_PS ADDR=0x20e0802d0 Retry_en=0 Retry_cnt=0*/
0x000000b4, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_ISP_PS ADDR=0x20e080300 */
0x000000c0, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_ISP_PS ADDR=0x20e080300 Retry_en=0 Retry_cnt=0*/
0x000000c0, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_MEDIA_PS ADDR=0x20e080308 */
0x000000c2, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_MEDIA_PS ADDR=0x20e080308 Retry_en=0 Retry_cnt=0*/
0x000000c2, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_MSR_PS ADDR=0x20e080318 */
0x000000c6, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_MSR_PS ADDR=0x20e080318 Retry_en=0 Retry_cnt=0*/
0x000000c6, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_VDEC0_PS ADDR=0x20e080330 */
0x000000cc, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_VDEC0_PS ADDR=0x20e080330 Retry_en=0 Retry_cnt=0*/
0x000000cc, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_VENC_CPU_PS ADDR=0x20e080340 */
0x000000d0, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_VENC_CPU_PS ADDR=0x20e080340 Retry_en=0 Retry_cnt=0*/
0x000000d0, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08801, /* WR:PMGR_PS_VENC_PIPE_PS ADDR=0x20e088000 */
0x00000000, /* Offset */
0x0000000f, /* Data */
0x20e08802, /* Poll:PMGR_PS_VENC_PIPE_PS ADDR=0x20e088000 Retry_en=0 Retry_cnt=0*/
0x00000000, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08801, /* WR:PMGR_PS_VENC_ME0_PS ADDR=0x20e088008 */
0x00000002, /* Offset */
0x0000000f, /* Data */
0x20e08802, /* Poll:PMGR_PS_VENC_ME0_PS ADDR=0x20e088008 Retry_en=0 Retry_cnt=0*/
0x00000002, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08801, /* WR:PMGR_PS_VENC_ME1_PS ADDR=0x20e088010 */
0x00000004, /* Offset */
0x0000000f, /* Data */
0x20e08802, /* Poll:PMGR_PS_VENC_ME1_PS ADDR=0x20e088010 Retry_en=0 Retry_cnt=0*/
0x00000004, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_GFX_PS ADDR=0x20e080388 */
0x000000e2, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_GFX_PS ADDR=0x20e080388 Retry_en=0 Retry_cnt=0*/
0x000000e2, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_DISP1MUX_PS ADDR=0x20e0802e8 */
0x000000ba, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_DISP1MUX_PS ADDR=0x20e0802e8 Retry_en=0 Retry_cnt=0*/
0x000000ba, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08001, /* WR:PMGR_PS_SRS_PS ADDR=0x20e080390 */
0x000000e4, /* Offset */
0x0000000f, /* Data */
0x20e08002, /* Poll:PMGR_PS_SRS_PS ADDR=0x20e080390 Retry_en=0 Retry_cnt=0*/
0x000000e4, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
0x20e08041, /* WR:PMGR_PS_SEP_PS ADDR=0x20e080400 */
0x00000000, /* Offset */
0x4000000f, /* Data */
0x20e08042, /* Poll:PMGR_PS_SEP_PS ADDR=0x20e080400 Retry_en=0 Retry_cnt=0*/
0x00000000, /* Offset */
0x000000f0, /* Mask */
0x000000f0, /* Data */
/* Comment: // Per CoreOS request, turn off the above PS registers (except SEP_PS, which cannot be turned off by config engine) */
0x20e08809, /* 3-word write */
0x00040200, /* Offset */
0x00000000, /* Data WR:PMGR_PS_VENC_PIPE_PS ADDR=0x20e088000 */
0x00000000, /* Data WR:PMGR_PS_VENC_ME0_PS ADDR=0x20e088008 */
0x00000000, /* Data WR:PMGR_PS_VENC_ME1_PS ADDR=0x20e088010 */
0x20e08021, /* 9-word write */
0xd0e2bae4, /* Offset */
0xc0c2c6cc, /* Offset */
0x000000b4, /* Offset */
0x00000000, /* Data WR:PMGR_PS_SRS_PS ADDR=0x20e080390 */
0x00000000, /* Data WR:PMGR_PS_DISP1MUX_PS ADDR=0x20e0802e8 */
0x00000000, /* Data WR:PMGR_PS_GFX_PS ADDR=0x20e080388 */
0x00000000, /* Data WR:PMGR_PS_VENC_CPU_PS ADDR=0x20e080340 */
0x00000000, /* Data WR:PMGR_PS_VDEC0_PS ADDR=0x20e080330 */
0x00000000, /* Data WR:PMGR_PS_MSR_PS ADDR=0x20e080318 */
0x00000000, /* Data WR:PMGR_PS_MEDIA_PS ADDR=0x20e080308 */
0x00000000, /* Data WR:PMGR_PS_ISP_PS ADDR=0x20e080300 */
0x00000000, /* Data WR:PMGR_PS_RTMUX_PS ADDR=0x20e0802d0 */
/* Comment: // Set the proxy clock back to 96MHz to save power */
0x21024002, /* Poll:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 Retry_en=1 Retry_cnt=255*/
0x0001ff06, /* Offset */
0x04000000, /* Mask */
0x00000000, /* Data */
0x21024001, /* WR:MINIPMGR_MINI_CLKCFG_PROXY_FABRIC_CLK_CFG ADDR=0x210240018 */
0x00000006, /* Offset */
0x82100000, /* Data */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

//
// This additional stitch point is an intentional deviation from the SEG-provided
// sequence.
//
#define D2A_POST_PREBOOT_BEFORE 2
static const uint32_t aop_ddr_to_awake_a0_post_2[] = {
/* Comment: // Boot CPU0 by writing to the WAKE_CORES register */
0x20e0d401, /* WR:PMGR_MISC_CORES_WAKE_CORES ADDR=0x20e0d4008 */
0x00000002, /* Offset */
0x00000001, /* Data */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

static const uint32_t aop_ddr_to_awake_a0_post_end[] = {
0x00000000, /*Cfg End */
0x00000044 /* NOP to align binary to 64-bit boundary */
};

#if 0
/*Intentional removal of the following definition which would otherwise duplicate the definition from the A1 sequence*/
typedef struct {
    uint32_t *sequence;
    size_t elements;
} reconfig_subsequence_t;
#endif

static const reconfig_subsequence_t reconfig_aop_ddr_to_awake_a0_pre_seqs[] = {
   { (uint32_t *) aop_ddr_to_awake_a0_pre_0, sizeof(aop_ddr_to_awake_a0_pre_0) / sizeof(uint32_t) },
   { NULL, 0 },
};

static const reconfig_subsequence_t reconfig_awake_to_aop_ddr_a0_pre_seqs[] = {
   { (uint32_t *) awake_to_aop_ddr_a0_pre_0, sizeof(awake_to_aop_ddr_a0_pre_0) / sizeof(uint32_t) },
   { NULL, 0 },
};

static const reconfig_subsequence_t reconfig_aop_ddr_to_s2r_aop_a0_pre_seqs[] = {
   { (uint32_t *) aop_ddr_to_s2r_aop_a0_pre_0, sizeof(aop_ddr_to_s2r_aop_a0_pre_0) / sizeof(uint32_t) },
   { NULL, 0 },
};

static const reconfig_subsequence_t reconfig_s2r_aop_to_aop_ddr_a0_post_seqs[] = {
   { (uint32_t *) s2r_aop_to_aop_ddr_a0_post_0, sizeof(s2r_aop_to_aop_ddr_a0_post_0) / sizeof(uint32_t) },
   { (uint32_t *) s2r_aop_to_aop_ddr_a0_post_1, sizeof(s2r_aop_to_aop_ddr_a0_post_1) / sizeof(uint32_t) },
   { (uint32_t *) s2r_aop_to_aop_ddr_a0_post_2, sizeof(s2r_aop_to_aop_ddr_a0_post_2) / sizeof(uint32_t) },
   { (uint32_t *) s2r_aop_to_aop_ddr_a0_post_3, sizeof(s2r_aop_to_aop_ddr_a0_post_3) / sizeof(uint32_t) },
   { NULL, 0 },
};

static const reconfig_subsequence_t reconfig_s2r_aop_to_aop_ddr_a0_pre_seqs[] = {
   { (uint32_t *) s2r_aop_to_aop_ddr_a0_pre_0, sizeof(s2r_aop_to_aop_ddr_a0_pre_0) / sizeof(uint32_t) },
   { NULL, 0 },
};

static const reconfig_subsequence_t reconfig_aop_ddr_to_awake_a0_post_seqs[] = {
   { (uint32_t *) aop_ddr_to_awake_a0_post_0, sizeof(aop_ddr_to_awake_a0_post_0) / sizeof(uint32_t) },
   { (uint32_t *) aop_ddr_to_awake_a0_post_1, sizeof(aop_ddr_to_awake_a0_post_1) / sizeof(uint32_t) },
   { (uint32_t *) aop_ddr_to_awake_a0_post_2, sizeof(aop_ddr_to_awake_a0_post_2) / sizeof(uint32_t) },
   { NULL, 0 },
};

static const reconfig_subsequence_t reconfig_awake_to_aop_ddr_a0_post_seqs[] = {
   { (uint32_t *) awake_to_aop_ddr_a0_post_0, sizeof(awake_to_aop_ddr_a0_post_0) / sizeof(uint32_t) },
   { (uint32_t *) awake_to_aop_ddr_a0_post_1, sizeof(awake_to_aop_ddr_a0_post_1) / sizeof(uint32_t) },
   { NULL, 0 },
};

#endif // AOP_CONFIG_SEQUENCES_H
