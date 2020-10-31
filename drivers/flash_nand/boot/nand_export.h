/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _NAND_EXPORT_H
#define _NAND_EXPORT_H

__BEGIN_DECLS

struct _nand_boot_context
{
        NandPartProvider        npp;
        NandPartInterface *     npi;

        bool                    is_fil_available;
        bool                    syscfg_needs_init;
        bool                    have_nvram_part;
        bool                    found_fsys_offset;

        uint32_t                fsys_block_offset;
};

typedef struct _nand_boot_context nand_boot_context_t;

#define withContext(_ctxt) ((nand_boot_context_t*)(_ctxt))

bool nand_export_init(nand_boot_context_t * cxt);
void nand_failval_set(int op_num, int fail_val);

#define READ_OP 0
#define WRITE_OP        1
#define ERASE_OP        2

__END_DECLS

#endif /* ! _NAND_EXPORT_H */
