/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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
#if defined(WITH_NAND_BOOT)
    { "nand_firmware", 0x00000 }
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
