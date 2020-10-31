/*                                                                              
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef ANC_LLB_H
#define ANC_LLB_H

typedef struct dip_info_t
{
    uint32_t    cau;
    uint32_t    bus;
} dip_info_t;

typedef struct _anc_ppn_device_params_t
{
    char     header[16];
    uint32_t   caus_per_channel;
    uint32_t   cau_bits;
    uint32_t   blocks_per_cau;
    uint32_t   block_bits;
    uint32_t   pages_per_block;
    uint32_t   pages_per_block_slc;
    uint32_t   page_address_bits;
    uint32_t   address_bits_bits_per_cell;
    uint32_t   default_bits_per_cell;
    uint32_t   page_size;
    uint32_t   dies_per_channel;
    uint32_t   block_pairing_scheme;
    uint32_t   bytes_per_row_address;
    uint8_t    reserved[92];
    uint8_t    tRC;
    uint8_t    tREA;
    uint8_t    tREH;
    uint8_t    tRHOH;
    uint8_t    tRHZ;
    uint8_t    tRLOH;
    uint8_t    tRP;
    uint8_t    reserved2;
    uint8_t    tWC;
    uint8_t    tWH;
    uint8_t    tWP;
    uint8_t    reserved3[53];
    uint32_t   read_queue_size;
    uint32_t   program_queue_size;
    uint32_t   erase_queue_size;
    uint32_t   prep_function_buffer_size;
    uint32_t   tRST;
    uint32_t   tPURST;
    uint32_t   tSCE;
    uint32_t   tCERDY;
    uint8_t    reserved4[256];
} anc_ppn_device_params_t;

int anc_llb_init(void);
uint32_t anc_get_dip(uint32_t bus, uint32_t cau);
uint32_t anc_get_dies_per_channel();
uint32_t anc_llb_read_phys_page(uint32_t  band,
                                uint32_t  dip,
                                uint32_t  page,
                                uint32_t  num_lbas,
                                void     *data,
                                uint32_t *meta);


#endif // ANC_LLB_H

