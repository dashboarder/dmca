// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.

#include "WhimoryBoot.h"
#include "WhimoryBootTypes.h"
#include "FTL.h"
#include "VFL.h"

Boot_FTL_Registry_t g_ftl_registry[] =
{
    { YAFTL_PPN_MAJOR_VER, YAFTL_PPN_Register },
    { 0, NULL },
};

Boot_VFL_Registry_t g_vfl_registry[] =
{
    { PPN_VFL_MAJOR_VER, PPN_VFL_GetFunctions },
    { 0, NULL },
};

