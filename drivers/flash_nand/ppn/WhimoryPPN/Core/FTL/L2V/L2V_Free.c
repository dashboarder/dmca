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

#include "WMRFeatures.h"
#include "L2V_Extern.h"

#if ENABLE_L2V_TREE

void L2V_Free(void)
{
    if (0 == L2V.numRoots)
        return;

    // Free arrays:

    if (NULL != L2V.Tree) {
        WMR_FREE(L2V.Tree, L2V.numRoots * sizeof(lPtr_t));
        L2V.Tree = NULL;
    }

    if (NULL != L2V.TreeNodes) {
        WMR_FREE(L2V.TreeNodes, L2V.numRoots * sizeof(UInt32));
        L2V.TreeNodes = NULL;
    }

    if (NULL != L2V.UpdatesSinceRepack) {
        WMR_FREE(L2V.UpdatesSinceRepack, L2V.numRoots * sizeof(UInt32));
        L2V.UpdatesSinceRepack = NULL;
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

}

#endif // ENABLE_L2V_TREE

