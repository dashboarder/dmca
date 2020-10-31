// *****************************************************************************
//
// File: fmiss.c
//
// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
// *****************************************************************************
#include "fmiss.h"


#if FMISS_ENABLED

#if FMISS_DUMP_ENABLED

static void fmiss_fifo_print(UInt32 val, UInt32 cur, UInt32 ptr, UInt32 lev, UInt32 len, BOOL32 err)
{
    BOOL32 is_head = FALSE32;
    BOOL32 is_edge = FALSE32;
    BOOL32 is_queued = FALSE32;

    if (ptr > cur)
    {
        cur += len;
    }

    if (ptr + lev == cur)
    {
        is_edge = TRUE32;
    }
    else if (ptr == cur)
    {
        is_head = TRUE32;
        is_edge = TRUE32;
    }
    else if (ptr + lev > cur)
    {
        is_queued = TRUE32;
    }

    printf("0x%08x %c %c%c   ", val,
        (err ? '!' : ' '),
        (is_head ? '<' : ' '),
        (is_edge ? '-' : (is_queued ? '|' : ' ')));
}

void fmiss_dump(h2fmi_t *fmi)
{
    const UInt32 mLength   = SEQUENCER_MACROS(~0) - SEQUENCER_MACROS(0) + 1;
    const UInt32 mControl  = h2fmi_rd(fmi, SEQ_MACRO_CONTROL);
    const UInt32 mHead     = SEQ_MACRO_CONTROL__GET_MACRO_ADDRESS(mControl);
    const UInt32 mLevel    = SEQ_MACRO_CONTROL__GET_WORD_COUNT(mControl);
    const UInt32 cLength   = SEQUENCER_COMMANDS(~0) - SEQUENCER_COMMANDS(0) + 1;
    const UInt32 cPointer  = h2fmi_rd(fmi, COMMAND_FIFO_PTR);
    const UInt32 cHead     = COMMAND_FIFO_PTR__READ(cPointer);
    const UInt32 cLevel    = COMMAND_FIFO_PTR__LEVEL(cPointer);
    const UInt32 oLength   = SEQUENCER_OPERANDS(~0) - SEQUENCER_OPERANDS(0) + 1;
    const UInt32 oPointer  = h2fmi_rd(fmi, OPERAND_FIFO_PTR);
    const UInt32 oHead     = OPERAND_FIFO_PTR__READ(oPointer);
    const UInt32 oLevel    = OPERAND_FIFO_PTR__LEVEL(oPointer);
#if FMI_VERSION >= 4
    const UInt32 sLength   = SEQUENCER_STORES(~0) - SEQUENCER_STORES(0) + 1;
    const UInt32 sPointer  = h2fmi_rd(fmi, STORE_FIFO_PTR);
    const UInt32 sHead     = STORE_FIFO_PTR__READ(sPointer);
    const UInt32 sLevel    = STORE_FIFO_PTR__LEVEL(sPointer);
#else // FMI_VERSION < 4
    const UInt32 sLength   = 0;
    const UInt32 sPointer  = 0;
#endif // FMI_VERSION < 4
    const UInt32 maxLength = WMR_MAX(WMR_MAX(mLength, cLength), WMR_MAX(oLength, sLength));
    UInt32 *macros;
    UInt32 macro_count = 0;
    UInt32 reg, i, j;

    printf("SEQ_MACRO_CONTROL: 0x%08x, COMMAND_FIFO_PTR: 0x%08x, OPERAND_FIFO_PTR: 0x%08x, STORE_FIFO_PTR: 0x%08x\n",
           mControl, cPointer, oPointer, sPointer);

    printf("SEQ_INT_PND: 0x%08x, TIMEOUT_VALUE: 0x%08x, COMMAND_INT_CODE: 0x%08x\n",
           h2fmi_rd(fmi, SEQ_INT_PEND),
           h2fmi_rd(fmi, TIMEOUT_VALUE),
           h2fmi_rd(fmi, COMMAND_INT_CODE));

    macros = (fmi->is_ppn ? fmiss_ppn_macros(&macro_count) : fmiss_raw_macros(&macro_count));

    printf("     %-18s%-18s%-18s%-18s", "Seq Macros:", "Seq Commands:", "Seq Operands:", "Seq Stores:");

    for (i = 0, j = 0 ; i < maxLength; i += sizeof(UInt32), j++)
    {
        printf("\n%-3d: ", j);

        if (i < mLength)
        {
            reg = h2fmi_rd(fmi, SEQUENCER_MACROS(i));
            fmiss_fifo_print(reg, j, mHead, mLevel, mLength / sizeof(UInt32),
                (j < macro_count ? reg != macros[j] : FALSE32));
        }

        if (i < cLength)
        {
            reg = h2fmi_rd(fmi, SEQUENCER_COMMANDS(i));
            fmiss_fifo_print(reg, j, cHead, cLevel, cLength / sizeof(UInt32), FALSE32);

        }

        if (i < oLength)
        {
            reg = h2fmi_rd(fmi, SEQUENCER_OPERANDS(i));
            fmiss_fifo_print(reg, j, oHead, oLevel, oLength / sizeof(UInt32), FALSE32);
        }

#if FMI_VERSION >= 4
        if (i < sLength)
        {
            reg = h2fmi_rd(fmi, SEQUENCER_STORES(i));
            fmiss_fifo_print(reg, j, sHead, sLevel, sLength / sizeof(UInt32), FALSE32);
        }
#endif // FMI_VERSION >= 4
    }

    printf("\n");
}

#endif // FMISS_DUMP_ENABLED

#endif // FMISS_ENABLED
