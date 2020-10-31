/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_IMAGE_DEVICES_H
#define __PLATFORM_IMAGE_DEVICES_H

static const struct image_device platform_image_devices[] = {
#if defined(WITH_HW_ASP)
    { "asp_fw", 0x00000 },

    #if defined(WITH_LLB_NVRAM)
    { "anc_firmware", 0x00000 } // iBoot->Dali may also use ANC firmware instead of ASP
    #endif
    #if WITH_LLB_BLKDEV
    , { "anc_llb", 0x00000 } // iBoot->Dali needs to read Dali out of block0
    #endif
#elif defined(WITH_ANC_FIRMWARE)
    { "anc_firmware", 0x00000 }
    #if WITH_LLB_BLKDEV
    , { "anc_llb", 0x00000 }
    #endif
#else
    { "nor0", 0x00000 }
#endif
};

#if WITH_SYSCFG
static const struct image_device platform_syscfg_devices[] = {
#if defined(WITH_NAND_BOOT)
       { "nand_syscfg", 0x00000 }
#else
       { "nor0", 0x00000 }
#endif
};
#endif /* WITH_SYSCFG */

#endif /* ! __PLATFORM_IMAGE_DEVICES_H */
