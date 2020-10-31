/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "L2V_Extern.h"

void L2V_Free(void)
{
    if (0 == L2V.numRoots)
        return;

    // Free arrays:

    if (NULL != L2V.Root) {
        WMR_FREE(L2V.Root, L2V.numRoots * sizeof(Root_t));
        L2V.Root = NULL;
    }

    if (NULL != L2V.Pool.Node) {
        WMR_FREE(L2V.Pool.Node, L2V_NODEPOOL_MEM);
        L2V.Pool.Node = NULL;
    }

    if(L2V_root_depth != NULL)
    {
        WMR_FREE(L2V_root_depth, L2V.numRoots);
        L2V_root_depth = NULL;
    }
    
    if(tempNodes.isEntryAvailable)
    {
        WMR_FREE(tempNodes.isEntryAvailable, tempNodes.poolSize);
    }
    if(tempNodes.tempNodeEntries)
    {
        WMR_FREE(tempNodes.tempNodeEntries, sizeof(lNodeBig_t) * tempNodes.poolSize);
    }
    tempNodes.poolSize = 0;

}

