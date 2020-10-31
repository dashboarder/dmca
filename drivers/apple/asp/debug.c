/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#define ASP_TARGET_RTXC 1
#define CASSERT(x, name) typedef char __ASP_CASSERT_##name[(x) ? 1 : -1]
#include <debug.h>
#include <sys/menu.h>
#include <lib/env.h>
#include <platform/memmap.h>
#include <platform/soc/hwclocks.h>
#include <stdint.h>
#include <assert.h>
#include <csi_ipc_protocol.h>
#include <arch.h>
#include <aspcore_mbox.h>
#include <aspcore_protocol.h>
#include <aspcore_nand_debug.h>
#include <debug.h>
#include <drivers/asp.h>
#include <drivers/csi.h>
#include <lib/env.h>
#include <lib/heap.h>
#include <lib/partition.h>
#include <libDER/DER_Keys.h>
#include <libDER/DER_Decode.h>
#include <libDER/asn1Types.h>
#include <platform.h>
#include <stdio.h>
#include <sys.h>
#include <platform/timer.h> // ticks
#include "common_util.h"
#include <drivers/usb/usb_debug.h>

#if DEBUG_BUILD && WITH_MENU

#define METADATA_PER_LBA     (16)
#define LBA_TOKEN_SIZE       (4)
#define TOKEN_TERMINATOR     (0xFFFFFFFF)
#define LBA_TOKEN_BASE       (0x0FF00000)
#define LBA_TOKEN_BLOG       (LBA_TOKEN_BASE + 0x13)
#define BYTES_PER_SEC        (4096)
#define BLOG_NUM_ENTRY       (BYTES_PER_SEC / sizeof(BlogE_t))
#define MAX_RMA_CHUNK_PAGES  (1024)
#define PERFECT_PAGE_SIZE    (4112)
#define VENDOR_CODE__MICRON  (0x2C)
#define VENDOR_CODE__HYNIX   (0xAD)
#define VENDOR_CODE__SANDISK (0x45)
#define SIZE_OF_WFALL_TBL_ENTRY (4)

#define TEST_DATA_PATTERN (0xA5)
#define WATERMARK_PATTERN (0xFF)

#define FLOW_DEAD 12
#define FLOW_UTIL 13
extern aspproto_nand_geom_t nand_info;
extern asp_t asp;
extern struct blockdev *asp_nand_dev;

typedef enum {
    NOB_TOKEN_UNMAPPED  = 0,
    NOB_TOKEN_W_UNC     = 1,
    NOB_TOKEN_W_UNC_LOG = 2,
    NOB_TOKEN_GC_UNC    = 3,
    NOB_TOKEN_GC_BLANK  = 4,
    NOB_TOKEN_TRANSLATE = 5,
    NUM_NOB_TOKENS      = 6,
    NOB_TOKEN_MAX       = 7
} NOB_TOKEN_e;

#define NOB_BAND_BITS 12
#define NOB_BOFF_BITS (32 - NOB_BAND_BITS)
typedef union {
    struct {
        uint32_t boff : NOB_BOFF_BITS;
        uint32_t band : NOB_BAND_BITS;
    };
    uint32_t    all;              // note that the all field is used to acclerate math on the boff field which is assumed to be the least significant bits without risk of roll over.
    NOB_TOKEN_e token;
} nob_t;

typedef struct {
    uint32_t lba;
    uint32_t size;
} BlogE_t;

typedef enum {
    BONFIRE_SLC_STATE_NONE = 0,
    BONFIRE_SLC_STATE_A    = 1,
    BONFIRE_SLC_STATE_B    = 2
} BONFIRE_SLC_STATE_e;

BONFIRE_SLC_STATE_e bonfire_slc_state = BONFIRE_SLC_STATE_NONE;

typedef enum {
    ASP_USER_PARTITION,
    ASP_INTERMEDIATE_PARTITION,
    NUM_ASP_PARITIONS
} asp_partitions_e;

char partition_names[NUM_ASP_PARITIONS][30] = {
    "USER PARTITION",
    "INTERMEDIATE PARTITION"
};

#define ROUNDUPTO(num, gran)        ((((num) + (gran) - 1) / (gran)) * (gran))

static void usage(void)
{
    printf("usage:\n");
    printf("asp sync\n");
#if defined(ASP_ENABLE_NEURALIZE) && ASP_ENABLE_NEURALIZE
    printf("asp neuralize\n");
#endif
    printf("asp readid\n");
    printf("asp info\n");
    printf("asp debug_counter_supported <channel>\n");
    printf("asp get_debug_counter <channel>\n");
    printf("asp reset_debug_counter <channel>\n");
    printf("asp dies_in_parallel <total dies>\n");
    printf("asp dies_in_parallel  (for MLC device: <slc_write> <read> <erase> <mlc_write>) (for TLC device: <slc_write> <read> <erase> <tlc_write>)\n");
    printf("asp test_scratchpad\n");
    printf("asp bonfirereadband <band> <mode(%d for MLC, %d for SLC, %d for TLC)> <display stats (0/1)\n", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp readband <band> <mode(%d for MLC, %d for SLC, %d for TLC)> < no. of stripes to be issued during 1 read transaction> <Optional: time in seconds to repeatedly read. if 0, then read only once>\n", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp readrandom <mode(%d for MLC, %d for SLC, %d for TLC, %d for mixed)> <Num of reads (each read is 4k/8k/16k)> <no. of stripes to be issued during 1 read transaction> <num sectors_per_page> <if reads in a stripe should be on same_die (1 for same die, 2 for diff die, 3 for random)> <Optional: time in seconds to repeatedly read. if 0, then read only once>\n", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC, CELL_TYPE_IS_MIXED);
    printf("asp readmixedsuperpage <Num stripes> < no. of stripes to be issued during 1 read transaction> <Optional: time in seconds to repeatedly read. if 0, then read only once>\n");
    printf("asp secperband <band>\n");
    printf("asp v2p <vba>\n");
    printf("asp getburnincode\n");
    printf("asp cbp2r <cau> <blk> <page>\n");
    printf("asp dbp2r <dip> <bork> <page>\n");
    printf("asp r2cbp <row_address>\n");
    printf("asp l2dbp <lba>\n");
    printf("asp getlastfailure\n");
    printf("asp set_photoflow_slc\n");
    printf("asp set_photoflow_mlc\n");
    printf("asp enable_bg\n");
    printf("asp disable_bg\n");
    printf("asp bbt\n");
    printf("asp dm\n");
    printf("asp dipinfo\n");
    printf("asp read <bus> <ce> <row address> <sector_offset> <no. of %d byte sectors> <0/1/2 for bit flips or in-depth health monitoring> <mode(%d for MLC, %d for SLC, %d for TLC)>\n", ASP_NAND_BLKSZ, CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp readpagemeta <bus> <ce> <row address> <no. of pages>\n");
    printf("asp readbandmeta <band> <verbose (0/1)\n");
    printf("asp bandstat\n");
    printf("asp disableuid\n");
    printf("asp dumpblog <band> <audit (0/1)> <verbose (0/1)>\n");
    printf("asp vthsweep <channel> <cau> <block> <file_path>\n");
    printf("asp rma_configure <channel> <mask>\n");
    printf("asp rma_set <channel>\n");
    printf("asp rma_get <channel> [file_path]\n");
    printf("asp rma_delete <channel>\n");
    printf("asp ppn_recover\n");
    printf("asp ppn_get_calibration <file_path>\n");
    printf("asp waterfall_size\n");
    printf("asp waterfall\n");
    printf("asp devparam\n");
    printf("asp testbdevread\n");
    printf("asp setTLCwritestripes <number of stripes that can be programmed together>\n");
    printf("asp readverify <band>\n");
    printf("asp istlc\n");
    printf("asp getlinkclkfreq\n");
    printf("asp resetperfticks\n");
    printf("asp getperfticks\n");
    printf("asp printslcbonfirebands\n");
#if ASP_ENABLE_WRITES
    printf("asp setwritable\n");
    printf("asp utilFormat\n");
    printf("asp lbaFormat\n");
    printf("asp register\n");
    printf("asp bonfireeraseband <band>\n");
    printf("asp bonfirewriteband <band> <mode(%d for MLC, %d for SLC, %d for TLC)> <Optional: 4 byte data pattern. otherwise random>\n", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp setburnincode <code>\n");
    printf("asp eraseband <band> <mode(%d for MLC, %d for SLC, %d for TLC)>\n", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp writeband <band> <mode(%d for MLC, %d for SLC, %d for TLC)> <Optional: 4 byte data pattern. otherwise random>\n", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp eploop <band> <mode(%d for MLC, %d for SLC, %d for TLC) <time in seconds to repeatedly Erase+Prog. if 0, then only once> <Optional: 4 byte data pattern. otherwise random>", CELL_TYPE_IS_MLC, CELL_TYPE_IS_SLC, CELL_TYPE_IS_TLC);
    printf("asp testwaterfall\n");
    printf("asp testbdevwrite\n");
    printf("asp groupaslc <seed>\n");
    printf("asp groupbslc\n");
    printf("asp ungroup\n");
#endif //ASP_ENABLE_WRITES
}

static int asp_nand_info(void)
{
    printf("ce_per_bus = %d\n", nand_info.ce_per_bus);
    printf("cau_per_die = %d\n", nand_info.cau_per_die);
    printf("num_bus = %d\n", nand_info.num_bus);
    printf("die_per_bus = %d\n", nand_info.die_per_bus);
    printf("num_dip = %d\n", nand_info.num_dip);
    printf("num_bands = %d\n", nand_info.num_bands);
    printf("sec_per_page = %d\n", nand_info.sec_per_page);
    printf("sec_per_full_band = %d\n", nand_info.sec_per_full_band);
    printf("sec_per_full_band_slc = %d\n", nand_info.sec_per_full_band_slc);
    printf("bytes_per_sec_meta = %d\n", nand_info.bytes_per_sec_meta);
    printf("pages_per_block = %d\n", nand_info.pages_per_block);
    printf("pages_per_block_slc = %d\n", nand_info.pages_per_block_slc);
    printf("cell type = %d. (%d for MLC, %d for TLC)\n", nand_info.cell_type, CELL_TYPE_IS_MLC, CELL_TYPE_IS_TLC);
    printf("lastUserBand = %d\n", asp.lastUserBand);
    printf("lastBand = %d\n", asp.lastBand);
    printf("numVirtualSLCBonfireBands = %d\n", asp.numVirtualSLCBonfireBands);
    printf("firstIntermediateBand = %d\n", asp.firstIntermediateBand);
    printf("lastIntermediateBand = %d\n", asp.lastIntermediateBand);

    return 0;
}

static void asp_print_status_key() {

    printf("\nType key: \n0:Success, \n3:UECC/Efail/Pfail, \n5:Refresh, \n6: clean, \n8:Unknown\n\n");
}

static bool asp_format(uint8_t type)
{

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    if(!asp.writable)
    {
        printf("Need to make system writable. Execute 'asp setwritable' first\n");
        return false;
    }

    if ((asp.state != ASP_STATE_INITIALIZED)
        && (asp.state != ASP_STATE_READY)
        && (asp.state != ASP_STATE_ERROR_NOT_REGISTERED))
    {
        dprintf(DEBUG_CRITICAL, "ASP is not initialized/ready! Current state is %d\n", asp.state);
    	return false;
    }

    if ((type == ASP_FORMAT_UTIL)
        || (type == ASP_FORMAT_ALL))
    {
        command->op  = ASPPROTO_CMD_UTIL_FORMAT;
    }

    else
    {
        command->op = ASPPROTO_CMD_LBA_FORMAT;
    }

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        // NOTE: If format fails, we maintain the last formatted
        // status from asp_get_geomtry()
        dprintf(DEBUG_CRITICAL, "ASP Format Failed\n");
        return false;
    }

    if (command->op == ASPPROTO_CMD_UTIL_FORMAT)
    {
        asp.util_formatted = true;
    }

    asp.lba_formatted = true;

    return true;

}

static void output_nand_die_uid(uint8_t *buffer)
{
    uint32_t die_idx, bus_idx, byteIdx;

    for(bus_idx = 0; bus_idx < nand_info.num_bus; bus_idx++)
    {
        for(die_idx = 0; die_idx < nand_info.die_per_bus; die_idx++)
        {
            printf("CE %d: ", bus_idx);
            for(byteIdx = 0; byteIdx < ASPPROTO_DEV_ID_LEN_PPN; byteIdx++)
            {
                printf("%02X ", buffer[(ASPPROTO_DEV_ID_LEN_PPN*(die_idx + bus_idx * nand_info.die_per_bus)) + byteIdx]);
            }
            printf("\n");
        }
    }
}

static void output_nand_uid (uint8_t *buffer, uint32_t size, uint32_t parameter)
{
    uint32_t Idx, byteIdx;

    for(Idx = 0; Idx < size/ASPPROTO_DEV_ID_LEN_PPN; Idx++)
    {
        printf("CE %d: ", Idx);
        for(byteIdx = 0; byteIdx < ASPPROTO_DEV_ID_LEN_PPN; byteIdx++)
        {
            if(parameter == ASPPROTO_CMD_FW_VERSION)
            {
                printf("%c", buffer[(ASPPROTO_DEV_ID_LEN_PPN*Idx) + byteIdx]);
            }
            else
            {
                printf("%02X ", buffer[(ASPPROTO_DEV_ID_LEN_PPN*Idx) + byteIdx]);
            }
        }
        printf("\n");
    }
}

static int asp_nand_get_uid()
{
    aspproto_cmd_t *    command;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_PKG_ASSEMBLY_CODE;
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get package assembly code: %d\n", status);
        return -1;
    }
    printf("\nPackage Assembly Code:\n");
    output_nand_uid(command->device_id.id, command->device_id.size, ASPPROTO_CMD_PKG_ASSEMBLY_CODE);

    command->op = ASPPROTO_CMD_CONTROLLER_UID;
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get controller uid: %d\n", status);
        return -1;
    }
    printf("\nController UID:\n");
    output_nand_uid(command->device_id.id, command->device_id.size, ASPPROTO_CMD_CONTROLLER_UID);

    command->op = ASPPROTO_CMD_CONTROLLER_HWID;
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get controller hwid: %d\n", status);
        return -1;
    }
    printf("\nController HWID:\n");
    output_nand_uid(command->device_id.id, command->device_id.size, ASPPROTO_CMD_CONTROLLER_HWID);

    command->op = ASPPROTO_CMD_FW_VERSION;
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get fw version: %d\n", status);
        return -1;
    }
    printf("\nFW Version:\n");
    output_nand_uid(command->device_id.id, command->device_id.size, ASPPROTO_CMD_FW_VERSION);

    command->op = ASPPROTO_CMD_DIE_UID;
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get die uid: %d\n", status);
        return -1;
    }
    printf("\nDie UID:\n");
    output_nand_die_uid(command->device_id.id);

    return 0;
}

static uint32_t asp_get_link_clk_freq(bool verbose, uint64_t *freq_ptr) {
    aspproto_cmd_t * command        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    uint8_t          *bounce_buffer;
    uint32_t         paddr32;

    bounce_buffer = memalign(sizeof(uint64_t), ASP_NAND_BLK_ALIGNMENT);
    if(!bounce_buffer)
    {
        printf("unable to allocate bounce buffer\n");
        return -1;
    }

    paddr32 = VADDR_TO_PADDR32(bounce_buffer);
    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_GET_LINK_CLK_FREQ;
    command->tunnel.buffer_paddr    = paddr32;
    command->tunnel.bufferLen       = sizeof(uint64_t);

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, bounce_buffer, CPU_CACHELINE_SIZE);
#endif

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to get link clk freq\n");
        return -1;
    } else {

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_CLEAN, bounce_buffer, CPU_CACHELINE_SIZE);
#endif

        if(verbose)
        {
            printf("link clk freq: %lld\n", *(uint64_t *)bounce_buffer);
        }
    }

    if(freq_ptr)
    {
        *freq_ptr = *(uint64_t *)bounce_buffer;
    }
    return 0;
}

static uint32_t asp_get_perf_ticks(bool verbose, uint64_t *ticks_ptr, uint32_t channel) {
    aspproto_cmd_t * command        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    uint8_t          *bounce_buffer;
    uint32_t         paddr32;

    bounce_buffer = memalign(sizeof(uint64_t), ASP_NAND_BLK_ALIGNMENT);
    if(!bounce_buffer)
    {
        printf("unable to allocate bounce buffer\n");
        return -1;
    }

    paddr32 = VADDR_TO_PADDR32(bounce_buffer);
    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_GET_PERF_TICKS;
    command->tunnel.buffer_paddr    = paddr32;
    command->tunnel.bufferLen       = sizeof(uint64_t);
    command->tunnel.options.mask    = 1 << channel;

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, bounce_buffer, CPU_CACHELINE_SIZE);
#endif

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to get perf ticks\n");
        return -1;
    } else {

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_CLEAN, bounce_buffer, CPU_CACHELINE_SIZE);
#endif

        if(verbose)
        {
            printf("For channel %d, ticks: %lld\n", channel, *(uint64_t *)bounce_buffer);
        }
        if(verbose && (0 == (*(uint64_t *)bounce_buffer)))
        {
            printf("Got 0 ticks. Is ANC_ENABLE_TUNNEL_PERF_COUNTER turned ON?\n");
        }
    }

    if(ticks_ptr)
    {
        *ticks_ptr = *(uint64_t *)bounce_buffer;
    }
    return 0;
}


static int asp_get_bonfire_get_phy_band_num(uint32_t virtual_band) {

    aspproto_cmd_t * command        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode     = CORE_DEBUG_BONFIRE_GET_PHY_BAND_NUM;
    command->tunnel.options.value   = virtual_band;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to get actual band number\n");
        return -1;
    }
    return command->tunnel.options.value;
}


static uint32_t asp_print_slc_bonfire_bands() {
    uint32_t i;

    for(i = 0; i < asp.numVirtualSLCBonfireBands; i++) {
        printf("virtual band: %d -> actual band: %d\n", i, asp_get_bonfire_get_phy_band_num(i));
    }
    return 0;
}


static uint32_t asp_reset_perf_ticks() {

    aspproto_cmd_t * command        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_RESET_PERF_TICKS;
    command->tunnel.options.mask    = (1 << 0) | (1 << 1);

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to reset perf ticks\n");
        return -1;
    }
    return 0;
}

static int asp_cbp2r(uint32_t cau, uint32_t block, uint32_t page)
{
    aspproto_cmd_t *    command;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_CBP_TO_ROW_ADDR;
    command->addr_xlate.cau = cau;
    command->addr_xlate.block = block;
    command->addr_xlate.page = page;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get row address: %d\n", status);
        return -1;
    }
    printf("cau: %d, blk: %d, page: %d --> row_addr: 0x%X\n", cau, block, page, command->addr_xlate.row_addr);
    return 0;
}

static int asp_l2dbp(uint32_t lba)
{
    nand_debug_lba_map_t *lbaMap;
    uint32_t paddr32;
    uint32_t lbaMapSize;
    aspproto_cmd_t * command;
    int status;

    lbaMap     = NULL;
    command    = NULL;
    lbaMapSize = sizeof(nand_debug_lba_map_t);

    command    = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    if(!command)
    {
        return -1;
    }

    lbaMap = memalign(lbaMapSize, ASP_NAND_BLK_ALIGNMENT);
    if(!lbaMap)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate data_buffer\n");
        return -1;
    }
    memset(lbaMap, 0x00, lbaMapSize);
    paddr32                        = VADDR_TO_PADDR32(lbaMap);
    lbaMap->lba                    = lba;

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, lbaMap, ROUNDUPTO(lbaMapSize,CPU_CACHELINE_SIZE));
#endif

    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_LBA_TO_PADDR;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = lbaMapSize;

    status = asp_send_command(command->tag);

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_INVALIDATE, lbaMap, ROUNDUPTO(lbaMapSize,CPU_CACHELINE_SIZE));
#endif
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get Physical Address for lba: %d\n", lbaMap->lba);
        return -1;
    }

    printf("lba: 0x%x --> band: 0x%x dip: %d bork: 0x%x page: 0x%x\n", lbaMap->lba, lbaMap->band, lbaMap->dip, lbaMap->bork, lbaMap->page);
    return 0;
}

static int asp_r2cbp(uint32_t row_addr)
{
    aspproto_cmd_t *    command;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_ROW_ADDR_TO_CBP;
    command->addr_xlate.row_addr = row_addr;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get row address: %d\n", status);
        return -1;
    }
    printf("row_addr: 0x%X --> cau: %d, block: %d, page: %d\n", row_addr, command->addr_xlate.cau, command->addr_xlate.block, command->addr_xlate.page);
    return 0;
}

static int asp_set_burnin_code(uint32_t burnin_code)
{
    aspproto_cmd_t *    command;
    int status;

    if(!asp.writable)
    {
        printf("Need to make system writable. Execute 'asp setwritable' first\n");
        return -1;
    }

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_SET_BURNIN_CODE;
    command->lba = burnin_code;

    status = asp_send_command(command->tag);
    return 0;
}

static int asp_set_tlcwritestripes(uint32_t write_stripes)
{
    aspproto_cmd_t *    command;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_SETOPTIONS;
    command->opts.TLCwriteStripes = write_stripes;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to set tlc writes stripes\n");
        return -1;
    }
    return 0;
}

static int asp_get_burnin_code()
{
    aspproto_cmd_t *    command;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_GET_BURNIN_CODE;

    status = asp_send_command(command->tag);
    printf("Burnin Code: %d. Status: 0x%X\n", command->lba, status);
    return 0;
}

static int asp_get_last_failure()
{
    aspproto_cmd_t *    command;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_GET_LAST_FAILURE;

    status = asp_send_command(command->tag);
    printf("Type: %d CE: %d, page: 0x%X\n\n", command->last_failure.failure_mode, command->last_failure.ce, command->last_failure.physical_page);
    return 0;
}

static int asp_sectors_per_band(uint32_t band, bool verbose)
{
    aspproto_cmd_t *    command;
    int status;

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_SECTORS_PER_BAND;
    command->lba = band;

    status = asp_send_command(command->tag);
    if(verbose)
    {
        printf("Band: %d has %d sectors.\n", band, command->count);
    }
    return (int)command->count;
}

static int asp_nand_get_addr(uint32_t vba, uint32_t *bus, uint32_t *row_addr, bool verbose)
{
    aspproto_cmd_t *    command;
    int status;

    if(vba < nand_info.sec_per_full_band)
    {
        printf("Cannot decode vba in band 0: should be >= %d\n", nand_info.sec_per_full_band);
        return -1;
    }

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_GET_ADDR;
    command->addr.vba = vba;

    status = asp_send_command(command->tag);
    if(verbose)
    {
        printf("vba: %d -> ce: %d, row_addr: 0x%X, cau: %d, column: %d\n", vba, command->addr.ce, command->addr.row_addr, command->addr.cau, command->addr.column);
    }

    if(bus)
    {
        *bus = command->addr.ce;
    }
    if(row_addr)
    {
        *row_addr = command->addr.row_addr;
    }
    return 0;
}

static uint32_t get_num_pages_for_sectors(uint32_t sector_offset, uint32_t num_sectors)
{
    uint32_t last_sector = (sector_offset + num_sectors);
    uint32_t leading_sectors = 0;
    uint32_t trailing_sectors = 0;
    uint32_t intermediate_sectors = 0;

    if(last_sector >= nand_info.sec_per_page)
    {
        leading_sectors = sector_offset ? (nand_info.sec_per_page - sector_offset) : 0;
    }
    else
    {
        leading_sectors = sector_offset ? (last_sector - sector_offset) : 0;
    }
    if((!sector_offset) || (last_sector > nand_info.sec_per_page))
    {
        trailing_sectors = last_sector % nand_info.sec_per_page;
    }
    intermediate_sectors = num_sectors - (leading_sectors + trailing_sectors);
    assert(!(intermediate_sectors % nand_info.sec_per_page));

    return (leading_sectors ? 1 : 0) + (trailing_sectors ? 1 : 0) + (intermediate_sectors / nand_info.sec_per_page);
}

/*
    Description:
    Does a physical NAND read without encryption.
    Reads are done one physical page at a time.
    Output data is arranged as first all data followed by all metadata.
    Metadata will be available only at a full page boundary - even for partial page reads.
    For partial page reads, output data is arranged as follows:
    Example - For a 16k page with a read of 2 4k sectors starting from offset 1,
              data will be available as the first 8k, meta will be available at a 16k offset.

*/
static int asp_physical_read(uint32_t bus,
                             uint32_t ce,
                             uint32_t row_addr,
                             uint32_t sector_offset,
                             uint32_t num_sectors,
                             nand_debug_health_monitoring_e health_monitoring,
                             aspproto_cell_type_t cell_type,
                             uint8_t * buffer,
                             bool verbose,
                             int *status_ptr)
{
    aspproto_cmd_t *command = 0;
    int             status = 0;
    uint8_t        *bounce_buffer = NULL;
    uint32_t        paddr32 = 0;
    const uint32_t  main_size = ASP_NAND_BLKSZ * nand_info.sec_per_page;
    const uint32_t  meta_size = nand_info.sec_per_page * METADATA_PER_LBA;
    const uint32_t  num_pages = get_num_pages_for_sectors(sector_offset, num_sectors);
    const uint32_t  kB_sectors_per_page = main_size / 1024;
    uint32_t        bytes_per_read = main_size + meta_size;
    uint32_t        sectors_remaining = num_sectors;
    uint32_t        offset_cursor = sector_offset;
    uint32_t        sectors_for_current_page;
    uint8_t        *data_cursor = buffer;
    uint8_t        *meta_cursor = buffer + (num_pages * main_size);
    uint32_t        current_row_addr = row_addr;
    uint8_t        *buf_stats_cursor = NULL;
    uint32_t        idx = 0;
    uint8_t        *value_ptr;
    uint64_t        perf_ticks;
    uint64_t        link_clk_freq;

    if(0 != asp_reset_perf_ticks())
    {
        return -1;
    }

    if((sector_offset > (nand_info.sec_per_page - 1)))
    {
        printf("sector offset allowed to be only between 0 and %d\n", nand_info.sec_per_page - 1);
        return -1;
    }
    if((NAND_DEBUG_HEALTH_MONITORING_NONE != health_monitoring) && ((sector_offset != 0) || (num_sectors % nand_info.sec_per_page)))
    {
        printf("health monitoring reads are allowed only for full pages\n");
        return -1;
    }
    if(NAND_DEBUG_HEALTH_MONITORING_COUNT <= health_monitoring)
    {
        printf("invalid health monitoring option\n");
        return -1;
    }

    if(NAND_DEBUG_HEALTH_MONITORING_NONE != health_monitoring)
    {
        if (!security_allow_memory(buffer, num_pages * kB_sectors_per_page))
        {
            printf("Permission Denied\n");
            return -1;
        }
        memset(buffer, 0xA5, num_pages * kB_sectors_per_page);
        bytes_per_read = kB_sectors_per_page;
    }
    else
    {
        if (!security_allow_memory(buffer, num_pages * bytes_per_read))
        {
            printf("Permission Denied\n");
            return -1;
        }
        memset(buffer, 0xA5, num_pages * bytes_per_read);
    }

    bounce_buffer = memalign(bytes_per_read, 4096);
    if(!bounce_buffer)
    {
        dprintf(DEBUG_CRITICAL, "Failed to allocate data buffer\n");
        return -1;
    }
    paddr32 = VADDR_TO_PADDR32(bounce_buffer);

    while(sectors_remaining)
    {
        if (NAND_DEBUG_HEALTH_MONITORING_IN_DEPTH == health_monitoring) {
            command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
            command->op                   = ASPPROTO_CMD_NAND_DEBUG;
            command->tunnel.opcode        = NAND_DEBUG_ENABLE_IN_DEPTH_HEALTH;
            command->tunnel.buffer_paddr  = 0;
            command->tunnel.bufferLen     = 0;
            command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << bus);

            if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
                dprintf(DEBUG_CRITICAL, "Error enabling depth health monitoring for channel %d rowAddr: 0x%x\n", bus, current_row_addr);
            }
        }

        if((offset_cursor % nand_info.sec_per_page) && ((sector_offset + sectors_remaining) > nand_info.sec_per_page))
        {
            sectors_for_current_page = (offset_cursor % nand_info.sec_per_page) ? (nand_info.sec_per_page - sector_offset) : 0;
        }
        else
        {
            sectors_for_current_page = ((sectors_remaining < nand_info.sec_per_page) ? sectors_remaining : nand_info.sec_per_page);
        }
        memset(bounce_buffer, 0xC7, bytes_per_read);

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, bounce_buffer, ROUNDUPTO(bytes_per_read,CPU_CACHELINE_SIZE));
#endif


        command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
        command->op                                               = ASPPROTO_CMD_NAND_DEBUG;
        command->tunnel.opcode                                    = NAND_DEBUG_READ;
        command->tunnel.buffer_paddr                              = paddr32;
        command->tunnel.bufferLen                                 = bytes_per_read;
        command->tunnel.options.debug_epr_info.bus                = bus;
        command->tunnel.options.debug_epr_info.ce                 = ce;
        command->tunnel.options.debug_epr_info.row_addr           = current_row_addr;
        command->tunnel.options.debug_epr_info.num_sectors        = sectors_for_current_page;
        command->tunnel.options.debug_epr_info.sector_offset      = offset_cursor % nand_info.sec_per_page;
        command->tunnel.options.debug_epr_info.health_monitoring  = health_monitoring;
        command->tunnel.options.debug_epr_info.cell_type          = cell_type;
        command->tunnel.options.mask                              = 1 << bus;
        command->flags.all                                        = 0;
        command->flags.noAesKey                                   = 1;

        status = asp_send_command(command->tag);

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_INVALIDATE, bounce_buffer, ROUNDUPTO(bytes_per_read,CPU_CACHELINE_SIZE));
#endif
        if(verbose)
        {
            printf("Bus: %01d, ce: %02d, row_addr: 0x%08X, num_sectors: %02d, sector_offset: %02d. Status: %02d\t", bus, ce, current_row_addr, sectors_for_current_page, (offset_cursor % nand_info.sec_per_page), status);
        }

        if(status_ptr)
        {
            *status_ptr = status;
        }

        if(NAND_DEBUG_HEALTH_MONITORING_SECTOR_STATS == health_monitoring)
        {
            // display sector stats
            printf("Bitflips: ");
            buf_stats_cursor = bounce_buffer;
            for(idx = 0; idx < kB_sectors_per_page; idx++)
            {
                printf("%02X ", *buf_stats_cursor);
                buf_stats_cursor++;
            }

            memcpy(data_cursor, bounce_buffer, kB_sectors_per_page);
            data_cursor += kB_sectors_per_page;

        }
        else if (NAND_DEBUG_HEALTH_MONITORING_IN_DEPTH == health_monitoring)
        {
            command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
            command->op                   = ASPPROTO_CMD_NAND_DEBUG;
            command->tunnel.opcode        = NAND_DEBUG_GET_IN_DEPTH_HEALTH;
            command->tunnel.buffer_paddr  = 0;
            command->tunnel.bufferLen     = 0;
            command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << bus);
            command->tunnel.options.value = 0;

            if ((asp_send_command(command->tag) == ASPPROTO_CMD_STATUS_SUCCESS)) {
                value_ptr = (uint8_t*) &(command->tunnel.options.value);
                printf("Correctability: 0x%x tR-Status: 0x%x",value_ptr[0], value_ptr[1]);
            } else {
                dprintf(DEBUG_CRITICAL, "Error pulling in depth health monitoring for channel %d rowAddr: 0x%x\n", bus, current_row_addr);
            }
        }
        else
        {
            memcpy(data_cursor, bounce_buffer, main_size);
            memcpy(meta_cursor, (bounce_buffer + main_size), meta_size);
            data_cursor += main_size;
            meta_cursor += meta_size;
        }
        if(verbose)
        {
            printf("\n");
        }

        sectors_remaining -= sectors_for_current_page;
        offset_cursor += sectors_for_current_page;
        current_row_addr++;
    }

    if(verbose)
    {
        asp_print_status_key();
    }

    if(0 != asp_get_perf_ticks(false, &perf_ticks, bus))
    {
        return -1;
    }

    if(0 != asp_get_link_clk_freq(false, &link_clk_freq))
    {
        return -1;
    }

    printf("%u bytes transferred in %llu ticks @ %llu Hz\n\n", (num_sectors * ASP_NAND_BLKSZ),
                                                                perf_ticks,
                                                                link_clk_freq);

    printf("To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (num_sectors * ASP_NAND_BLKSZ),
                                                                                              link_clk_freq,
                                                                                              perf_ticks);

    printf("Total time taken in us: (%llu) / (%llu / 1000000)\n\n", perf_ticks, link_clk_freq);

    printf("Note: This is actual bus utilization with very minimal SW overhead. This command uses only 1 channel at a time\n");


    free(bounce_buffer);
    return 0;
}

static int asp_read_pagemeta(uint32_t bus, uint32_t ce, uint32_t row_addr, uint32_t num_pages, uint8_t * buffer)
{
    uint32_t page_idx = 0;
    const uint32_t  main_size = ASP_NAND_BLKSZ * nand_info.sec_per_page;
    const uint32_t  meta_size = nand_info.sec_per_page * METADATA_PER_LBA;
    uint8_t         *meta_cursor = buffer + (num_pages * main_size);

    asp_physical_read(bus, ce, row_addr, 0, (num_pages * nand_info.sec_per_page), false, CELL_TYPE_IS_MLC, buffer, false, NULL);

    printf("LBA tokens:\n");
    for(page_idx = row_addr; page_idx < row_addr + num_pages; page_idx++)
    {
        printf("Bus:%01d, ce:%02d, row_addr:0x%08X\t", bus, ce, page_idx);
        printf("Meta: %08X %08X %08X %08X\n", *((uint32_t*)meta_cursor), *((uint32_t*)meta_cursor + 4), *((uint32_t*)meta_cursor + 8), *((uint32_t*)meta_cursor + 12));
        meta_cursor += meta_size;
    }
    return 0;
}

static int asp_read_bandmeta(uint32_t band, uint8_t * buffer, bool verbose)
{
    uint32_t page_idx = 0;
    const uint32_t  main_size = ASP_NAND_BLKSZ * nand_info.sec_per_page;
    const uint32_t  meta_size = nand_info.sec_per_page * METADATA_PER_LBA;
    const uint32_t  sectors_per_band = asp_sectors_per_band(band, false);
    const uint32_t  pages_per_band = sectors_per_band / nand_info.sec_per_page;
    uint32_t        vba = band * nand_info.sec_per_full_band;
    uint8_t         *meta_cursor = buffer;
    uint32_t        row_addr = 0;
    uint32_t        bus = 0;
    uint8_t         *page_buffer = NULL;

    if (!security_allow_memory(buffer, meta_size * pages_per_band))
    {
        printf("Permission Denied\n");
        return -1;
    }
    memset(buffer, 0xA5, meta_size * pages_per_band);

    page_buffer = memalign((main_size + meta_size), 4096);
    if(!page_buffer)
    {
        printf("could not allocate page buffer\n");
        return -1;
    }

    if(verbose)
    {
        printf("LBA tokens:\n");
    }
    for(page_idx = 0; page_idx < pages_per_band; page_idx++)
    {
        asp_nand_get_addr (vba, &bus, &row_addr, false);
        asp_physical_read(bus, 0, row_addr, 0, nand_info.sec_per_page, false, CELL_TYPE_IS_MLC, page_buffer, false, NULL);
        memcpy(meta_cursor, page_buffer + main_size, meta_size);
        if(verbose)
        {
            printf("Bus:%01d, ce:%02d, row_addr:0x%08X\t", bus, 0, row_addr);
            printf("Meta: %08X %08X %08X %08X\n", *((uint32_t*)meta_cursor), *((uint32_t*)meta_cursor + (1 * LBA_TOKEN_SIZE)), *((uint32_t*)meta_cursor + (2 * LBA_TOKEN_SIZE)), *((uint32_t*)meta_cursor + (3 * LBA_TOKEN_SIZE)));
        }
        vba += nand_info.sec_per_page;
        meta_cursor += meta_size;
    }
    free(page_buffer);
    return 0;
}

static int audit_blog(uint32_t band, uint8_t *buffer, uint32_t num_leafs, bool verbose)
{
    const uint32_t sectors_per_band = asp_sectors_per_band(band, false);
    const uint32_t pages_per_band = sectors_per_band / nand_info.sec_per_page;
    BlogE_t        *blog_buffer = (BlogE_t *)buffer;
    uint32_t       current_lba_token;
    uint32_t       current_size;
    uint32_t       *lba_token_buffer = NULL;
    uint32_t       max_blog_entries = 0;
    uint32_t       blog_entry_idx = 0;
    uint32_t       *lba_token_cursor = NULL;
    uint32_t       size_idx;
    uint32_t       boff_idx = 0;
    int            status = 0;

    assert(buffer);

    printf("Auditing Blog entries against LBA's in NAND...\n");
    lba_token_buffer = (uint32_t *)malloc(nand_info.sec_per_page * METADATA_PER_LBA * pages_per_band);
    if(!lba_token_buffer)
    {
        printf("Unable to allocate memory for lba token buffer\n");
        return -1;
    }

    if(asp_read_bandmeta(band, (uint8_t *)lba_token_buffer, false))
    {
        free(lba_token_buffer);
        return -1;
    }

    lba_token_cursor = lba_token_buffer;

    max_blog_entries = (num_leafs * ASP_NAND_BLKSZ) / sizeof(BlogE_t);
    for(blog_entry_idx = 0; blog_entry_idx < max_blog_entries; blog_entry_idx++)
    {
        current_lba_token = blog_buffer[blog_entry_idx].lba;
        current_size = blog_buffer[blog_entry_idx].size;

        if((blog_entry_idx % (ASP_NAND_BLKSZ / sizeof(BlogE_t))) == ((ASP_NAND_BLKSZ / sizeof(BlogE_t)) - 1))
        {
            if(verbose)
            {
                printf("End of leaf.\n");
            }
            assert(current_lba_token == TOKEN_TERMINATOR);
            continue;
        }
        else if(current_lba_token == TOKEN_TERMINATOR)
        {
            if(verbose)
            {
                printf("End of blog.\n");
            }
            break;
        }

        assert(current_size <= sectors_per_band);
        for(size_idx = 0; size_idx < current_size; size_idx++)
        {
            if(verbose)
            {
                printf("Blog Entry idx: %04d, Blog Entry LBA: 0x%08X, LBA in NAND: 0x%08X. Boff: %04d\n", blog_entry_idx, current_lba_token, *lba_token_cursor, boff_idx);
            }
            if((current_lba_token >= LBA_TOKEN_BASE) && (current_lba_token != LBA_TOKEN_BLOG))
            {
                //Page pads are written slightly differently in NAND and Blog
                current_lba_token = LBA_TOKEN_BASE;
            }
            if(current_lba_token != *lba_token_cursor)
            {
                printf("Audit failed. Blog says LBA: 0x%08X. NAND says LBA: 0x%08X. boff: %d (vba: %d)\n", current_lba_token, *lba_token_cursor, boff_idx, ((band * nand_info.sec_per_full_band) + boff_idx));
                status = -1;
            }
            lba_token_cursor += LBA_TOKEN_SIZE;
            current_lba_token++;
            boff_idx++;
        }
    }
    if(status)
    {
        printf("Audit failed\n");
    }
    else
    {
        printf("Audit passed\n");
    }
    free(lba_token_buffer);

    return status;
}

//Dumps the blog and optionally audits if all LBA's in band correspond to blog entries.
static int asp_dump_blog(uint32_t band, uint8_t * buffer, bool audit, bool verbose)
{
    const uint32_t  main_size = ASP_NAND_BLKSZ * nand_info.sec_per_page;
    const uint32_t  meta_size = nand_info.sec_per_page * METADATA_PER_LBA;
    const uint32_t  sectors_per_band = asp_sectors_per_band(band, true);
    const uint32_t  max_blog_size = sectors_per_band * sizeof(BlogE_t);
    const uint32_t  max_possible_leafs = (max_blog_size / ASP_NAND_BLKSZ) + 1;
    uint32_t        *leaf_ptr;
    uint32_t        vba = band * nand_info.sec_per_full_band;
    uint32_t        blogdir_vba;
    uint32_t        row_addr = 0;
    uint32_t        bus = 0;
    uint32_t        lba_token;
    uint32_t        expected_lba_offset = 0;
    uint32_t        num_leafs = 0;
    uint32_t        status = 0;
    uint8_t         *page_buffer = NULL;
    bool            audit_allowed = true;
    int             lba_status = ASPPROTO_CMD_STATUS_SUCCESS;


    if (!security_allow_memory(buffer, max_blog_size))
    {
        printf("Permission Denied\n");
        return -1;
    }
    memset(buffer, 0xA5, max_blog_size);
    page_buffer = memalign((main_size + meta_size), 4096);
    if(!page_buffer)
    {
        printf("Could not allocate page buffer\n");
        return -1;
    }
    leaf_ptr = malloc(max_possible_leafs * sizeof(uint32_t));
    if(!leaf_ptr)
    {
        printf("Could not allocate leaf pointer array\n");
        return -1;
    }
    //Blog always starts at last sector of band.
    vba = vba + sectors_per_band - 1;
    blogdir_vba = vba;

    asp_nand_get_addr(vba , &bus, &row_addr, false);
    expected_lba_offset = vba % nand_info.sec_per_page;

    //Physical read of the Blog sector
    asp_physical_read(bus, 0, row_addr, expected_lba_offset, 1, false, CELL_TYPE_IS_MLC, page_buffer, false, &lba_status);
    if(lba_status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        printf("Received a non-success status: %d when reading bus: %d, ce: %d, row_addr: 0x%X, lba offset: %d\n", lba_status, bus, 0, row_addr, expected_lba_offset);
        asp_print_status_key();
        audit_allowed = false;
        goto FreeMemory;
    }

    lba_token = *((uint32_t*)(page_buffer + main_size));
    if(lba_token < LBA_TOKEN_BASE)
    {
        BlogE_t  *ptr = (BlogE_t  *)page_buffer;

        ptr[0].lba = lba_token - (sectors_per_band - 1);
        ptr[0].size = sectors_per_band;
        ptr[1].lba = TOKEN_TERMINATOR;
        ptr[1].size = 0;
        num_leafs = 1;
        printf("No blog. The entrire band was written sequentially. lba 0x%x size 0x%x\n ", ptr[0].lba, ptr[0].size);
        memcpy(buffer, page_buffer, ASP_NAND_BLKSZ);
    }
    else
    {
        if (LBA_TOKEN_BLOG == lba_token)
        {
            BlogE_t  *ptr = (BlogE_t  *)page_buffer;
            nob_t    *nobPtr;
            uint32_t i, j;

            ptr          += BLOG_NUM_ENTRY;
            nobPtr        = (nob_t*)ptr;
            nobPtr--;
            i             = nobPtr->all;
            nobPtr       -= i;
            printf("Number of leafs is %d\n",i);
            num_leafs = i;
            if(i == 1)
            {
                ptr -= BLOG_NUM_ENTRY;
                memcpy(buffer, page_buffer, ASP_NAND_BLKSZ);
                j = 0;
                while (( ptr[j].lba != TOKEN_TERMINATOR) && (ptr[j].size != 0))
                {
                    printf("lba 0x%x count 0x%x\n", ptr[j].lba,ptr[j].size);
                    j++;
                }
            }
            else
            {
                nob_t *dir = malloc(sizeof(nob_t) * i);
                uint32_t dir_no = i;

                if(dir)
                {
                    memcpy(dir, nobPtr, sizeof(nob_t) * dir_no);
                }
                else
                {
                    printf("cannot allocate memory . getting out\n");
                    goto FreeMemory;
                }
                for(j = 0; j < num_leafs; j ++)
                {
                    vba = (dir[j].band * nand_info.sec_per_full_band) + dir[j].boff;
                    asp_nand_get_addr(vba , &bus, &row_addr, false);
                    expected_lba_offset = vba % nand_info.sec_per_page;

                    //Physical read of the Blog sector
                    asp_physical_read(bus, 0, row_addr, expected_lba_offset, 1, false, CELL_TYPE_IS_MLC, page_buffer, false, &lba_status);
                    if(lba_status != ASPPROTO_CMD_STATUS_SUCCESS)
                    {
                        printf("Received a non-success status: %d when reading bus: %d, ce: %d, row_addr: 0x%X, lba offset: %d\n", lba_status, bus, 0, row_addr, expected_lba_offset);
                        asp_print_status_key();
                        audit_allowed = false;
                        free(dir);
                        goto FreeMemory;
                    }
                    memcpy(buffer + (j * ASP_NAND_BLKSZ), page_buffer, ASP_NAND_BLKSZ);
                    ptr = (BlogE_t  *)page_buffer;
                    while (( ptr[i].lba != TOKEN_TERMINATOR) && (ptr[i].size != 0))
                    {
                        printf("lba 0x%x count 0x%x\n", ptr[i].lba,ptr[i].size);
                        i++;
                    }
                }

                free(dir);
            }
        }
        else
        {
            goto FreeMemory;
        }
    }

    printf("%d bytes of Blog dumped to %p\n", (num_leafs * ASP_NAND_BLKSZ), buffer);

    if(audit && audit_allowed)
    {
        status = audit_blog(band, buffer, num_leafs, verbose);
    }

FreeMemory :

    free(page_buffer);
    free(leaf_ptr);
    return status;
}

static uint32_t asp_istlc(bool verbose) {

    aspproto_cmd_t * command        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_ISTLC;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to get if device is TLC\n");
        return -1;
    } else {
        if(verbose)
        {
            printf("istlc: %d\n", command->tunnel.options.value);
        }
        return command->tunnel.options.value;
    }
}

static uint32_t get_rma_chunk(uint32_t channel, uint32_t startPage, uint8_t *bounceBuffer, uint32_t buffLen) {
    uint32_t paddr32;

    paddr32 = VADDR_TO_PADDR32(bounceBuffer);
    memset(bounceBuffer, 0xA5, buffLen);

    aspproto_cmd_t * command        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_GET_RMA_DATA;
    command->tunnel.buffer_paddr    = paddr32;
    command->tunnel.bufferLen       = buffLen;
    command->tunnel.options.mask    = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);
    command->tunnel.options.value   = startPage;


#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, bounceBuffer, ROUNDUPTO(buffLen,CPU_CACHELINE_SIZE));
#endif
    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to pull rma data for channel %d\n",channel);
        return -1;
    }
#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, bounceBuffer, ROUNDUPTO(buffLen,CPU_CACHELINE_SIZE));
#endif

    return 0;
}

static uint32_t asp_recover() {
    aspproto_cmd_t * command;

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_RECOVER;
    command->tunnel.buffer_paddr  = 0;
    command->tunnel.bufferLen     = 0;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
        dprintf(DEBUG_CRITICAL, "Unable to recover NAND\n");
        return -1;
    }

    printf("NAND recovered\n");
    return 0;
}

static uint32_t asp_rma_delete(uint32_t channel) {
    aspproto_cmd_t * command;

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_DELETE_RMA_DATA;
    command->tunnel.buffer_paddr  = 0;
    command->tunnel.bufferLen     = 0;
    command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
        dprintf(DEBUG_CRITICAL, "Unable to delete rma data for channel %d\n",channel);
        return -1;
    }

    printf("RMA deleted from channel %d\n",channel);
    return 0;
}

static uint32_t asp_rma_configure(uint32_t channel, uint32_t mask) {
    aspproto_cmd_t * command;

    mask &= 0x0F;

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_CONFIGURE_RMA_DATA;
    command->tunnel.buffer_paddr  = 0;
    command->tunnel.bufferLen     = 0;
    command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);
    command->tunnel.options.value = mask;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
        dprintf(DEBUG_CRITICAL, "Unable to configure rma data for channel %d with mask 0x%x\n", channel, mask);
        return -1;
    }

    printf("RMA configured on channel %d with mask 0x%x\n",channel,mask);
    return 0;
}

static uint32_t asp_rma_set(uint32_t channel) {
    aspproto_cmd_t * command;

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_TRIGGER_RMA_DATA;
    command->tunnel.buffer_paddr  = 0;
    command->tunnel.bufferLen     = 0;
    command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
        dprintf(DEBUG_CRITICAL, "Unable to set RMA data for channel %d\n", channel);
        return -1;
    }

    printf("Set RMA data with address FFh FFh FFh on channel %d\n",channel);
    return 0;
}

static uint32_t asp_rma_get(uint32_t channel, char *destFile, uint32_t *failure_type) {
    aspproto_cmd_t * command;
    uint8_t  *bounceBuffer;
    uint32_t buffLen;
    uint32_t startPage;
    uint32_t totalPages;
    uint32_t pagesRemaining;
    uint32_t numPages;
    uint32_t chunkSize;
    uint32_t paddr32;
    uint32_t status;

    buffLen = sizeof(nand_debug_failure_info_t);
    bounceBuffer = memalign(buffLen, 4096);
    if(!bounceBuffer)
    {
        dprintf(DEBUG_CRITICAL, "Failed to allocate data buffer\n");
        return -1;
    }
    paddr32 = VADDR_TO_PADDR32(bounceBuffer);

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_GET_FAILURE_INFO;
    command->tunnel.buffer_paddr  = paddr32;
    command->tunnel.bufferLen     = buffLen;
    command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, bounceBuffer, ROUNDUPTO(buffLen,CPU_CACHELINE_SIZE));
#endif

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
        dprintf(DEBUG_CRITICAL, "Unable to trigger Debug Data for channel %d\n",channel);
        free(bounceBuffer);
        return -1;
    }
#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, bounceBuffer, ROUNDUPTO(buffLen,CPU_CACHELINE_SIZE));
#endif

    if (NULL != destFile) {
        startPage  = ((nand_debug_failure_info_t *)bounceBuffer)->start_page;
        totalPages = ((nand_debug_failure_info_t *)bounceBuffer)->page_len;
        if (NULL != failure_type) {
            *failure_type = ((nand_debug_failure_info_t *)bounceBuffer)->failure_type;
        }

        //pull the debug data pages
        buffLen = MAX_RMA_CHUNK_PAGES * PERFECT_PAGE_SIZE;
        free(bounceBuffer);
        bounceBuffer = memalign(buffLen, 4096);
        if(!bounceBuffer) {
            dprintf(DEBUG_CRITICAL, "Failed to allocate data buffer\n");
            return -1;
        }


        printf("\033[1;31m"); // change font color to red & bold
        for (pagesRemaining = totalPages; pagesRemaining > 0; pagesRemaining -= numPages) {
            printf("Status: %d%%, Pages: %d/%d ", (totalPages - pagesRemaining) * 100 / totalPages, totalPages - pagesRemaining, totalPages);

            numPages = MIN(pagesRemaining,MAX_RMA_CHUNK_PAGES);
            chunkSize = numPages * PERFECT_PAGE_SIZE;

            if (get_rma_chunk(channel, startPage, bounceBuffer, chunkSize) != 0) {
                break;
            }
            addr_t loadAddr = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
            memcpy( ((void*)loadAddr),bounceBuffer,chunkSize);

            status = usb_send_data_to_file(destFile, chunkSize, loadAddr, 0);
            if (status) {
                printf("usb_cmd_put failed ret = 0x%x\n", status);
                printf("\033[0m\n"); // change font color back to default
                free(bounceBuffer);
                return -1;
            }

            startPage += numPages;
            printf("\033[0E\033[2K"); // go to start of line and erase the line respectively
        }

        printf("Status: %d%%, Pages: %d/%d", (totalPages - pagesRemaining) * 100 / totalPages, totalPages - pagesRemaining, totalPages);
        printf("\033[0m\n"); // change font color back to default

    }

    asp_recover();
    free(bounceBuffer);

    return 0;
}

static uint32_t asp_ppn_get_calibration(char *destFile) {
    aspproto_cmd_t * command;
    uint8_t  *bounceBuffer;
    uint32_t buffLen;
    uint32_t paddr32;
    uint32_t status;

    buffLen = PERFECT_PAGE_SIZE;
    bounceBuffer = memalign(buffLen, 4096);
    if(!bounceBuffer)
    {
        dprintf(DEBUG_CRITICAL, "Failed to allocate data buffer\n");
        return -1;
    }
    paddr32 = VADDR_TO_PADDR32(bounceBuffer);

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_GET_CALIBRATION;
    command->tunnel.buffer_paddr  = paddr32;
    command->tunnel.bufferLen     = buffLen;
    command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << 0);

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, bounceBuffer, buffLen);
#endif
    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
        dprintf(DEBUG_CRITICAL, "Unable to get calibration data\n");
        free(bounceBuffer);
        return -1;
    }

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_INVALIDATE, bounceBuffer, buffLen);
#endif

    addr_t loadAddr = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    memcpy( ((void*)loadAddr),bounceBuffer,buffLen);

    if (NULL != destFile) {
            status = usb_send_data_to_file(destFile, buffLen, loadAddr, 0);
            if (status) {
                printf("usb_cmd_put failed ret = 0x%x\n", status);
                free(bounceBuffer);
                return -1;
            }
    } else {
        printf("PPN calibration data extraced to address %p. Save to host by running 'usb put <filename> %u'\n",
               (void *)loadAddr, buffLen);
    }

    free(bounceBuffer);

    return 0;
}

static uint32_t asp_vth_sweep(uint32_t channel, uint32_t cau, uint32_t block, char *destFile)
{
    aspproto_cmd_t * command;
    uint32_t rowAddr;
    uint32_t status;
    uint32_t failure_type;

    // bounds check the parameters
    if (channel >= nand_info.num_bus || cau >= (nand_info.cau_per_die * nand_info.die_per_bus) || block >= nand_info.num_bands) {
        printf("Invalid parameter. Max bus=%d \n", nand_info.num_bus);
        return -1;
    }

    // get manufID to check for supported vendor
    if ((asp.mfg_id[0][0] != VENDOR_CODE__HYNIX) && (asp.mfg_id[0][0] != VENDOR_CODE__SANDISK) && (asp.mfg_id[0][0] != VENDOR_CODE__MICRON)) {
        printf("Vendor 0x%x not supported!\n", asp.mfg_id[0][0]);
        return -1;
    }

    // get the row address
    command                   = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op               = ASPPROTO_CMD_CBP_TO_ROW_ADDR;
    command->addr_xlate.cau   = cau;
    command->addr_xlate.block = block;
    command->addr_xlate.page  = 0;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS) {
        dprintf(DEBUG_CRITICAL, "Failed to get row address: %d\n", status);
        return -1;
    }
    rowAddr = command->addr_xlate.row_addr;

    command                       = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                   = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode        = NAND_DEBUG_VTH_SWEEP;
    command->tunnel.buffer_paddr  = 0;
    command->tunnel.bufferLen     = 0;
    command->tunnel.options.mask  = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);
    command->tunnel.options.value = rowAddr;

     if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)) {
         dprintf(DEBUG_CRITICAL, "Unable to perform Vth sweep for channel %d\n",channel);
         return -1;
     }

    status = asp_rma_get(channel, destFile, &failure_type);
    if (0 != status) {
        printf("Vth sweep failed to get RMA data\n");
        return -1;
    }

    if (PPN_GEB_TYPE_VTHSWEEP != failure_type) {
        printf("Vth sweep expected failure type 0x%X, received 0x%X\n", PPN_GEB_TYPE_VTHSWEEP, failure_type);
        return -1;
    }

    return 0;
}

static uint32_t find_sectors_per_stripe (uint32_t sectors_per_band)
{
    const uint32_t sec_per_dip = nand_info.sec_per_full_band / nand_info.num_dip;

    return (sectors_per_band / sec_per_dip) * nand_info.sec_per_page;
}

#define DEBUG_LARGE_HEAP_HACK (0)

#if DEBUG_LARGE_HEAP_HACK
//***HACK: iBoot does not support such a large heap. 
//         However it has a large hole of free unmanaged memory below IBOOT_BASE (defined in memmap.h)
//         Define IBOOT_HACK_ADDR well below IBOOT_BASE and use it.
#define IBOOT_HACK_ADDR (0x8397FB000) //currently set to 64MB below IBOOT_BASE
#endif // #if DEBUG_LARGE_HEAP_HACK

static int asp_bonfire_write_band(uint32_t band, aspproto_cell_type_t cell_type, bool random_pattern, uint32_t data_pattern)
{
    uint8_t *dataBuf = NULL;
    uint32_t paddr32;
    aspproto_cmd_t *    command;
    int status;
    uint32_t sectors_per_band;
    uint32_t band_size;
    uint32_t i;

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    sectors_per_band = (uint32_t) asp_sectors_per_band(band, false);
    if(0 == sectors_per_band)
    {
        printf("Cannot program band - It has 0 good sectors.\n");
        return -1;
    }

    band_size = sectors_per_band * ASP_NAND_BLKSZ;

#if DEBUG_LARGE_HEAP_HACK
    dataBuf = (uint8_t *)IBOOT_HACK_ADDR;
#else
    // Provide a 4k buffer which will be written for the entire band.
    band_size = ASP_NAND_BLKSZ;
    dataBuf = memalign(band_size, ASP_NAND_BLK_ALIGNMENT);
#endif //#if DEBUG_LARGE_HEAP_HACK

    if(!dataBuf)
    {
        printf("could not allocate stats buffer\n");
        return -1;
    }

    assert((band_size % sizeof(data_pattern)) == 0);
    for(i = 0; i < band_size / sizeof(data_pattern); i++)
    {
        if(!random_pattern)
        {
            ((uint32_t *)dataBuf)[i] = data_pattern;
        }
        else
        {
            ((uint32_t *)dataBuf)[i] = rand();
        }
    }

    paddr32 = VADDR_TO_PADDR32(dataBuf);

    command = asp_get_cmd_for_tag(ASP_TAG_NAND);
    command->op                            = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode            = CORE_DEBUG_WRITE_BAND;
    command->tunnel.buffer_paddr           = (uint64_t) paddr32;
    command->tunnel.bufferLen              = band_size;
    command->tunnel.options.outputLen      = 0;
    command->tunnel.options.debug_epr_info.band = band;
    command->tunnel.options.debug_epr_info.cell_type = cell_type;
    command->flags.all = 0;
    command->flags.noAesKey = 1;
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not write band. Error: %d\n\n", status);
        asp_print_status_key();
        return -1;
    }

#if !DEBUG_LARGE_HEAP_HACK
    free(dataBuf);
#endif //#if DEBUG_LARGE_HEAP_HACK

    return 0;
}

static int asp_bonfire_erase_band(uint32_t band)
{
    aspproto_cmd_t *    command;
    int status;

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    if(!asp.writable)
    {
        printf("Need to make system writable. Execute 'asp setwritable' first\n");
        return -1;
    }

    command = asp_get_cmd_for_tag(ASP_TAG_NAND);

    command->op = ASPPROTO_CMD_BONFIRE_ERASE;
    command->lba = band;

    status = asp_send_command(command->tag);
    printf("Band: %d erase status: 0x%X. \n", band,  status);
    return 0;

}

static aspproto_nand_col_t asp_get_single_dip_info(uint32_t dip)
{
    aspproto_cmd_t *    command;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_NAND);

    command->op = ASPPROTO_CMD_GET_DIP_INFO;
    command->lba = dip;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        printf("Error getting info for dip %d. status: 0x%X\n", dip,  status);
    }
    return command->dip_info;
}

static int asp_get_all_dip_info(void)
{
    uint32_t i = 0;
    aspproto_nand_col_t dip_info;

    for(i = 0; i < nand_info.num_dip; i++)
    {
        dip_info = asp_get_single_dip_info(i);
        printf("dip: %d - bus: %d, ce: %d, cau: %d\n", i, dip_info.bus, dip_info.ce, dip_info.cau);
    }
    return 0;
}

static int asp_dbp2r(uint32_t dip, uint32_t bork, uint32_t page)
{
    aspproto_nand_col_t dip_info;
    aspproto_cmd_t *    command;
    int status;

    if(dip < nand_info.num_dip)
    {
        dip_info = asp_get_single_dip_info(dip);
    }
    else
    {
        dprintf(DEBUG_CRITICAL, "dip: %d exceeds maximum number of dips: %d\n", dip, nand_info.num_dip);
        return -1;
    }

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_CBP_TO_ROW_ADDR;
    command->addr_xlate.cau = dip_info.cau;
    command->addr_xlate.block = bork;
    command->addr_xlate.page = page;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get row address: %d\n", status);
        printf("ce: %d cau: %d, blk: %d, page: %d\n", dip_info.ce, dip_info.cau, bork, page);
        return -1;
    }
    printf("dip: %d, bork: %d, page: %d --> bus: %d row_addr: 0x%X\n",
           dip, bork, page, dip_info.bus, command->addr_xlate.row_addr);

    return 0;
}

static int asp_get_waterfall_table_size()
{
    uint32_t table_size = 0;
    aspproto_cmd_t *    command;

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode         = NAND_DEBUG_GET_WATERFALL_TBL_SIZE;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not get waterfall table size.\n\n");
        return -1;
    }

    table_size = command->tunnel.options.outputLen;
    assert(!(table_size % SIZE_OF_WFALL_TBL_ENTRY));
    printf("Waterfall table size: %d bytes\n", table_size);
    return table_size;
}

static int asp_erase_band(uint32_t band, aspproto_cell_type_t cell_type, bool verbose)
{
    aspproto_cmd_t *    command;
    int status;

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_ERASE_BAND;
    command->tunnel.options.debug_epr_info.band  = band;
    command->tunnel.options.debug_epr_info.cell_type = cell_type;

    status = asp_send_command(command->tag);
    if(verbose)
    {
        printf("Band: %d erase status: 0x%X. \n", band,  status);
    }

    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        asp_print_status_key();
        dprintf(DEBUG_CRITICAL, "ERROR: Could not erase band %d\n\n", band);
        return -1;
    }
    return 0;
}

static int asp_read_verify(uint32_t band)
{
    aspproto_cmd_t *    command;
    uint64_t            perf_ticks0;
    uint64_t            perf_ticks1;
    uint64_t            link_clk_freq;
    uint32_t            pages_verified = 0;

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    if(0 != asp_reset_perf_ticks())
    {
        return -1;
    }

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_READ_VERIFY_BAND;
    command->tunnel.options.debug_epr_info.band  = band;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not read verify band.\n\n");
        return -1;
    }
    pages_verified += command->tunnel.options.debug_epr_info.output_value;

    if(0 != asp_get_perf_ticks(false, &perf_ticks0, 0))
    {
        return -1;
    }
    if(0 != asp_get_perf_ticks(false, &perf_ticks1, 1))
    {
        return -1;
    }

    if(0 != asp_get_link_clk_freq(false, &link_clk_freq))
    {
        return -1;
    }

    printf("Ch0: %u bytes verified in %llu ticks @ %llu Hz\n\n", (pages_verified * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                  perf_ticks0,
                                                                  link_clk_freq);
    printf("Ch1: %u bytes verified in %llu ticks @ %llu Hz\n\n", (pages_verified * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                  perf_ticks1,
                                                                  link_clk_freq);

    printf("Ch0: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (pages_verified * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                   link_clk_freq,
                                                                                                   perf_ticks0);
    printf("Ch1: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (pages_verified * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                   link_clk_freq,
                                                                                                   perf_ticks1);

    printf("Note: This is actual bus utilization with negligible overhead.\n");

    return 0;
}


static int asp_get_timing_parameters(aspproto_nand_timing_params_t *buf)
{
    uint32_t                      paddr32;
    aspproto_nand_timing_params_t *timing_params = NULL;
    aspproto_cmd_t                *command;

    timing_params = memalign(sizeof(aspproto_nand_timing_params_t), ASP_NAND_BLK_ALIGNMENT);
    if(!timing_params)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate data_buffer\n");
        return -1;
    }

    memset(timing_params, 0x00, sizeof(aspproto_nand_timing_params_t));
    paddr32 = VADDR_TO_PADDR32(timing_params);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode         = NAND_DEBUG_GET_PPN_TIMING;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = sizeof(aspproto_nand_timing_params_t);
    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not get timing parameters.\n\n");
        free(timing_params);
        return -1;
    }

    memcpy(buf, timing_params, sizeof(aspproto_nand_timing_params_t));

    free(timing_params);
    return 0;
}

static int asp_get_device_parameters(aspproto_ppn_device_params_t *buf)
{
    uint32_t paddr32;
    aspproto_ppn_device_params_t *device_params = NULL;
    aspproto_cmd_t *    command;

    device_params = memalign(sizeof(aspproto_ppn_device_params_t), ASP_NAND_BLK_ALIGNMENT);
    if(!device_params)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate data_buffer\n");
        return -1;
    }
    memset(device_params, 0x00, sizeof(aspproto_ppn_device_params_t));
    paddr32 = VADDR_TO_PADDR32(device_params);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode         = NAND_DEBUG_GET_PPN_GEOM;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = sizeof(aspproto_ppn_device_params_t);

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not get waterfall table.\n\n");
        free(device_params);
        return -1;
    }

    memcpy(buf, device_params, sizeof(aspproto_ppn_device_params_t));

    free(device_params);
    return 0;
}


static int asp_print_ppn_parameters(void)
{
    aspproto_ppn_device_params_t  device_params;
    aspproto_nand_timing_params_t timing_params;
    int status;

    status  = asp_get_device_parameters(&device_params);
    status |= asp_get_timing_parameters(&timing_params);

    if(!status)
    {
        printf("Device parameters:\n");
        printf("caus_per_channel: %d\n", device_params.caus_per_channel);
        printf("cau_bits: %d\n", device_params.cau_bits);
        printf("blocks_per_cau: %d\n", device_params.blocks_per_cau);
        printf("block_bits: %d\n", device_params.block_bits);
        printf("pages_per_block: %d\n", device_params.pages_per_block);
        printf("pages_per_block_slc: %d\n", device_params.pages_per_block_slc);
        printf("page_address_bits: %d\n", device_params.page_address_bits);
        printf("address_bits_bits_per_cell: %d\n", device_params.address_bits_bits_per_cell);
        printf("default_bits_per_cell: %d\n", device_params.default_bits_per_cell);
        printf("page_size: %d\n", device_params.page_size);
        printf("dies_per_channel: %d\n", device_params.dies_per_channel);
        printf("block_pairing_scheme: %d\n", device_params.block_pairing_scheme);
        printf("bytes_per_row_address: %d\n", device_params.bytes_per_row_address);
        printf("pages_in_block0: %d\n", device_params.pages_in_block0);
        printf("pages_in_read_verify: %d\n", device_params.pages_in_read_verify);
        printf("multi_cau_bits: %d\n", device_params.multi_cau_bits);
        printf("pattern_bits: %d\n", device_params.pattern_bits);
        printf("channel0_die: %d\n",device_params.channel0_die);
        printf("package_blocks_at_EOL: %d\n", device_params.package_blocks_at_EOL);
        printf("tRC: %d\n", timing_params.tRC_ns);
        printf("tREA: %d\n", timing_params.tREA_ns);
        printf("tREH: %d\n", timing_params.tREH_ns);
        printf("tRHOH: %d\n", timing_params.tRHOH_ns);
        printf("tRHZ: %d\n", timing_params.tRHZ_ns);
        printf("tRLOH: %d\n", timing_params.tRLOH_ns);
        printf("tRP: %d\n", timing_params.tRP_ns);
        printf("tWC: %d\n", timing_params.tWC_ns);
        printf("tWH: %d\n", timing_params.tWH_ns);
        printf("tWP: %d\n", timing_params.tWP_ns);
        printf("read_queue_size: %d\n", device_params.read_queue_size);
        printf("program_queue_size: %d\n", device_params.program_queue_size);
        printf("erase_queue_size: %d\n", device_params.erase_queue_size);
        printf("prep_function_buffer_size: %d\n", device_params.prep_function_buffer_size);
        printf("tRST: %d\n", timing_params.tRST_ms);
        printf("tPURST: %d\n", timing_params.tPURST_ms);
        printf("tSCE: %d\n", timing_params.tSCE_ms);
        printf("tCERDY: %d\n", timing_params.tCERDY_us);
        printf("caus_per_package: %d\n", device_params.caus_per_package);
        printf("dies_per_package: %d\n", device_params.dies_per_package);
        printf("tRC_ddr: %d\n", timing_params.tRC_ddr_ns);
        printf("tREH_ddr: %d\n", timing_params.tREH_ddr_ns);
        printf("tRP_ddr: %d\n", timing_params.tRP_ddr_ns);
        printf("tDQSL: %d\n", timing_params.tDQSL_ps);
        printf("tDQSH: %d\n", timing_params.tDQSH_ps);
        printf("tDSC: %d\n", timing_params.tDSC_ps);
        printf("tDQSRE: %d\n", timing_params.tDQSRE_ps);
        printf("tDQSQ: %d\n", timing_params.tDQSQ_ps);
        printf("tDVW: %d\n", timing_params.tDVW_ps);
        printf("max_interface_speed: %d\n", timing_params.max_interface_speed_mhz);
        printf("\n");
        printf("num_bus: %d\n", device_params.num_bus);
        printf("ces_per_bus: %d\n", device_params.ces_per_bus);
        printf("logical_page_size: %d\n", device_params.logical_page_size);
    }

    return status;
}

static uint32_t tlc_pattern = 0;

static int get_tlc_pattern(void)
{
    aspproto_ppn_device_params_t *device_params;

    device_params = NULL;
    device_params = (aspproto_ppn_device_params_t *) malloc(sizeof(aspproto_ppn_device_params_t));
    assert(device_params);
    asp_get_device_parameters(device_params);

    if((tlc_pattern >= (uint32_t)((1 << device_params->pattern_bits) - 1)))
    {
        tlc_pattern = 0;
    } else {
        tlc_pattern++;
    }
    return tlc_pattern;
}

static uint32_t test_pattern = 0;

static int asp_bonfire_group_a_slc(uint32_t seed)
{
    aspproto_cmd_t *    command;

    printf("Opening ASP\n"); 
    if (!asp_send_open())
    {
        dprintf(DEBUG_CRITICAL, "Unable to open ASP\n");
        return false;
    }

    printf("Setting writable\n");
    if (!asp_set_writable())
    {
        asp.state = ASP_STATE_ERROR_NOT_WRITABLE;
        printf("Unable to set writable\n");
        return false;
    }

    assert(bonfire_slc_state == BONFIRE_SLC_STATE_NONE);
    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_BONFIRE_SLC_GROUP_A;
    command->tunnel.options.value  = seed;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not group A for SLC Bonfire.\n\n");
        return -1;
    }

    bonfire_slc_state = BONFIRE_SLC_STATE_A;

    asp_get_geometry();

    return 0;
}

static int asp_bonfire_group_b_slc(void)
{
    aspproto_cmd_t *    command;

    assert(bonfire_slc_state == BONFIRE_SLC_STATE_A);
    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_BONFIRE_SLC_GROUP_B;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not group A for SLC Bonfire.\n\n");
        return -1;
    }

    bonfire_slc_state = BONFIRE_SLC_STATE_B;

    asp_get_geometry();

    return 0;
}

static int asp_bonfire_ungroup(void)
{
    aspproto_cmd_t *    command;

    assert(bonfire_slc_state == BONFIRE_SLC_STATE_B);
    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_BONFIRE_SLC_UNGROUP;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not group A for SLC Bonfire.\n\n");
        return -1;
    }

    bonfire_slc_state = BONFIRE_SLC_STATE_NONE;

    asp_get_geometry();

    return 0;
}

static int asp_write_band(uint32_t band, aspproto_cell_type_t cell_type, bool verbose, bool random_pattern, uint32_t data_pattern)
{
    aspproto_cmd_t *    command;
    uint8_t *           dataBuf;
    uint32_t paddr32;
    uint64_t perf_ticks0;
    uint64_t perf_ticks1;
    uint64_t link_clk_freq;
    uint32_t pages_written = 0;
    uint32_t pages_in_block;
    uint32_t join_program_pages;
    aspproto_cmd_status_e status;
    uint32_t i;
    uint32_t band_size = 0;
    uint32_t buffer_length = 0;
    uint32_t sectors_per_band;

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    sectors_per_band = (uint32_t) asp_sectors_per_band(band, false);
    if(0 == sectors_per_band)
    {
        printf("Cannot program band - It has 0 good sectors.\n");
        return -1;
    }

    band_size = sectors_per_band * ASP_NAND_BLKSZ;

    join_program_pages = (cell_type == CELL_TYPE_IS_TLC) ? 3 : 1;
    pages_in_block = (cell_type == CELL_TYPE_IS_SLC) ? nand_info.pages_per_block_slc : nand_info.pages_per_block;

    if(0 != asp_reset_perf_ticks())
    {
        return -1;
    }

#if DEBUG_LARGE_HEAP_HACK
    buffer_length = band_size;
    dataBuf = (uint8_t *)IBOOT_HACK_ADDR;
#else
    buffer_length = ASP_NAND_BLKSZ;
    dataBuf = memalign(buffer_length, ASP_NAND_BLK_ALIGNMENT);
#endif //#if DEBUG_LARGE_HEAP_HACK
    if(!dataBuf)
    {
        printf("could not allocate data buffer\n");
        return -1;
    }

    assert((buffer_length % sizeof(data_pattern)) == 0);
    for(i = 0; i < buffer_length / sizeof(data_pattern); i++)
    {
        if(!random_pattern)
        {
            ((uint32_t *)dataBuf)[i] = data_pattern;
        }
        else
        {
            ((uint32_t *)dataBuf)[i] = rand();
        }
    }

    paddr32 = VADDR_TO_PADDR32(dataBuf);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_WRITE_BAND;
    command->tunnel.options.debug_epr_info.band  = band;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = buffer_length;
    command->tunnel.options.debug_epr_info.cell_type = cell_type;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;
    if(cell_type == CELL_TYPE_IS_TLC)
    {
        command->tunnel.options.debug_epr_info.tlc_pattern = test_pattern++;
    }

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not write band. Error: %d\n\n", status);
        asp_print_status_key();
        return -1;
    }
    pages_written = command->tunnel.options.debug_epr_info.output_value;

    if(0 != asp_get_perf_ticks(false, &perf_ticks0, 0))
    {
        return -1;
    }
    if(0 != asp_get_perf_ticks(false, &perf_ticks1, 1))
    {
        return -1;
    }

    if(0 != asp_get_link_clk_freq(false, &link_clk_freq))
    {
        return -1;
    }

    if(verbose)
    {
        printf("Ch0: %u bytes transferred in %llu ticks @ %llu Hz\n\n", (pages_written * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                         perf_ticks0,
                                                                         link_clk_freq);
        printf("Ch1: %u bytes transferred in %llu ticks @ %llu Hz\n\n", (pages_written * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                         perf_ticks1,
                                                                         link_clk_freq);

        printf("Ch0: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (pages_written * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                       link_clk_freq,
                                                                                                       perf_ticks0);
        printf("Ch1: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (pages_written * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                       link_clk_freq,
                                                                                                       perf_ticks1);

        printf("Ch0: Time for each superstripe in us: (%llu / %d) / (%llu / 1000000)\n\n", perf_ticks0, (pages_in_block / join_program_pages), link_clk_freq);
        printf("Ch1: Time for each superstripe in us: (%llu / %d) / (%llu / 1000000)\n\n", perf_ticks1, (pages_in_block / join_program_pages), link_clk_freq);

        printf("Note: This is actual bus utilization with negligible overhead.\n");
    }
    return 0;
}

static int asp_eploop_band(uint32_t band, aspproto_cell_type_t cell_type, uint32_t seconds, bool random_pattern, uint32_t data_pattern)
{
    uint64_t start_ticks;
    uint32_t iterations;

    start_ticks = system_time();

    if((band < 1) || (band > asp.lastBand))
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    if(seconds)
    {
        printf("Going to erase+program band: %d for %d seconds. cell_type: %d\n", band, seconds, cell_type);
    }
    else
    {
        printf("Going to erase+program band: %d once. cell_type: %d\n", band, cell_type);
    }

    iterations = 0;
    while(1)
    {
        if(-1 == asp_erase_band(band, cell_type, false))
        {
            printf("error erasing band. returning\n");
            return -1;
        }
        if(-1 == asp_write_band(band, cell_type, false, random_pattern, data_pattern))
        {
            printf("error programming band. returning\n");
            return -1;
        }
        iterations++;

        if(0 == seconds)
        {
            break;
        }
        else if(time_has_elapsed(start_ticks, (uint64_t)seconds * 1000000))
        {
            break;
        }
    }
    printf("Completed %d iterations.\n", iterations);
    return 0;
}

static int asp_get_waterfall_table(uint8_t *buffer)
{
    uint32_t paddr32;
    uint32_t table_size = 0;
    uint8_t *table = NULL;
    aspproto_cmd_t *    command;

    table_size = asp_get_waterfall_table_size();

    if (!security_allow_memory(buffer, table_size))
    {
        printf("Permission Denied\n");
        return -1;
    }
    memset(buffer, 0xA5, table_size);

    table = memalign(table_size, ASP_NAND_BLK_ALIGNMENT);
    if(!table)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate data_buffer\n");
        return -1;
    }
    memset(table, 0x00, table_size);
    paddr32 = VADDR_TO_PADDR32(table);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode         = NAND_DEBUG_GET_WATERFALL_TABLE;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = table_size;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: Could not get waterfall table.\n\n");
        return -1;
    }

    memcpy(buffer, table, table_size);

    printf("Waterfall table available at address: %p\n", buffer);

    free(table);
    return 0;
}

static int asp_get_dm(void)
{
    uint8_t *dmBuf = NULL;
    ExportDefects_t *defects = NULL;
    uint32_t dmBufSize;
    uint32_t paddr32;
    aspproto_cmd_t *    command;

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_EXPORT_BAND_DEFECTS;
    command->tunnel.buffer_paddr   = 0;
    command->tunnel.bufferLen      = 0;
    command->tunnel.options.outputLen = 0;
    command->tunnel.options.flags.forceExport   = 1;
    command->tunnel.options.flags.sizeOnly   = 1;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: no defect management list found\n\n");
        asp.state = ASP_STATE_ERROR_INVALID_PPNFW;
        return -1;
    }
    dmBufSize = command->tunnel.options.outputLen;

    if (dmBufSize == 0){
        dprintf(DEBUG_CRITICAL, "ERROR: defect management list returned size 0");
        return -1;
    }

    //get the defect list
    dmBuf = memalign(dmBufSize, ASP_NAND_BLK_ALIGNMENT);
    if(!dmBuf)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate data_buffer\n");
        return -1;
    }
    memset(dmBuf, 0x00, dmBufSize);
    paddr32 = VADDR_TO_PADDR32(dmBuf);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode  = CORE_DEBUG_EXPORT_BAND_DEFECTS;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = dmBufSize;
    command->tunnel.options.flags.forceExport   = 1;
    command->tunnel.options.flags.sizeOnly   = 0;


    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS){
        dprintf(DEBUG_CRITICAL, "ERROR: failed to get the defect management list\n\n");
        asp.state = ASP_STATE_ERROR_INVALID_PPNFW;
        free(dmBuf);
        return -1;
    }
    defects = (ExportDefects_t *)dmBuf;

    //ensure that grownBadCount doesn't exceed the buffersize
    if(defects->grownBadCount * sizeof(ExportDefectEntry_t) + sizeof(ExportDefects_t) - sizeof(uint32_t) > dmBufSize){
        dprintf(DEBUG_CRITICAL, "ERROR: number of defects, %d, exceeds buffersize of %d bytes\n",defects->grownBadCount, dmBufSize);
        free(dmBuf);
        return -1;
    }

    //print the dm list
    if (defects->grownBadCount == 0){
        printf("No Grown Bad Blocks Found\n");
    } else {
        printf("Grown Bad Blocks:\n");
        ExportDefectEntry_t * exportDefectEntries = (ExportDefectEntry_t *) &defects->exportDefectEntriesPtr;
        for(uint32_t i=0; i < defects->grownBadCount; i++)
        {
            printf("bus: %d, ce: %d, cau: %d -> block: %d cycles: %d reason: 0x%x\n",
                   exportDefectEntries[i].bus,
                   exportDefectEntries[i].ce,
                   exportDefectEntries[i].cau,
                   exportDefectEntries[i].band,
                   exportDefectEntries[i].cycles,
                   exportDefectEntries[i].reason);
        }
    }

    free(dmBuf);
    return 0;
}

static int asp_get_bbt(void)
{
    uint8_t *bbtBuf = NULL;
    uint32_t paddr32;
    uint32_t    numSec = 0;
    uint32_t    numDip = 0;
    uint32_t    numBand = 0;
    uint32_t    dip = 0;
    uint32_t    band = 0;
    uint32_t    mask = 0;
    uint32_t    statsBufSize = 0;
    uint8_t     *dipPtr = NULL;
    uint8_t     *bandPtr = NULL;
    aspproto_nand_col_t dip_info;
    aspproto_cmd_t *    command;
    int status;

    numSec = (nand_info.num_bands + 7) / 8;                                          // bit per band round up to number of bytes.
    numSec = (numSec + (ASP_NAND_BLKSZ-1)) / ASP_NAND_BLKSZ;            // then round number of bytes up to number of sector
    numDip = nand_info.num_dip;
    numBand = nand_info.num_bands;
    statsBufSize = numDip * numSec * ASP_NAND_BLKSZ;

    bbtBuf = memalign(statsBufSize, ASP_NAND_BLK_ALIGNMENT);
    if(!bbtBuf)
    {
        printf("could not allocate BBT buffer\n");
        return -1;
    }
    memset(bbtBuf, 0x00, statsBufSize);
    paddr32 = VADDR_TO_PADDR32(bbtBuf);

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_GET_BBT;
    command->count = numDip * numSec;
    command->sglAndIv[0] = paddr32;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get BBT: %d\n", status);
        return -1;
    }

    printf("BBT:\n");
    for(dip = 0; dip < numDip; dip++)
    {
        dip_info = asp_get_single_dip_info(dip);

        dipPtr = bbtBuf + (dip *numSec * ASP_NAND_BLKSZ);
        for(band=0; band < numBand; band++)
        {
            bandPtr = dipPtr + (band/8);
            mask = (0x80 >> (band % 8));
            if (0 == (*bandPtr & mask))
            {
                printf("bus: %d, ce: %d, cau: %d -> block: %d\n", dip_info.bus, dip_info.ce, dip_info.cau, band);
            }
        }
        printf("\n");
    }

    free(bbtBuf);
    return 0;
}

static void output_band_stats(uint32_t *buf32)
{
    uint32_t numBands;
    uint32_t band;
    uint32_t flags;
    uint32_t flow;
    uint32_t valid;
    uint32_t eraseCycles;
    uint32_t age;
    uint32_t GCcan;
    uint32_t GCmust;
    uint32_t GCdone;
    uint32_t pFail;
    uint32_t mode;
    uint32_t maxEraseCnt[NUM_ASP_PARITIONS] = {0, 0};
    uint32_t minEraseCnt[NUM_ASP_PARITIONS] = {~0, ~0};
    uint32_t avgEraseCnt[NUM_ASP_PARITIONS] = {0, 0};
    uint32_t maxAge[NUM_ASP_PARITIONS] = {0, 0};
    uint32_t firstBand[NUM_ASP_PARITIONS];
    uint32_t lastBand[NUM_ASP_PARITIONS];
    uint32_t * statsPtr;
    uint32_t partitions = 0;
    uint32_t partition;

    printf ( "Band stats:\n" );
    printf ( "===========\n" );

    numBands = *buf32++;
    partitions = *buf32++;

    assert(partitions <= NUM_ASP_PARITIONS);

    for(partition = 0; partition < partitions; partition++) {
        firstBand[partition] = *buf32++;
        lastBand[partition] = *buf32++;
    }

    statsPtr = buf32;

    for (band = 0; band < numBands; band++)
    {
        flags = *buf32++;
        flow = *buf32++;
        valid = *buf32++;
        eraseCycles = *buf32++;
        age         = *buf32++;

        for(partition = 0; partition < partitions; partition++) {
            if((band >= firstBand[partition]) && (band <= lastBand[partition]))
            {
                if(band == firstBand[partition])
                {
                    printf("\n%s:\n", partition_names[partition]);
                }
                break;
            }
        }

        if(partition < partitions)
        {
            maxAge[partition]      = MAX(maxAge[partition], age);
            maxEraseCnt[partition] = MAX(maxEraseCnt[partition], eraseCycles);
            minEraseCnt[partition] = MIN(minEraseCnt[partition], eraseCycles);
            avgEraseCnt[partition] += eraseCycles;
        }

        GCcan = flags & 1;
        GCmust = (flags >> 1) & 1;
        GCdone = (flags >> 2) & 1;
		pFail  = (flags >> 3) & 1;
        mode = (flags >> 4) & 0x3;

        if (~0U != valid)
        {
            printf("band:%4d\tflow:%2d\tvalid:%5d\terases:%5d\tage:%5d\tGCcan:%d GCmust:%d GCDone: %d pFail: %d mode:%d\n",
				   band, flow, valid, eraseCycles, age, GCcan, GCmust, GCdone, pFail, mode);
        }
        else
        {
            printf("band:%4d\tflow:%2d\tvalid:   NA\terases:%5d\tage:%5d\tGCcan:%d GCmust:%d GCDone: %d pFail: %d mode:%d\n",
				   band, flow, eraseCycles, age, GCcan, GCmust, GCdone, pFail, mode);
        }

    }

    for(partition = 0; partition < partitions; partition++) {
        if(lastBand[partition] >= firstBand[partition]) {
            avgEraseCnt[partition] = avgEraseCnt[partition] / (lastBand[partition] + 1 - firstBand[partition]);
            printf("%s        : Erase Cycles: Max ( %d ) Min ( %d ) Avg ( %d )\n", partition_names[partition],
                                            maxEraseCnt[partition], minEraseCnt[partition], avgEraseCnt[partition]);
            printf("                        Max band age:     ( %d )\n", maxAge[partition]);
        }
    }
}

static int asp_get_band_stats(void)
{
    uint8_t *buffer = NULL;
    uint32_t paddr32;
    uint32_t statsBufSize = 0;
    aspproto_cmd_t * command = NULL;
    int status;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    if(!command)
    {
        return -1;
    }

    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_READ_BAND_STATS;
    command->tunnel.buffer_paddr   = 0;
    command->tunnel.bufferLen      = 0;
    command->tunnel.options.outputLen = 0;
    command->tunnel.options.flags.forceExport   = 0;
    command->tunnel.options.flags.sizeOnly   = 1;

    status = asp_send_command(command->tag);

    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get Band Stats 0x%08X\n", status);
        return -1;
    }

    statsBufSize = command->tunnel.options.outputLen;

    statsBufSize = ALIGN_UP(statsBufSize, ASP_NAND_BLKSZ);

    buffer = memalign(statsBufSize, ASP_NAND_BLK_ALIGNMENT);
    if(!buffer)
    {
        printf("could not allocate Band Stats buffer\n");
        return -1;
    }
    memset(buffer, 0x00, statsBufSize);
    paddr32 = VADDR_TO_PADDR32(buffer);

    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_READ_BAND_STATS;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = statsBufSize;
    command->tunnel.options.flags.forceExport   = 0;
    command->tunnel.options.flags.sizeOnly   = 0;

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, buffer, statsBufSize);
#endif
    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to get Band Stats 0x%08X\n", status);
        free(buffer);
        return -1;
    }

#if WITH_NON_COHERENT_DMA
        platform_cache_operation(CACHE_INVALIDATE, buffer, statsBufSize);
#endif
    output_band_stats((uint32_t *)buffer);

    free(buffer);
    return 0;
}

static bool debug_counter_supported(uint32_t channel)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_DEBUG_COUNTER_SUPPORTED;
    command->tunnel.buffer_paddr    = 0;
    command->tunnel.bufferLen       = 0;
    command->tunnel.options.mask    = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);
    if (nand_info.num_bus==0)
    {
        return false;
    }
    command->tunnel.options.value = 0;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS) || (command->tunnel.options.value == 0))
    {
        dprintf(DEBUG_CRITICAL, "Unable to get debug counter for channel %d\n",channel);
        return false;
    }
    else
    {
        dprintf(DEBUG_CRITICAL, "channel %d debug counter supported: %d\n", channel, command->tunnel.options.value);
    }
    return true;
}

static bool get_debug_counter(uint32_t channel, uint8_t * buffer)
{
    uint8_t *bounce_buffer = NULL;
    uint32_t buffer_size = ASP_NAND_BLKSZ;
    bool status = false;

    if (nand_info.num_bus==0)
    {
        return false;
    }

    if (!security_allow_memory(buffer, buffer_size))
    {
        printf("Permission Denied\n");
        return -1;
    }
    memset(buffer, 0xA5, buffer_size);

    bounce_buffer = memalign(buffer_size, ASP_NAND_BLK_ALIGNMENT);
    if(!bounce_buffer)
    {
        dprintf(DEBUG_CRITICAL, "Failed to allocate data buffer\n");
        return -1;
    }
    memset(bounce_buffer, 0xC7, buffer_size);

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_GET_DEBUG_COUNTER;
    command->tunnel.buffer_paddr    = VADDR_TO_PADDR32(bounce_buffer);
    command->tunnel.bufferLen       = buffer_size;
    command->tunnel.options.mask    = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);
    command->tunnel.options.value   = 0;

    if (asp_send_command(command->tag) == ASPPROTO_CMD_STATUS_SUCCESS)
    {
        memcpy(buffer, bounce_buffer, buffer_size);
        status = true;
        printf("Debug Counter Buffer extracted for ch %d.  Save to host by running 'usb put <filename> %d'\n",
               channel, ASP_NAND_BLKSZ);
    }
    else
    {
        dprintf(DEBUG_CRITICAL, "Unable to get debug counter for channel %d\n",channel);
    }

    free(bounce_buffer);
    return status;
}

static bool reset_debug_counter(uint32_t channel)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_RESET_DEBUG_COUNTER;
    command->tunnel.buffer_paddr    = 0;
    command->tunnel.bufferLen       = 0;
    command->tunnel.options.mask    = ((1<<ANC_MAX_BUSSES)-1) & (1 << channel);
    if (nand_info.num_bus==0)
    {
        return false;
    }
    command->tunnel.options.value = 0;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to reset debug counter for channel %d\n",channel);
        return false;
    }
    else
    {
        dprintf(DEBUG_CRITICAL, "debug counter reset for channel %d\n",channel);
    }
    return true;
}

/*NOTE: This command does not read actual data to host buffer. 
  It just overwrites a placeholder sector data buffer.
  If you need actual data, use 'asp read'
  If you need bitflips, use 'asp bonfirereadband'
*/
static int asp_read_band(uint32_t band, aspproto_cell_type_t cell_type, uint32_t num_stripes, uint32_t seconds)
{
    aspproto_cmd_t *    command;
    uint8_t *           scratch_buffer; //placeholder for putting in page data. every sector.
    uint32_t            paddr32;
    uint64_t            start_ticks;
    uint64_t            perf_ticks0;
    uint64_t            perf_ticks1;
    uint64_t            link_clk_freq;
    uint64_t            pages_read = 0;
    uint32_t pages_in_block;
    uint32_t join_program_pages;
    int status;

    if(band > asp.lastBand)
    {
        printf("Band %d not allowed. Please choose a band between 1 and %d\n", band, asp.lastBand);
        return -1;
    }

    join_program_pages = (cell_type == CELL_TYPE_IS_TLC) ? 3 : 1;
    pages_in_block = (cell_type == CELL_TYPE_IS_SLC) ? nand_info.pages_per_block_slc : nand_info.pages_per_block;

    if(0 != asp_reset_perf_ticks())
    {
        return -1;
    }

    scratch_buffer = memalign(ASP_NAND_BLKSZ, ASP_NAND_BLK_ALIGNMENT);
    paddr32 = VADDR_TO_PADDR32(scratch_buffer);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_READ_BAND;
    command->tunnel.options.debug_epr_info.band  = band;
    command->tunnel.options.debug_epr_info.cell_type = cell_type;
    command->tunnel.options.debug_epr_info.num_stripes = num_stripes;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = ASP_NAND_BLKSZ;
    start_ticks = system_time();

    while(1)
    {
        status = asp_send_command(command->tag);
        printf("Finished Band read with status: %d\n", status);

        if (status != ASPPROTO_CMD_STATUS_SUCCESS)
        {
            asp_print_status_key();
            dprintf(DEBUG_CRITICAL, "ERROR: Could not read band.\n\n");
            return -1;
        }
        pages_read += command->tunnel.options.debug_epr_info.output_value;

        if(0 == seconds)
        {
            break;
        }
        else if(time_has_elapsed(start_ticks, (uint64_t)seconds * 1000000))
        {
            break;
        }
    }
    if(0 != asp_get_perf_ticks(false, &perf_ticks0, 0))
    {
        return -1;
    }
    if(0 != asp_get_perf_ticks(false, &perf_ticks1, 1))
    {
        return -1;
    }

    if(0 != asp_get_link_clk_freq(false, &link_clk_freq))
    {
        return -1;
    }

    printf("Ch0: %llu bytes transferred in %llu ticks @ %llu Hz\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                       perf_ticks0,
                                                                       link_clk_freq);
    printf("Ch1: %llu bytes transferred in %llu ticks @ %llu Hz\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                       perf_ticks1,
                                                                       link_clk_freq);

    printf("Ch0: To calculate total throughput in MiB/s: (%llu * %llu) / (%llu * 1024 * 1024)\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                     link_clk_freq,
                                                                                                     perf_ticks0);
    printf("Ch1: To calculate total throughput in MiB/s: (%llu * %llu) / (%llu * 1024 * 1024)\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                     link_clk_freq,
                                                                                                     perf_ticks1);

    printf("Ch0: Time for each superstripe in us: (%llu / %d) / (%llu / 1000000)\n\n", perf_ticks0, (pages_in_block / join_program_pages), link_clk_freq);
    printf("Ch1: Time for each superstripe in us: (%llu / %d) / (%llu / 1000000)\n\n", perf_ticks1, (pages_in_block / join_program_pages), link_clk_freq);

    printf("Note: This is actual bus utilization with negligible overhead.\n");

    return 0;
}

//Make sure these macros are the same in AppleStorageProcessor/src/aspcore/core/cmd.c
#define SLC_TEST_BAND_START 101
#define SLC_TEST_BAND_END   110
#define MBC_TEST_BAND_START 111
#define MBC_TEST_BAND_END   120

static bool prefill_complete_slc = false;
static bool prefill_complete_mbc = false;

static void asp_prefill_bands(aspproto_cell_type_t cell_type)
{
    uint32_t band_idx;
    uint32_t start_band;
    uint32_t end_band;

    if(cell_type == CELL_TYPE_IS_SLC)
    {
        if (prefill_complete_slc)
        {
            return;
        }
        start_band = SLC_TEST_BAND_START;
        end_band = SLC_TEST_BAND_END;
    }
    else
    {
        if (prefill_complete_mbc)
        {
            return;
        }
        start_band = MBC_TEST_BAND_START;
        end_band = MBC_TEST_BAND_END;
    }

    printf("Prefilling band: %d to band: %d\n", start_band, end_band);
    for(band_idx = start_band; band_idx <= end_band; band_idx++)
    {
        if(-1 == asp_erase_band(band_idx, cell_type, false))
        {
            panic("error erasing band");
        }
        if(-1 == (asp_write_band(band_idx, cell_type, false, true, 0)))
        {
            panic("error writing band");
        }
    }

    //Do prefill only once
    if(cell_type == CELL_TYPE_IS_SLC)
    {
        prefill_complete_slc = true;
    }
    else
    {
        prefill_complete_mbc = true;
    }
    printf("Prefill complete\n");
}

/* This command does superpage reads in a hard coded range of bands.
   It goes dip-by-dip and enqueues a page read. Each of that read may be either SLC or TLC/MLC.
   NOTE: This command does not read actual data to host buffer. 
   It just overwrites a placeholder data buffer.
   If you need actual data, use 'asp read'
   If you need bitflips, use 'asp bonfirereadband'
   The band range is first erassed and programmed.
*/
static int asp_read_mixed_superpage(uint32_t num_superpages, uint32_t num_stripes, uint32_t seconds)
{
    aspproto_cmd_t *    command;
    uint8_t *           scratch_buffer; //placeholder for putting in page data. every sector.
    uint32_t            paddr32;
    uint64_t            start_ticks;
    uint32_t            iterations = 0;
    uint64_t            perf_ticks0;
    uint64_t            perf_ticks1;
    uint64_t            link_clk_freq;
    uint32_t            pages_read = 0;

    asp_prefill_bands(CELL_TYPE_IS_SLC);

    if(asp_istlc(false))
    {
        asp_prefill_bands(CELL_TYPE_IS_TLC);
    }
    else
    {
        asp_prefill_bands(CELL_TYPE_IS_MLC);
    }

    printf("Will now start the reads\n");

    if(0 != asp_reset_perf_ticks())
    {
        return -1;
    }

    scratch_buffer = memalign(ASP_NAND_BLKSZ, ASP_NAND_BLK_ALIGNMENT);
    paddr32 = VADDR_TO_PADDR32(scratch_buffer);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_READ_MIXED_SUPERPAGE;
    command->tunnel.options.debug_epr_info.num_sectors = num_superpages;
    command->tunnel.options.debug_epr_info.num_stripes = num_stripes;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = ASP_NAND_BLKSZ;

    start_ticks = system_time();

    while(1)
    {
        iterations++;
        if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
        {
            dprintf(DEBUG_CRITICAL, "ERROR: Could not read band.\n\n");
            return -1;
        }
        pages_read += command->tunnel.options.debug_epr_info.output_value;

        if(0 == seconds)
        {
            break;
        }
        else if(time_has_elapsed(start_ticks, (uint64_t)seconds * 1000000))
        {
            break;
        }
    }

    if(0 != asp_get_perf_ticks(false, &perf_ticks0, 0))
    {
        return -1;
    }
    if(0 != asp_get_perf_ticks(false, &perf_ticks1 , 1))
    {
        return -1;
    }

    if(0 != asp_get_link_clk_freq(false, &link_clk_freq))
    {
        return -1;
    }

    printf("Ch0: %u bytes transferred in %llu ticks @ %llu Hz\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                     perf_ticks0,
                                                                     link_clk_freq);
    printf("Ch1: %u bytes transferred in %llu ticks @ %llu Hz\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                     perf_ticks1,
                                                                     link_clk_freq);

    printf("Ch0: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                   link_clk_freq,
                                                                                                   perf_ticks0);
    printf("Ch1: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (pages_read * nand_info.sec_per_page * ASP_NAND_BLKSZ) / 2,
                                                                                                   link_clk_freq,
                                                                                                   perf_ticks1);

    printf("Note: This is actual bus utilization with negligible overhead.\n");

    return 0;
}

/* This command does random reads in a hard coded range of bands.
   For every page address, 'sectors_per_page' is the number of sectors of that page that is read.
   'num_sectors' is the number of page addresses sent to the NAND.
   'num_stripes' is the number of stripes done in one read transaction.
   'same_die' determines if in a stripe, pages on the same die are read, or not, or if its random.
   NOTE: This command does not read actual data to host buffer. 
   It just overwrites a placeholder sector data buffer.
   If you need actual data, use 'asp read'
   If you need bitflips, use 'asp bonfirereadband'
   The band range is first erassed and programmed.
*/
static int asp_read_random(aspproto_cell_type_t cell_type, uint32_t num_sectors, uint32_t num_stripes, uint32_t sectors_per_page, uint32_t same_die ,uint32_t seconds)
{
    aspproto_cmd_t *    command;
    uint8_t *           scratch_buffer; //placeholder for putting in page data. every sector.
    uint32_t            paddr32;
    uint64_t            start_ticks;
    uint32_t            iterations = 0;
    uint64_t            perf_ticks0;
    uint64_t            perf_ticks1;
    uint64_t            link_clk_freq;

    if(sectors_per_page > nand_info.sec_per_page)
    {
        printf("unit_size (%d) should be < sectors per page (%d)\n", sectors_per_page, nand_info.sec_per_page);
        return -1;
    }

    asp_prefill_bands(cell_type);
    printf("Will now start the reads\n");

    if(0 != asp_reset_perf_ticks())
    {
        return -1;
    }

    scratch_buffer = memalign(ASP_NAND_BLKSZ, ASP_NAND_BLK_ALIGNMENT);
    paddr32 = VADDR_TO_PADDR32(scratch_buffer);

    command                        = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op                    = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode    = CORE_DEBUG_READ_RANDOM;
    command->tunnel.options.debug_epr_info.cell_type = cell_type;
    command->tunnel.options.debug_epr_info.num_sectors = num_sectors;
    command->tunnel.options.debug_epr_info.num_stripes = num_stripes;
    command->tunnel.options.debug_epr_info.same_die    = same_die;
    command->tunnel.options.debug_epr_info.sectors_per_page = sectors_per_page;
    command->flags.all                               = 0;
    command->flags.noAesKey                          = 1;
    command->tunnel.buffer_paddr   = paddr32;
    command->tunnel.bufferLen      = ASP_NAND_BLKSZ;
    if(cell_type == CELL_TYPE_IS_MIXED)
    {
        command->tunnel.options.flags.mixedMode = 1;
    }

    start_ticks = system_time();

    while(1)
    {
        iterations++;
        if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
        {
            dprintf(DEBUG_CRITICAL, "ERROR: Could not read band.\n\n");
            return -1;
        }

        if(0 == seconds)
        {
            break;
        }
        else if(time_has_elapsed(start_ticks, (uint64_t)seconds * 1000000))
        {
            break;
        }
    }

    if(0 != asp_get_perf_ticks(false, &perf_ticks0, 0))
    {
        return -1;
    }
    if(0 != asp_get_perf_ticks(false, &perf_ticks1, 1))
    {
        return -1;
    }

    if(0 != asp_get_link_clk_freq(false, &link_clk_freq))
    {
        return -1;
    }

    printf("Ch 0: %u bytes transferred in %llu ticks @ %llu Hz\n\n", (iterations * num_sectors * ASP_NAND_BLKSZ) / 2,
                                                                      perf_ticks0,
                                                                      link_clk_freq);
    printf("Ch 1: %u bytes transferred in %llu ticks @ %llu Hz\n\n", (iterations * num_sectors * ASP_NAND_BLKSZ) / 2,
                                                                      perf_ticks1,
                                                                      link_clk_freq);


    printf("Ch 0: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (iterations * num_sectors * ASP_NAND_BLKSZ) / 2,
                                                                                                    link_clk_freq,
                                                                                                    perf_ticks0);
    printf("Ch 1: To calculate total throughput in MiB/s: (%u * %llu) / (%llu * 1024 * 1024)\n\n", (iterations * num_sectors * ASP_NAND_BLKSZ) / 2,
                                                                                                    link_clk_freq,
                                                                                                    perf_ticks1);

    printf("Ch 0: Time taken for %d 4k sector in us: (%llu / %d) / (%llu / 1000000)\n\n", sectors_per_page, perf_ticks0, ((iterations * num_sectors) / 2), link_clk_freq); // Divide by 2 is because 2 buses are active at a time
    printf("Ch 1: Time taken for %d 4k sector in us: (%llu / %d) / (%llu / 1000000)\n\n", sectors_per_page, perf_ticks1, ((iterations * num_sectors) / 2), link_clk_freq); // Divide by 2 is because 2 buses are active at a time

    printf("Note: This is actual bus utilization with negligible overhead.\n");

    return 0;
}

static int asp_bonfire_read_band(uint32_t band, aspproto_cell_type_t cell_type, bool stats)
{
    uint8_t     *statsBuf = NULL;
    uint32_t    paddr32 = 0;
    aspproto_cmd_t *    command;
    int status;
    uint32_t sectors_per_band;

    if((band > asp.lastBand))
    {
        printf("Enter valid band: <= %d.\n", asp.lastBand);
        return -1;
    }

    sectors_per_band = (uint32_t) asp_sectors_per_band(band, false);
    if(0 == sectors_per_band)
    {
        printf("Cannot program band - It has 0 good sectors.\n");
        return -1;
    }

    statsBuf = memalign(sectors_per_band * nand_info.bytes_per_sec_meta, ASP_NAND_BLK_ALIGNMENT);
    if(!statsBuf)
    {
        printf("could not allocate stats buffer\n");
        return -1;
    }
    memset(statsBuf, 0xFF, sectors_per_band * nand_info.bytes_per_sec_meta);
    if(stats)
    {
        paddr32 = VADDR_TO_PADDR32(statsBuf);
    }
    command = asp_get_cmd_for_tag(ASP_TAG_NAND);

    command->flags.all = 0;
    command->flags.noAesKey = 1;
    command->op = ASPPROTO_CMD_BONFIRE_READ;
    command->bonfire_info.num_vpn = sectors_per_band / nand_info.sec_per_page;

    if(cell_type == CELL_TYPE_IS_SLC) {
        command->bonfire_info.start_vpn = band * (nand_info.sec_per_full_band_slc / nand_info.sec_per_page);
    } else {
        command->bonfire_info.start_vpn = band * (nand_info.sec_per_full_band / nand_info.sec_per_page);
    }
    command->bonfire_info.stats_buf32 = paddr32;
    command->bonfire_info.cell_type = cell_type;

    status = asp_send_command(command->tag);


    if(stats)
    {
        printf("Sector Stats:\n");
        uint8_t *page_cursor = statsBuf;
        uint8_t *buf_stats_cursor = page_cursor;
        bool zero_flips = true;
        uint32_t idx = 0, pageIdx = 0;
        uint32_t sectors_per_page = 16;

        for(pageIdx = 0; pageIdx < command->bonfire_info.num_vpn;  pageIdx++)
        {
            buf_stats_cursor = page_cursor;
            for(idx = 0; idx < sectors_per_page; idx++)
            {
                if(*buf_stats_cursor != 0)
                {
                    zero_flips = false;
                    break;
                }
                buf_stats_cursor++;
            }
            if(!zero_flips)
            {
                buf_stats_cursor = page_cursor ;
                printf("vpn: %d: ", pageIdx);
                for(idx = 0; idx < sectors_per_page; idx++)
                {
                    printf("%02X ", *buf_stats_cursor);
                    buf_stats_cursor++;
                }
                printf("\n");
            }
            page_cursor = page_cursor + 16;
        }
    }

    printf("Finished Band read with status: %d\n", status);

    asp_print_status_key();

    free(statsBuf);
    return 0;
}

static int asp_nand_read_id(void)
{
    uint32_t i = 0;
    printf("\nChip ID:\n");
    for(i = 0; i < nand_info.num_bus; i++)
    {
        printf("Bus %d: %02X %02X %02X %02X %02X %02X\n",
           i,
           asp.chip_id[i][0], asp.chip_id[i][1], asp.chip_id[i][2],
           asp.chip_id[i][3], asp.chip_id[i][4], asp.chip_id[i][5]);
    }

    printf("\nMfg ID:\n");
    for(i = 0; i < nand_info.num_bus; i++)
    {
        printf("Bus %d: %02X %02X %02X %02X %02X %02X\n",
           i,
           asp.mfg_id[i][0], asp.mfg_id[i][1], asp.mfg_id[i][2],
           asp.mfg_id[i][3], asp.mfg_id[i][4], asp.mfg_id[i][5]);
    }

    asp_nand_get_uid();
    return 0;
}

void asp_test_write_bdev(uint32_t start_lba, int num_lbas)
{
    uint8_t *write_ptr;
    int result;

    if(!asp.writable)
    {
        panic("Need to make system writable. Execute 'asp setwritable' first\n");
    }

    printf("Going to do a bdev write\n");

    write_ptr = malloc(num_lbas * ASP_NAND_BLKSZ);
    if(!write_ptr)
    {
        panic("Could not allocate memory\n");
    }

    asp.state = ASP_STATE_READY;
    memset(write_ptr, TEST_DATA_PATTERN, num_lbas * ASP_NAND_BLKSZ);
    result = asp_write_block(asp_nand_dev, write_ptr, start_lba, num_lbas);
    printf("bdev write returned %d\n", result);

    if(num_lbas != result)
    {
        panic("bdev write returned %d instead of %d\n", result, num_lbas);
    }
    free(write_ptr);
}

void asp_test_read_bdev(uint32_t start_lba, int num_lbas)
{
    int result;
    uint8_t *read_ptr = (uint8_t *) env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    int i;

    printf("Going to do a bdev read\n");

    if (!security_allow_memory(read_ptr, num_lbas * ASP_NAND_BLKSZ))
    {
        panic("Permission Denied\n");
    }

    asp.state = ASP_STATE_READY;
    memset(read_ptr, WATERMARK_PATTERN, num_lbas * ASP_NAND_BLKSZ);
    result = asp_read_block(asp_nand_dev, read_ptr, start_lba, num_lbas);

    if(num_lbas != result)
    {
        panic("bdev read returned %d instead of %d\n", result, num_lbas);
    }

    for(i = 0; i < num_lbas * ASP_NAND_BLKSZ; i++)
    {
        if(TEST_DATA_PATTERN != read_ptr[i])
        {
            panic("expected data: 0x%X, read data: 0x%X, byte offset in buffer: %d\n", TEST_DATA_PATTERN, read_ptr[i], i);
        }
    }
    printf("bdev read returned %d\n", result);
}

void asp_test_waterfall_start(void)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    printf("Going to start waterfall\n");
    command->op  = ASPPROTO_CMD_WATERFALL_START;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        panic("ASP waterfall table start Failed\n");
    }
}

bool asp_test_waterfall(void)
{
    uint32_t lba = 0;
    int count = 256;

    printf("Opening ASP\n"); 
    if (!asp_send_open())
    {
        dprintf(DEBUG_CRITICAL, "Unable to open ASP\n");
        return false;
    }

    printf("Setting writable\n");
    if (!asp_set_writable())
    {
        asp.state = ASP_STATE_ERROR_NOT_WRITABLE;
        printf("Unable to set writable\n");
        return false;
    }

    printf("Going to util format\n");
    if (!asp_format(ASP_FORMAT_UTIL))
    {
        printf("Failed to do util format\n");
        return false;
    }

    asp_get_geometry();

    asp_test_write_bdev(lba, count);

    asp_test_waterfall_start();

    asp_test_read_bdev(lba, count);

    lba += count;
    asp_test_write_bdev(lba, count);

    asp_test_waterfall_start();

    asp_test_read_bdev(lba, count);

    lba += count;
    asp_test_write_bdev(lba, count);

    asp_test_waterfall_start();

    asp_test_read_bdev(lba, count);

    return true;
}

static int do_asp(int argc, struct cmd_arg *args)
{
    void *buffer;
    size_t buffer_len;
    int err = 0;
    const char *cmd;

    if (argc < 2)
    {
       printf("Not enough arguments\n");
       usage();
       return 0;
    }

    buffer = (void *) env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    buffer_len = (size_t) env_get_uint("filesize", 0);
    cmd = args[1].str;

    if (asp.state == ASP_STATE_ERROR_CRITICAL)
    {
        printf("Error - ASP has failed to initialize successfully!\n");
        return -1;
    }

    if (!strcmp(args[1].str, "sync"))
    {
        if (!asp_sync())
        {
            printf("Failed to sync ASP\n");
            return -1;
        }
    }
#if defined(ASP_ENABLE_NEURALIZE) && ASP_ENABLE_NEURALIZE
    else if (!strcmp(args[1].str, "neuralize"))
    {
        if (!asp_set_writable())
        {
            asp.state = ASP_STATE_ERROR_NOT_WRITABLE;
            printf("Unable to neuralize device; media is not writable\n");
            return -1;
        }

        if (!asp_neuralize())
        {
            printf("Failed to neuralize\n");
            asp.state = ASP_STATE_ERROR_NEURALIZE;
            return -1;
        }
    }
#endif
    else if (!strcmp(cmd, "info"))
    {
        printf("nand info: \n");
        err = asp_nand_info();
    }
    else if (!strcmp(cmd, "readid"))
    {
        err = asp_nand_read_id();
    }
    else if (!strcmp(cmd, "istlc"))
    {
        err = asp_istlc(true);
    }
    else if(!strcmp(cmd, "getlinkclkfreq"))
    {
        err = asp_get_link_clk_freq(true, NULL);
    }
    else if(!strcmp(cmd, "resetperfticks"))
    {
        err = asp_reset_perf_ticks();
    }
    else if(!strcmp(cmd, "getperfticks"))
    {
        err = asp_get_perf_ticks(true, NULL, 0);
        err = asp_get_perf_ticks(true, NULL, 1);
    }
    else if(!strcmp(cmd, "printslcbonfirebands"))
    {
        err = asp_print_slc_bonfire_bands();
    }
    else if (!strcmp(cmd, "test_scratchpad"))
    {
        err = asp_test_scratchpad();
    }
    else if (!strcmp(cmd, "dies_in_parallel") && argc>=3)
    {
        if (argc==3)
        {
            err = asp_set_dies_in_parallel(args[2].u,args[2].u,
                                           args[2].u,args[2].u,
                                           args[2].u,args[2].u,
                                           args[2].u,args[2].u,
                                           CORE_POWERSTATE_HIGH_POWER);
            asp_set_power_state(CORE_POWERSTATE_HIGH_POWER);
        }
        else if (argc == 10)
        {
            err = asp_set_dies_in_parallel(args[2].u,args[3].u,
                                           args[4].u,args[5].u,
                                           args[6].u,args[7].u,
                                           args[8].u,args[9].u,
                                           CORE_POWERSTATE_HIGH_POWER);
            asp_set_power_state(CORE_POWERSTATE_HIGH_POWER);
        }
        else
        {
            usage();
            return 0;
        }
    }
    else if (!strcmp(cmd, "set_photoflow_slc"))
    {
        err = asp_set_photoflow_mode(CORE_FLOW_MODE_SLC);
    }
    else if (!strcmp(cmd, "set_photoflow_mlc"))
    {
        err = asp_set_photoflow_mode(CORE_FLOW_MODE_MLC);
    }
    else if (!strcmp(cmd, "enable_bg"))
    {
        err = asp_enable_bg();
    }
    else if (!strcmp(cmd, "disable_bg"))
    {
        err = asp_disable_bg();
    }
    else if (!strcmp(cmd, "debug_counter_supported"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = debug_counter_supported(args[2].u);
    }
    else if (!strcmp(cmd, "get_debug_counter"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = get_debug_counter(args[2].u, (uint8_t *)buffer);
    }
    else if (!strcmp(cmd, "reset_debug_counter"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = reset_debug_counter(args[2].u);
    }
    else if (!strcmp(cmd, "bonfirereadband"))
    {
        if(argc < 5)
        {
            usage();
            return 0;
        }
        err = asp_bonfire_read_band(args[2].u, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "readband"))
    {
        if(argc < 5)
        {
            usage();
            return 0;
        }
        if(argc == 5)
        {
            err = asp_read_band(args[2].u, args[3].u, args[4].u, 0);
        }
        else 
        {
            err = asp_read_band(args[2].u, args[3].u, args[4].u, args[5].u);
        }
    }
    else if (!strcmp(cmd, "readrandom"))
    {
        if(argc < 7)
        {
            usage();
            return 0;
        }
        if(argc == 7)
        {
            err = asp_read_random(args[2].u, args[3].u, args[4].u,  args[5].u, args[6].u, 0);
        }
        else
        {
            err = asp_read_random(args[2].u, args[3].u, args[4].u,  args[5].u, args[6].u, args[7].u);
        }
    }
    else if (!strcmp(cmd, "readmixedsuperpage"))
    {
        if(argc < 4)
        {
            usage();
            return 0;
        }
        if(argc == 4)
        {
            err = asp_read_mixed_superpage(args[2].u, args[3].u, 0);
        }
        else
        {
            err = asp_read_mixed_superpage(args[2].u, args[3].u, args[4].u);
        }
    }
    else if (!strcmp(cmd, "v2p"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_nand_get_addr(args[2].u, NULL, NULL, true);
    }
    else if (!strcmp(cmd, "l2dbp"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_l2dbp(args[2].u);
    }
    else if (!strcmp(cmd, "r2cbp"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_r2cbp(args[2].u);
    }
    else if (!strcmp(cmd, "cbp2r"))
    {
        if(argc < 5)
        {
            usage();
            return 0;
        }
        err = asp_cbp2r(args[2].u, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "dbp2r"))
    {
        if(argc < 5)
        {
            usage();
            return 0;
        }
        err = asp_dbp2r(args[2].u, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "secperband"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_sectors_per_band(args[2].u, true);
    }
    else if (!strcmp(cmd, "getburnincode"))
    {
        err = asp_get_burnin_code();
    }
    else if (!strcmp(cmd, "setTLCwritestripes"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_set_tlcwritestripes(args[2].u);
    }
    else if (!strcmp(cmd, "getlastfailure"))
    {
        err = asp_get_last_failure();
    }
    else if (!strcmp(cmd, "bbt"))
    {
        err = asp_get_bbt();
    }
    else if (!strcmp(cmd, "dm"))
    {
        err = asp_get_dm();
    }
    else if (!strcmp(cmd, "waterfall_size"))
    {
        err = asp_get_waterfall_table_size();
    }
    else if (!strcmp(cmd, "waterfall"))
    {
        err = asp_get_waterfall_table((uint8_t *)buffer);
    }
    else if (!strcmp(cmd, "devparam"))
    {
        err = asp_print_ppn_parameters();
    }
    else if (!strcmp(cmd, "readverify"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_read_verify(args[2].u);
    }
    else if (!strcmp(cmd, "dipinfo"))
    {
        err = asp_get_all_dip_info();
    }
    else if (!strcmp(cmd, "read"))
    {
        if(argc < 9)
        {
            usage();
            return 0;
        }
        err = asp_physical_read(args[2].u, args[3].u, args[4].u, args[5].u, args[6].u, args[7].u, args[8].u, (uint8_t *)buffer, true, NULL);
    }
    else if (!strcmp(cmd, "readpagemeta"))
    {
        if(argc < 6)
        {
            usage();
            return 0;
        }
        err = asp_read_pagemeta(args[2].u, args[3].u, args[4].u, args[5].u, (uint8_t *)buffer);
    }
    else if (!strcmp(cmd, "readbandmeta"))
    {
        if(argc < 4)
        {
            usage();
            return 0;
        }
        err = asp_read_bandmeta(args[2].u, (uint8_t *)buffer, args[3].u);
    }
    else if (!strcmp(cmd, "bandstat"))
    {
        err = asp_get_band_stats();
    }
    else if (!strcmp(cmd, "disableuid"))
    {
        err = asp_disable_uid();
    }
    else if (!strcmp(cmd, "dumpblog"))
    {
        if(argc < 5)
        {
            usage();
            return 0;
        }
        err = asp_dump_blog(args[2].u, (uint8_t *)buffer, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "vthsweep"))
    {
        if(argc < 6)
        {
            usage();
            return 0;
        }
        err = asp_vth_sweep(args[2].u, args[3].u, args[4].u, args[5].str);
    }
    else if (!strcmp(cmd, "rma_delete"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_rma_delete(args[2].u);
    }
    else if (!strcmp(cmd, "rma_configure"))
    {
        if(argc < 4)
        {
            usage();
            return 0;
        }
        err = asp_rma_configure(args[2].u, args[3].u);
    }
    else if (!strcmp(cmd, "rma_set"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_rma_set(args[2].u);
        err &= asp_recover();
    }
    else if (!strcmp(cmd, "rma_get"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        } else if(argc == 3) {
            err = asp_rma_get(args[2].u, NULL, NULL);
        } else {
            err = asp_rma_get(args[2].u, args[3].str, NULL);
        }
    }
    else if (!strcmp(cmd, "ppn_recover"))
    {
        err = asp_recover();
    }
    else if (!strcmp(cmd, "ppn_get_calibration"))
    {
        if(argc < 2)
        {
            usage();
            return 0;
        } else if(argc == 2) {
            err = asp_ppn_get_calibration(NULL);
        } else {
            err = asp_ppn_get_calibration(args[2].str);
        }
    }
    else if (!strcmp(cmd, "testbdevread"))
    {
        if(argc < 4)
        {
            usage();
            return 0;
        }
        asp_test_read_bdev(args[2].u, args[3].u);
    }
#if ASP_ENABLE_WRITES
    else if (!strcmp(args[1].str, "utilFormat"))
    {
        if (!asp_format(ASP_FORMAT_UTIL))
        {
            printf("Failed to format media\n");
            return -1;
        }
        return 0;
    }
    else if (!strcmp(args[1].str, "lbaFormat"))
    {
        if (!asp_format(ASP_FORMAT_LBA))
        {
            printf("Failed to format media\n");
            return -1;
        }
        return 0;
    }
    else if (!strcmp(args[1].str, "register"))
    {
        if(!asp.writable)
        {
            printf("Need to make system writable. Execute 'asp setwritable' first\n");
            return -1;
        }

        if (!asp_get_geometry())
        {
            asp.state = ASP_STATE_ERROR_CRITICAL;
            printf("Unable to register device; media is unformatted\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_NVRAM))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_NVRAM block device\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_NAND))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_NAND block device\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_FIRMWARE))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_FIRMWARE block device\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_LLB))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_LLB block device\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_EFFACEABLE))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_EFFACEABLE block device\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_SYSCFG))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_SYSCFG block device\n");
            return -1;
        }

        if (!asp_create_block_device(ASP_PANICLOG))
        {
            asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
            printf("Failed to create ASP_PANICLOG block device\n");
            return -1;
        }

        asp.state = ASP_STATE_READY;

        partition_scan_and_publish_subdevices("asp_nand");

        return 0;
    }
    else if (!strcmp(cmd, "bonfirewriteband"))
    {
        bool random_pattern = true;
        uint32_t data_pattern = 0;
        if(argc < 4)
        {
            usage();
            return 0;
        }
        if(argc > 4)
        {
            data_pattern = args[4].u;
            random_pattern = false;
        }

        err = asp_bonfire_write_band(args[2].u, args[3].u, random_pattern, data_pattern);
    }
    else if (!strcmp(cmd, "bonfireeraseband"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_bonfire_erase_band(args[2].u);
    }
    else if (!strcmp(cmd, "groupaslc"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_bonfire_group_a_slc(args[2].u);
    }
    else if (!strcmp(cmd, "groupbslc"))
    {
        err = asp_bonfire_group_b_slc();
    }
    else if (!strcmp(cmd, "ungroup"))
    {
        err = asp_bonfire_ungroup();
    }
    else if (!strcmp(cmd, "writeband"))
    {
        bool random_pattern = true;
        uint32_t data_pattern = 0;
        if(argc < 4)
        {
            usage();
            return 0;
        }
        if(argc > 4)
        {
            data_pattern = args[4].u;
            random_pattern = false;
        }

        err = asp_write_band(args[2].u, args[3].u, true, random_pattern, data_pattern);
    }
    else if (!strcmp(cmd, "eploop"))
    {
        bool random_pattern = true;
        uint32_t data_pattern = 0;
        if(argc < 5)
        {
            usage();
            return 0;
        }
        if(argc > 5)
        {
            data_pattern = args[5].u;
            random_pattern = false;
        }

        err = asp_eploop_band(args[2].u, args[3].u, args[4].u, random_pattern, data_pattern);
    }
    else if (!strcmp(cmd, "eraseband"))
    {
        if(argc < 4)
        {
            usage();
            return 0;
        }
        err = asp_erase_band(args[2].u, args[3].u, true);
    }
    else if (!strcmp(cmd, "testwaterfall"))
    {
        err = asp_test_waterfall();
    }
    else if (!strcmp(cmd, "testbdevwrite"))
    {
        if(argc < 4)
        {
            usage();
            return 0;
        }
        asp_test_write_bdev(args[2].u, args[3].u);
    }
    else if (!strcmp(cmd, "setwritable"))
    {
        err = asp_set_writable();
    }
    else if (!strcmp(cmd, "setburnincode"))
    {
        if(argc < 3)
        {
            usage();
            return 0;
        }
        err = asp_set_burnin_code(args[2].u);
    }
#endif // ASP_ENABLE_WRITES
    else
    {
        printf("invalid command\n");
        usage();
        err = -1;
    }

    return err;
}

MENU_COMMAND_DEVELOPMENT(asp, do_asp, "ASP maintenance and failure analysis commands", NULL);
#endif // DEBUG_BUILD && WITH_MENU

#if !RELEASE_BUILD && WITH_MENU
static bool find_blobs(void *src_buffer, UInt32 src_length, void **fw, UInt32 *fw_len)
{
    DERReturn drtn;
    DERSequence rdn;
    DERItem seq, top = { src_buffer, src_length };
    DERDecodedInfo topDecode, keyC, valC;

    if (DR_Success != DERDecodeItem(&top, &topDecode)) {
        printf("couldn't crack top level\n");
        return false;
    }

    if ((ASN1_CONSTRUCTED|ASN1_APPLICATION) != ((ASN1_CONSTRUCTED|ASN1_APPLICATION) & topDecode.tag)) {
        printf("top should be cons app\n");
        return false;
    }

    seq.length = topDecode.content.length;
    seq.data = topDecode.content.data;

    if (DR_Success != DERDecodeSeqContentInit(&seq, &rdn)) {
        printf("could not initialize DER decode\n");
        return false;
    }

    while ((drtn = DERDecodeSeqNext(&rdn, &keyC)) == DR_Success) {
        // Got key UTF8-str, check
        if (ASN1_UTF8_STRING != keyC.tag) {
            printf("key invalid: %lld\n", keyC.tag);
            return false;
        }

        // Get value Octet-str
        if (DR_Success != ((drtn = DERDecodeSeqNext(&rdn, &valC)))) {
            printf("no value found for key %lld\n", keyC.tag);
            return false;
        }

        // Check value tag
        if (ASN1_OCTET_STRING != valC.tag) {
            printf("value invalid: %lld\n", valC.tag);
            return false;
        }

        // Decode
        if ((6 == keyC.content.length) && (0 == strncmp((char*)keyC.content.data, "ppn-fw", 6))) {
            *fw = valC.content.data;
            *fw_len = valC.content.length;
        } else {
            printf("unknown key\n");
            return false;
        }
    }

    return true;
}

static int do_ppnfw(int argc, struct cmd_arg *args)
{
    addr_t      buf_ptr;
    size_t      buf_len;
    void        *fw_buffer = NULL;
    uint32_t    fw_length;

    if (argc > 3)
    {
        return -1;
    }

    buf_ptr = (argc > 1) ? args[1].u : env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    buf_len = (argc > 2) ? args[2].u : env_get_uint("filesize", 0);

    if (!security_allow_memory((void*)buf_ptr, buf_len))
    {
        return -2;
    }

    fw_buffer = NULL;
    fw_length = 0;
    find_blobs((void*)buf_ptr, buf_len, &fw_buffer, &fw_length);

    if ((NULL == fw_buffer))
    {
        printf("couldn't find the fw payload\n");
        return -3;
    }

    return asp_update_ppn_firmware(fw_buffer, fw_length);
}

MENU_COMMAND_DEVELOPMENT(ppnfw, do_ppnfw, "Update PPN controller firmware", NULL);
#endif // #if !RELEASE_BUILD && WITH_MENU
