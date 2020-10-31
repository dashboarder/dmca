
/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/anc_boot.h>
#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>
#include "anc_bootrom.h"
#include "anc_llb.h"
#include "anc_bootrom_private.h"
#include "util_boot.h"

anc_ppn_device_params_t *anc_geom;

#define NUM_DIP     256UL
#define NUM_BUS     2UL

dip_info_t  dip_info[NUM_DIP];
int anc_num_channel; // number of populated channels

static bool anc_llb_set_normal_mode(void);
static bool anc_llb_set_feature(anc_t *anc, uint16_t feature, const uint8_t *data, uint32_t len);
static bool anc_llb_get_geometry(void);
static bool anc_llb_read_chip_ids(void);
static bool anc_llb_read_chip_id(anc_t *anc, anc_chip_id_t *chip_id);
static bool anc_llb_read_device_parameters(anc_t                   *anc,
                                           anc_ppn_device_params_t *params);
static void anc_llb_handle_geb(anc_t *anc);

int anc_llb_init(void)
{
    unsigned int dip;
    unsigned int die;
    unsigned int plane;
    unsigned int bus;
    unsigned int cau_per_die;
    unsigned int numDip;
    unsigned int numPlane;
    unsigned int die_per_channel;

    anc_geom = memalign(NUM_BUS * sizeof(anc_ppn_device_params_t), CPU_CACHELINE_SIZE);
    if (!anc_geom) panic("Unable to allocate anc_geom");

    // Initialize ANC, but don't reset the NANDs - they're already good to go
    // in low-power mode coming out of SecureROM.
    if (anc_bootrom_init(false, ANC_BOOT_MODE_RESET_ALL_CONTROLLERS) != 0)
    {
        dprintf(DEBUG_CRITICAL, "Failed to init ANC");
        return -1;
    }

    if (!anc_llb_read_chip_ids())
    {
        dprintf(DEBUG_CRITICAL, "Failed to read chip IDs");
        return -1;
    }

    if (!anc_llb_set_normal_mode())
    {
        dprintf(DEBUG_CRITICAL, "Failed to enter normal mode");
        return -1;
    }

    if (!anc_llb_get_geometry())
    {
        dprintf(DEBUG_CRITICAL, "Failed to get geometry");
        return -1;
    }

    if (anc_geom[0].caus_per_channel > (NUM_DIP / NUM_BUS))
    {
        dprintf(DEBUG_CRITICAL, "caus_per_channel out of range: %d max %ld\n", anc_geom[0].caus_per_channel, NUM_DIP / NUM_BUS);
        return -1;
    }

    die_per_channel = anc_geom[0].dies_per_channel;
    cau_per_die     = (anc_geom[0].caus_per_channel / die_per_channel);
    dip             = 0;
    die             = 0;
    bus             = 0;
    plane           = 0;

    // PAIRING_SCHEME_TWO_BY_TWO_PAIRING
    if (4 == cau_per_die && PPN_BLOCK_PAIRING__FOUR_PLANE == anc_geom[0].block_pairing_scheme) {
        numDip   = (anc_geom[0].caus_per_channel * anc_num_channel) / 2;
        numPlane = cau_per_die / 2;
    } else { // PAIRING_SCHEME_ODD_EVEN_PAIRING, PAIRING_SCHEME_NO_PAIRING
        numDip   = anc_geom[0].caus_per_channel * anc_num_channel;
        numPlane = cau_per_die;
    }

    while (dip < numDip) {
        dip_info[dip].cau = plane + (die * cau_per_die);
        dip_info[dip].bus = bus;
        dip++;

        plane++;

        if (plane >= numPlane) {
            plane = 0;
            bus++;

            if ((int) bus >= anc_num_channel) {
                bus = 0;
                die++;
            }
        }
    }

    if (4 == cau_per_die && PPN_BLOCK_PAIRING__FOUR_PLANE == anc_geom[0].block_pairing_scheme) {
        die      = 0;
        bus      = 0;
        plane    = 2;
        numDip   = anc_geom[0].caus_per_channel * anc_num_channel;
        numPlane = cau_per_die;

        while (dip < numDip) {
            dip_info[dip].cau = plane + (die * cau_per_die);
            dip_info[dip].bus = bus;
            dip++;

            plane++;

            if (plane >= numPlane) {
                plane = 2;
                bus++;

                if ((int) bus >= anc_num_channel) {
                    bus = 0;
                    die++;
                }
            }
        }
    }

    return 0;
}

uint32_t anc_get_dip(uint32_t bus, uint32_t cau) {
    uint32_t dip;
    uint32_t num_dip;

    num_dip = anc_geom[0].caus_per_channel * anc_num_channel;

    for (dip = 0; dip < num_dip; dip++) {
        if (cau == dip_info[dip].cau && bus == dip_info[dip].bus) {
            break;
        }
    }

    return dip;
}

uint32_t anc_get_dies_per_channel() {
    return anc_geom[0].dies_per_channel;
}

static bool anc_llb_set_normal_mode(void)
{
    int   channel;

    for (channel = 0; channel < anc_num_channel; channel++)
    {
        anc_t   *anc         = &g_boot_anc[channel];
        uint32_t power_state = PPN_FEATURE__POWER_STATE__NORMAL_ASYNC;

        if (!anc_llb_set_feature(anc,
                                 PPN_FEATURE__POWER_STATE,
                                 (const uint8_t *)&power_state,
                                 sizeof(uint32_t)))
        {
            return false;
        }
        anc_wr(anc, R_ANC_LINK_SDR_DATA_TIMING, 0x04040404);
        anc_wr(anc, R_ANC_LINK_SDR_TIMING, 0x00000F0F);
        anc_wr(anc, R_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING, 0x00000302);
        anc_wr(anc, R_ANC_LINK_READ_STATUS_CONFIG, 0x00F04040);
    }

    return true;
}

static bool anc_llb_get_geometry(void)
{
    int  channel;

    for (channel = 0; channel < anc_num_channel; channel++)
    {
        anc_t  *anc = &g_boot_anc[channel];

        if (!anc_llb_read_device_parameters(anc, &anc_geom[channel]))
        {
            return false;
        }
    }
    return true;
}

static bool anc_llb_read_chip_ids(void)
{
    int channel;

    anc_num_channel=0;
    for (channel = 0; channel < ANC_BOOT_CONTROLLERS; channel++)
    {
        anc_t *anc = &g_boot_anc[channel];

        if (!anc_llb_read_chip_id(anc, &anc->chip_id))
        {
            return false;
        }
        else if (anc->chip_id[0]=='P' && anc->chip_id[1]=='P' && anc->chip_id[2]=='N')
        {
            anc_num_channel++;
        }
    }
    return true;
}

static bool anc_llb_read_chip_id(anc_t *anc, anc_chip_id_t *chip_id)
{
    bool     ret = false;
    const uint32_t int_mask = (M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG |
                               M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT);
    uint32_t int_status;

    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ_ID));
    anc_boot_put_link_command(anc, LINK_COMMAND__ADDR1(CHIPID_ADDR));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_PIO(ANC_NAND_ID_SIZE, false, false));
    anc_boot_put_link_command(anc, LINK_COMMAND__SEND_INTERRUPT(0, 0));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(0));

    int_status = anc_boot_wait_interrupt(anc, int_mask);
    if (!int_status || (int_status & M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT))
    {
        dprintf(DEBUG_CRITICAL, "Timeout waiting for CHAN_INT_STATUS - 0x%08x\n", int_status);
        return false;
    }

    if (G_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG(int_status))
    {
        uint32_t id_word;
        uint8_t *ptr = (uint8_t *)chip_id;

        // Chip ID is now available in Link PIO Read FIFO
        id_word = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
        ptr[0] = (id_word >> 0)  & 0xFF;
        ptr[1] = (id_word >> 8)  & 0xFF;
        ptr[2] = (id_word >> 16) & 0xFF;
        ptr[3] = (id_word >> 24) & 0xFF;
        id_word = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
        ptr[4] = (id_word >> 0) & 0xFF;
        ptr[5] = (id_word >> 8) & 0xFF;

        ret = true;
    }

    return ret;
}

static bool anc_llb_set_feature(anc_t         *anc,
                                uint16_t       feature,
                                const uint8_t *data,
                                uint32_t       len)
{
    bool     ret;
    uint32_t i;
    const uint32_t int_mask = (M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG |
                               M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE |
                               M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT);
    uint32_t int_status;

    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__SET_FEATURES));
    anc_boot_put_link_command(anc, LINK_COMMAND__ADDR2(feature & 0xFF,
                                                       (feature & 0xFF00) >> 8));
    anc_boot_put_link_command(anc, LINK_COMMAND__WRITE_PIO(len, false, false));
    for (i = 0; i < len / sizeof(uint32_t); i++)
    {
        anc_boot_put_link_command(anc, data[i]);
    }

    anc_boot_put_link_command(anc, LINK_COMMAND__CMD3(NAND_CMD__SET_GET_FEATURES_CONFIRM,
                                                      NAND_CMD__GET_NEXT_OPERATION_STATUS,
                                                      NAND_CMD__OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_STATUS(0, 0x40, 0x40));
    anc_boot_put_link_command(anc, LINK_COMMAND__SEND_INTERRUPT(0, 0));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__GET_NEXT_OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(0));

    int_status = anc_boot_wait_interrupt(anc, int_mask);
    if (!int_status || (int_status & M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT))
    {
        panic("Timeout waiting for CHAN_INT_STATUS - 0x%08x\n", int_status);
        return false;
    }

    if (G_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG(int_status))
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

static bool anc_llb_read_device_parameters(anc_t                   *anc,
                                           anc_ppn_device_params_t *params)
{
    const uint32_t intmask = (M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG |
                              M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT |
                              M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT |
                              M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE);
    uint32_t intstatus;
    bool     ret;
    uint64_t paddr = mem_static_map_physical((uintptr_t)params);

    anc_boot_put_dma_command(anc, DMA_COMMAND_CONFIG(DMA_DIRECTION_N2M, false));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ_DEVICE_PARAMETERS));
    anc_boot_put_link_command(anc, LINK_COMMAND__ADDR1(0x00));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD3(NAND_CMD__READ_DEVICE_PARAMETERS_CONFIRM,
                                                      NAND_CMD__GET_NEXT_OPERATION_STATUS,
                                                      NAND_CMD__OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_STATUS(0, 0x40, 0x40));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ_SERIAL_OUTPUT));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_DMA(sizeof(anc_ppn_device_params_t), false, false));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__GET_NEXT_OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(0));

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN, params, sizeof(anc_ppn_device_params_t));
#endif

    anc_boot_put_dma_command(anc, DMA_COMMAND_BUFDESC(sizeof(anc_ppn_device_params_t),
                                                      paddr));

    anc_boot_put_dma_command(anc, DMA_COMMAND_FLAG(0, 0));

    intstatus = anc_boot_wait_interrupt(anc, intmask);
    if (!intstatus || (intstatus & M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT))
    {
        panic("Timeout waiting for CHAN_INT_STATUS - 0x%08x\n", intstatus);
        return false;
    }

    if (intstatus & M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT)
    {
        dprintf(DEBUG_CRITICAL, "Timeout waiting for DMA to complete");
        ret = false;
    }
    else if (intstatus & M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE)
    {
        dprintf(DEBUG_INFO, "Unexpected NAND status\n");
        ret = false;
    }
    else
    {
        ret = true;
    }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, params, sizeof(anc_ppn_device_params_t));
#endif

    return ret;
}

static uint32_t make_physical_page(uint32_t dip, uint32_t band, uint16_t page)
{
    const unsigned int block_bits = anc_geom[0].block_bits;
    const unsigned int page_bits  = anc_geom[0].page_address_bits;
    const unsigned int cau_bits   = anc_geom[0].cau_bits;
    unsigned int mode;

    if((dip_info[dip].cau == 0) && (band == 0)) {
        mode = 0;
    } else {
        if (anc_geom[0].address_bits_bits_per_cell > 1) {
            // Device supports high-endurance SLC
            mode = 3;
        } else {
            // Device only supports regular SLC/MLC modes. Treat this high-endurance
            // work as regular SLC
            mode = 1;
        }
    }

    // LLB will always use SLC (but the PPN spec requires we not set the mode bit on CAU 0, block 0)
    return (page |
            (band << page_bits) |
            (dip_info[dip].cau  << (page_bits + block_bits)) |
            (mode << (page_bits + block_bits + cau_bits)));
}


uint32_t anc_llb_read_phys_page(uint32_t  band, // returns # of valid pages
                            uint32_t  dip,
                            uint32_t  page,
                            uint32_t  num_lbas,
                            void     *data,
                            uint32_t *meta)
{
    anc_t         *anc = &g_boot_anc[dip_info[dip].bus];
    const uint32_t intmask = (M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG |
                              M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT |
                              M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE);
    uint32_t     intstatus;
    unsigned int lba;
    addr_t       paddr;
    uint32_t    ret = num_lbas;
    uint32_t     ppage = make_physical_page(dip, band, page);
    bool         encrypted = false;
    uint8_t      nand_status;

    paddr = mem_static_map_physical((addr_t)data);
    if (!paddr)
    {
        dprintf(DEBUG_INFO, "No buffer\n");
        return 0;
    }
    if (!meta)
    {
        dprintf(DEBUG_INFO, "No meta buffer\n");
        return 0;
    }
    if (num_lbas == 0)
    {
        dprintf(DEBUG_INFO, "Invalid num_lbas %d\n", num_lbas);
        return 0;
    }

    if ((platform_get_chip_id() == 0x8960) && (platform_get_chip_revision() == 0))
    {
        // On Alcatraz A0, we encrypt everything but dip 0, block 0 (due to rdar://problem/11247422)
        if ((dip == 0) && (band == 0))
        {
            encrypted = false;
        }
        else
        {
            encrypted = true;
        }
    }
    else
    {
        encrypted = false;
    }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, data, num_lbas * NAND_BOOT_LOGICAL_PAGE_SIZE);
#endif

    anc_boot_put_dma_command(anc, DMA_COMMAND_CONFIG(DMA_DIRECTION_N2M, encrypted));
    if (encrypted)
    {
        anc_boot_put_dma_command(anc, DMA_COMMAND_AES_KEY_IV(DMA_COMMAND_AESKEY_256_BITS, false));
        // Key of all zeroes
        anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
        anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
        anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
        anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
        // IV of all zeroes
        anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
        anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
    }

    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__MULTIPAGE_READ_LAST));

    if (anc_geom[dip_info[dip].bus].bytes_per_row_address != 4)
    {
        anc_boot_put_link_command(anc, LINK_COMMAND__OPCODE(LINK_CMD_OP_ADDR3_OP) | (page & 0xFFFFFF));
    }
    else
    {
        anc_boot_put_link_command(anc, LINK_COMMAND__OPCODE(LINK_CMD_OP_ADDR4) | (ppage & 0xFFFFFF));
        anc_boot_put_link_command(anc, (ppage & 0xFF000000) >> 24);
    }


    anc_boot_put_link_command(anc, LINK_COMMAND__CMD3(NAND_CMD__MULTIPAGE_READ_CONFIRM,
                                                      NAND_CMD__GET_NEXT_OPERATION_STATUS,
                                                      NAND_CMD__OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_STATUS(0x00,
                                                             0x50,
                                                             0x40));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_REGISTER(R_ANC_LINK_NAND_STATUS - A_ANC_LINK_BASE));

    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ_SERIAL_OUTPUT));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));
    for (lba = 0; lba < num_lbas; lba++)
    {
        anc_boot_put_link_command(anc, LINK_COMMAND__READ_PIO(NAND_BOOT_BYTES_PER_META, false, false));
        if (lba == (num_lbas - 1))
        {
            anc_boot_put_link_command(anc, LINK_COMMAND__SEND_INTERRUPT(0, 0));
        }
        anc_boot_put_link_command(anc, LINK_COMMAND__READ_DMA(NAND_BOOT_LOGICAL_PAGE_SIZE, false, false));
        if (encrypted && (lba != 0))
        {
            anc_boot_put_dma_command(anc, DMA_COMMAND__OPCODE(DMA_COMMAND__OPCODE__AES_IV));
            anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
            anc_boot_put_dma_command(anc, 0x0000000000000000ULL);
        }
        anc_boot_put_dma_command(anc, DMA_COMMAND_BUFDESC(NAND_BOOT_LOGICAL_PAGE_SIZE,
                                                          (uint64_t)paddr + lba * NAND_BOOT_LOGICAL_PAGE_SIZE));
    }
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(32));

    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__GET_NEXT_OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(0));

    anc_boot_put_dma_command(anc, DMA_COMMAND_FLAG(0, 0));

    intstatus = anc_boot_wait_interrupt(anc, intmask);
    if (!intstatus)
    {
        dprintf(DEBUG_CRITICAL, "Timeout waiting for CHAN_INT_STATUS != 0\n");
        return 0;
    }

    // We've either timed out or gotten our status by now
    if (intstatus & M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT)
    {
        // Timeout waiting for status: flush LINK CmdQ and return error
        dprintf(DEBUG_CRITICAL, "Timeout waiting for NAND status\n");
        anc_wr(anc, R_ANC_LINK_CONTROL, M_ANC_LINK_CONTROL_RESET_CMDQ);
        return 0;
    }
    else if (intstatus & M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE)
    {
        uint8_t status = anc_rd(anc, R_ANC_LINK_NAND_STATUS);
        // Invalid status (probably has error bit set due to a clean or uncorrectable page)
        if (BOOT_LBA_TOKEN_UNKNOWN != meta[0]) // scanning blocks - may be clean or uecc
        {
            dprintf(DEBUG_CRITICAL, "Invalid NAND status 0x%02X\n", status);
        }
        anc_wr(anc, R_ANC_LINK_CONTROL, M_ANC_LINK_CONTROL_START);
        ret = 0;
        meta[0] = BOOT_ERR_BLANK;
    }

    nand_status = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
    if (nand_status != 0x40)
    {
        if (BOOT_LBA_TOKEN_UNKNOWN != meta[0]) // scanning blocks - may be clean or uecc
        {
            dprintf(DEBUG_CRITICAL, "Invalid NAND status: 0x%02X\n", nand_status);
        }
        ret = 0;
        meta[0] = BOOT_ERR_BLANK;
    }

    for (lba = 0; lba < num_lbas; lba++)
    {
        uint32_t meta_index;
        for (meta_index = 0; meta_index < (NAND_BOOT_BYTES_PER_META / sizeof(uint32_t)); meta_index++)
        {
            uint32_t meta_word = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
            if ((meta_index == 0) && (meta_word != meta[0]))
            {
                if (BOOT_LBA_TOKEN_UNKNOWN == meta[0]) 
                {
                    meta[0] = meta_word;
                } 
                else if (BOOT_ERR_BLANK != meta[0])
                {
                    if (lba==0) {
                        dprintf(DEBUG_CRITICAL, "Invalid meta: expected 0x%08X got 0x%08x\n", meta[0], meta_word);
                    }
                    ret = lba;
                }
            }
        }
    }

    intstatus = anc_boot_wait_interrupt(anc, M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG | M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT);
    if (!intstatus || (intstatus & M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT))
    {
        dprintf(DEBUG_CRITICAL, "Timeout waiting for DMA to complete - 0x%08x\n", intstatus);
        ret = 0;
    }

    anc_wr(anc, R_ANC_CHAN_INT_STATUS, M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG);

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, data, num_lbas * NAND_BOOT_LOGICAL_PAGE_SIZE);
#endif

    return ret;
}

typedef struct
{
    uint16_t failure_type;
    uint32_t start_page;
    uint8_t  page_len[3];
    uint8_t  checksum;
} anc_ppn_failure_info_t;


static void anc_llb_handle_geb(anc_t *anc)
{
    uint32_t  buffer[3];
    dprintf(DEBUG_CRITICAL, "General Error Dected on Bus %d, attempting to pull failure info", anc->bus_id);

    anc_wr(anc, R_ANC_CHAN_INT_ENABLE, 0);
    anc_wr(anc, R_ANC_DMA_CONTROL, V_ANC_DMA_CONTROL_RESET(1) | V_ANC_DMA_CONTROL_START(1));
    anc_wr(anc, R_ANC_LINK_CONTROL, (V_ANC_LINK_CONTROL_RESET_CMDQ(1) |
                                     V_ANC_LINK_CONTROL_RESET_PIO_READ(1) |
                                     V_ANC_LINK_CONTROL_START(1)));

    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__GET_DEBUG_DATA));
    anc_boot_put_link_command(anc, LINK_COMMAND__ADDR3(0xFF, 0xFF, 0xFF));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__SET_GET_FEATURES_CONFIRM));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(129000));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__OPERATION_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_STATUS(0, 0x40, 0x40));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ_SERIAL_OUTPUT));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_PIO(10, false, false));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(0));

    while (G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL(anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS)) < 3)
    {
        ;
    }

    buffer[0] = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
    buffer[1] = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
    buffer[2] = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);

    panic("GEB detected failure type 0x%04X  start page 0x%08X",
	((anc_ppn_failure_info_t *)buffer)->failure_type, ((anc_ppn_failure_info_t *)buffer)->start_page);
}


