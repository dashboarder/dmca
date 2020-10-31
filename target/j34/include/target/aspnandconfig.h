/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_ASPNANDCONFIG_H
#define __TARGET_ASPNANDCONFIG_H

//MLC devices
#define ASPNAND_MLC_WRITE_DIES_IN_PARALLEL 4
#define ASPNAND_SLC_WRITE_DIES_IN_PARALLEL 2
#define ASPNAND_ERASE_DIES_IN_PARALLEL 4
#define ASPNAND_READ_DIES_IN_PARALLEL 4
#define ASPNAND_MLC_WRITE_LOW_POWER_DIES_IN_PARALLEL 2
#define ASPNAND_SLC_WRITE_LOW_POWER_DIES_IN_PARALLEL 2
#define ASPNAND_ERASE_LOW_POWER_DIES_IN_PARALLEL 2
#define ASPNAND_READ_LOW_POWER_DIES_IN_PARALLEL 2

//TLC devices
#define ASPNAND_TLC_TLC_WRITE_DIES_IN_PARALLEL 8
#define ASPNAND_TLC_SLC_WRITE_DIES_IN_PARALLEL 8
#define ASPNAND_TLC_ERASE_DIES_IN_PARALLEL 2
#define ASPNAND_TLC_READ_DIES_IN_PARALLEL 8
#define ASPNAND_TLC_TLC_WRITE_LOW_POWER_DIES_IN_PARALLEL 2
#define ASPNAND_TLC_SLC_WRITE_LOW_POWER_DIES_IN_PARALLEL 2
#define ASPNAND_TLC_ERASE_LOW_POWER_DIES_IN_PARALLEL 2
#define ASPNAND_TLC_READ_LOW_POWER_DIES_IN_PARALLEL 2

// rdar://problem/18371949 - these values should be the same for post-OkemoTaos targets
#define ASPNAND_INDIRECTION_MEMORY (2*1024*1024)
#define ASPNAND_LEGACY_INDIRECTION_MEMORY ASPNAND_INDIRECTION_MEMORY

#endif
