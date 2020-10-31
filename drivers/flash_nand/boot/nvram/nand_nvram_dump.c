/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <sys/types.h>

#include "nand_part.h"
#include "nand_nvram_platform.h"
#include "nand_nvram_impl.h"
#include "nand_nvram_dump.h"

/* ========================================================================== */

static void
dumpData(nand_nvram_t * nv, uint8_t * data)
{
        const uint32_t bytes_per_row = 16;
        char alpha[bytes_per_row + 1];
        uint32_t col = 0;
        uint32_t idx;
        
        nand_nvram_printf(nv->context, "    <data>");
        nand_nvram_set_mem(nv->context, alpha, 0x00, bytes_per_row + 1);
        for (idx = 0; idx < nv->geo.bytes_per_page; idx++) {
                uint8_t byte = data[idx];
                col = (idx % bytes_per_row);
                if (0 == col) {
                        if (0 != alpha[0]) {
                                nand_nvram_printf(nv->context, "  <!-- %s -->", alpha);
                                nand_nvram_set_mem(nv->context, alpha, 0x00, bytes_per_row + 1);
                        }
                        nand_nvram_printf(nv->context, "\n     ");
                }
                alpha[col] = ((0x20 <= byte) && (byte <= 0x7e))  ? (char) byte : '.';
                nand_nvram_printf(nv->context, "%02X ", byte);
        }
        if (0 != alpha[0]) {
                for (idx = 0; idx < (bytes_per_row - (col + 1)); idx++) {
                        nand_nvram_printf(nv->context, "   ");
                }
                nand_nvram_printf(nv->context, "  <!-- %s -->", alpha);
                nand_nvram_set_mem(nv->context, alpha, 0x00, bytes_per_row + 1);
        }
        nand_nvram_printf(nv->context, "\n    </data>\n");
}

static void
dumpMeta(nand_nvram_t * nv, meta_t * meta)
{
        nand_nvram_printf(nv->context, "    <meta_t>\n");
        nand_nvram_printf(nv->context, "     <generation>0x%08X</generation>\n", meta->generation);
        nand_nvram_printf(nv->context, "     <page_max>0x%02X</page_max>\n", meta->page_max);
        nand_nvram_printf(nv->context, "     <page_idx>0x%02X</page_idx>\n", meta->page_idx);
        nand_nvram_printf(nv->context, "     <version_major>0x%02X</version_major>\n", meta->version_major);
        nand_nvram_printf(nv->context, "     <version_minor>0x%02X</version_minor>\n", meta->version_minor);
        nand_nvram_printf(nv->context, "     <signature>0x%04X</signature>\n", meta->signature);
        nand_nvram_printf(nv->context, "    </meta_t>\n");      
}

static void
dumpPage(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t page, meta_t * meta, uint8_t * data)
{
        uint32_t abs_page = page + (block * nv->geo.pages_per_block);

        nand_nvram_printf(nv->context, "   <page>\n");
        nand_nvram_printf(nv->context, "    <bank_num>%d</bank_num>\n", bank);
        nand_nvram_printf(nv->context, "    <block_num>%d</block_num>\n", block);
        nand_nvram_printf(nv->context, "    <page_num>%d</page_num>\n", page);
        if (nand_nvram_read_page(nv->context, bank, abs_page, data, meta, NULL)) {
                NandPartTable *closing = (NandPartTable*)meta;
                if (kNAND_NVRAM_SIGNATURE == meta->signature) {
                        nand_nvram_printf(nv->context, "    <type>content</type>\n");
                        dumpMeta(nv, meta);
                } else if (kNandPartHeaderMagic == closing->header.magic) {
                        nand_nvram_printf(nv->context, "    <type>closing</type>\n");
                } else {
                        nand_nvram_printf(nv->context, "    <type>unrecognized</type>\n");
                }
                dumpData(nv, data);
        } else {
                nand_nvram_printf(nv->context, "    <type>unreadable</type>\n");
        }
        nand_nvram_printf(nv->context, "   </page>\n");
}

static void
dumpBlock(nand_nvram_t * nv, uint32_t bank, uint32_t block, meta_t * meta, uint8_t * data)
{
        uint32_t page;
        
        nand_nvram_printf(nv->context, "  <block>\n");
        nand_nvram_printf(nv->context, "   <bank_num>%d</bank_num>\n", bank);
        nand_nvram_printf(nv->context, "   <block_num>%d</block_num>\n", block);
        if (nand_nvram_is_block_bad(nv->context, bank, block)) {
                nand_nvram_printf(nv->context, "   <bad/>\n");
        } else {
                for (page = 0; page < nv->geo.pages_per_block; page++)
                {
                        dumpPage(nv, bank, block, page, meta, data);
                }
        }
        nand_nvram_printf(nv->context, "  </block>\n");
}

static void
dumpBank(nand_nvram_t * nv, uint32_t bank, meta_t * meta, uint8_t * data)
{
        uint32_t block;

        nand_nvram_printf(nv->context, " <bank>\n");
        nand_nvram_printf(nv->context, "  <bank_num>%d</bank_num>\n", bank);
        for (block = 0; block < nv->geo.blocks_per_bank; block++)
        {
                dumpBlock(nv, bank, block, meta, data);
        }
        nand_nvram_printf(nv->context, " </bank>\n");
}

static void
dumpGeometry(nand_nvram_t * nv)
{
        nand_nvram_printf(nv->context, " <nand_nvram_geometry_t>\n");
        nand_nvram_printf(nv->context, "  <number_of_banks>%d</number_of_banks>\n", nv->geo.number_of_banks);
        nand_nvram_printf(nv->context, "  <blocks_per_bank>%d</blocks_per_bank>\n", nv->geo.blocks_per_bank);
        nand_nvram_printf(nv->context, "  <pages_per_block>%d</pages_per_block>\n", nv->geo.pages_per_block);
        nand_nvram_printf(nv->context, "  <data_pages_per_block>%d</data_pages_per_block>\n", nv->geo.data_pages_per_block);
        nand_nvram_printf(nv->context, "  <bytes_per_page>%d</bytes_per_page>\n", nv->geo.bytes_per_page);
        nand_nvram_printf(nv->context, "  <bytes_per_meta_buffer>%d</bytes_per_meta_buffer>\n", nv->geo.bytes_per_meta_buffer);
        nand_nvram_printf(nv->context, "  <bytes_per_meta_actual>%d</bytes_per_meta_actual>\n", nv->geo.bytes_per_meta_actual);
        nand_nvram_printf(nv->context, " </nand_nvram_geometry_t>\n");
}

bool
nand_nvram_dump(nand_nvram_t * nv)
{
        bool ok = false;
        uint32_t bank;
        meta_t * meta = NULL;
        uint8_t * data = NULL;
        
        if ((NULL != nv) && 
            (NULL != nv->context) &&
            (NULL != (meta = nand_nvram_alloc_mem(nv->context, nv->geo.bytes_per_meta_buffer))) &&
            (NULL != (data = nand_nvram_alloc_mem(nv->context, nv->geo.bytes_per_page)))) {
        
                nand_nvram_printf(nv->context, "<nvram>\n");
                dumpGeometry(nv);
                for (bank = 0; bank < nv->geo.number_of_banks; bank++)
                {
                        dumpBank(nv, bank, meta, data);
                }
                nand_nvram_printf(nv->context, "</nvram>\n");
                ok = true;
        }
        
        if (NULL != meta) {
                nand_nvram_free_mem(nv->context, meta, nv->geo.bytes_per_meta_buffer);
        }
        if (NULL != data) {
                nand_nvram_free_mem(nv->context, data, nv->geo.bytes_per_page);
        }

        return (ok);
}

/* ========================================================================== */
