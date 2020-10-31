/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _ANC_BOOTROM_PRIVATE_H_
#define _ANC_BOOTROM_PRIVATE_H_

#if SUB_PLATFORM_S7002
#    include "anc_bootrom_regs_m7.h"
#else
#    include "anc_bootrom_regs.h"
#endif
#include "ppn_npl_regs.h"
#include "anc_bootrom_cmds.h"

#define ANC_MAX_CES_PER_BUS  (1)
#define ANC_NAND_ID_SIZE     (6)

#define NAND_BOOT_PAGES_PER_BLOCK   128
#define NAND_BOOT_BYTES_PER_META    16
#define NAND_BOOT_BYTES_PER_PAGE    8192
#define NAND_BOOT_LOGICAL_PAGE_SIZE 4096

// NAND Commands
#define NAND_CMD__RESET          ((uint8_t)0xFF)
#define NAND_CMD__READ_ID        ((uint8_t)0x90)
#define NAND_CMD__READ_STATUS    ((uint8_t)0x70)
#define NAND_CMD__READ           ((uint8_t)0x00)
#define NAND_CMD__READ_CONFIRM   ((uint8_t)0x30)

#define NAND_CMD__MULTIPAGE_READ                 ((uint8_t)0x07)
#define NAND_CMD__MULTIPAGE_READ_LAST            ((uint8_t)0x0A)
#define NAND_CMD__MULTIPAGE_READ_CONFIRM         ((uint8_t)0x37)
#define NAND_CMD__MULTIPAGE_PROGRAM              ((uint8_t)0x87)
#define NAND_CMD__MULTIPAGE_PROGRAM_LAST         ((uint8_t)0x8A)
#define NAND_CMD__MULTIPAGE_PROGRAM_CONFIRM      ((uint8_t)0x17)
#define NAND_CMD__MULTIPAGE_PREP                 ((uint8_t)0xB0)
#define NAND_CMD__MULTIPAGE_PREP__CONFIRM        ((uint8_t)0xB8)
#define NAND_CMD__MULTIBLOCK_ERASE               ((uint8_t)0x67)
#define NAND_CMD__MULTIBLOCK_ERASE_LAST          ((uint8_t)0x6A)
#define NAND_CMD__MULTIBLOCK_ERASE_CONFIRM       ((uint8_t)0xD7)
#define NAND_CMD__LOW_POWER_READ_STATUS          ((uint8_t)0x70)
#define NAND_CMD__GET_NEXT_OPERATION_STATUS      ((uint8_t)0x77)
#define NAND_CMD__OPERATION_STATUS               ((uint8_t)0x7D)
#define NAND_CMD__READ_SERIAL_OUTPUT             ((uint8_t)0x7A)
#define NAND_CMD__CONTROLLER_STATUS              ((uint8_t)0x79)
#define NAND_CMD__READ_DEVICE_PARAMETERS         ((uint8_t)0x92)
#define NAND_CMD__READ_DEVICE_PARAMETERS_CONFIRM ((uint8_t)0x97)
#define NAND_CMD__SET_FEATURES                   ((uint8_t)0xEF)
#define NAND_CMD__GET_FEATURES                   ((uint8_t)0xEE)
#define NAND_CMD__SET_GET_FEATURES_CONFIRM       ((uint8_t)0xE7)
#define NAND_CMD__SET_DEBUG_DATA                 ((uint8_t)0xE9)
#define NAND_CMD__GET_DEBUG_DATA                 ((uint8_t)0xE8)
#define NAND_CMD__UPDATE_FW                      ((uint8_t)0xED)
#define NAND_CMD__PPN_STATUS_CONFIRM             ((uint8_t)0xED)

#define PPN_LOW_POWER_STATUS__FAIL              (1 << 0)
#define PPN_LOW_POWER_STATUS__READY             (1 << 6)
#define PPN_LOW_POWER_STATUS__WRITE_PROTECT_BAR (1 << 7)

#define PPN_FEATURE__POWER_STATE                      ((uint16_t)0x0180)

#define PPN_FEATURE__POWER_STATE__LOW_POWER       0x1
#define PPN_FEATURE__POWER_STATE__NORMAL_ASYNC    0x2
#define PPN_FEATURE__POWER_STATE__NORMAL_DDR      0xA
#define PPN_FEATURE__POWER_STATE__STANDBY         0x4

#define CHIPID_ADDR      (0x00)

#define PPN_BLOCK_PAIRING__NO_PAIRING (0x00)
#define PPN_BLOCK_PAIRING__EVEN_ODD   (0x10)
#define PPN_BLOCK_PAIRING__FOUR_PLANE (0x20)

#define ROUNDUPTO(num, gran) ((((num) + (gran) - 1) / (gran)) * (gran))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

#define ANC_BOOT_MAX_TIMING_REG_VALUE    (M_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME >> S_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME)

typedef uint8_t anc_chip_id_t[ANC_NAND_ID_SIZE];

typedef struct anc_t
{
    uint32_t    bus_id;
    bool        initialized;
    uint32_t   *regs;

    uint32_t    pages_per_block;
    uint32_t    bytes_per_meta;
    uint32_t    meta_words;
    uint32_t    bytes_per_page;
    uint32_t    logical_page_size;
    uint32_t    lbas_per_page;

    anc_chip_id_t chip_id;

} anc_t;

extern anc_t g_boot_anc[ANC_BOOT_CONTROLLERS];

#define anc_wr(anc, reg, val)  (*((anc)->regs + ((reg) >> 2)) = (val))
#define anc_wr64(anc, reg, val) anc_store64((volatile uint64_t *)((anc)->regs + ((reg) >> 2)), (val))
#define anc_rd(anc, reg)   (*(volatile uint32_t *)((uint8_t *)((anc)->regs) + (reg)))
#define anc_npl_wr(npl, reg, val)  (*(npl + ((reg) >> 2)) = (val))

void anc_boot_put_link_command(anc_t *anc, uint32_t value);
void anc_boot_put_dma_command(anc_t *anc, uint64_t value);
uint32_t anc_boot_wait_interrupt(anc_t *anc, uint32_t mask);
bool anc_boot_wait_reg(anc_t *anc, uint32_t reg, uint32_t mask, uint32_t value);
#if !APPLICATION_SECUREROM
bool anc_boot_wait_reg_int_timeout(anc_t *anc, uint32_t reg, uint32_t mask, uint32_t value, uint32_t tomask);
#endif // !APPLICATION_SECUREROM

#endif // _ANC_BOOTROM_PRIVATE_H_
