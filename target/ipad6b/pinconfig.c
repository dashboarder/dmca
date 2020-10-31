/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/apple/gpio.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <stdint.h>
#include <target.h>
#include <target/boardid.h>

extern uint32_t ipad6b_get_board_rev(void);
extern const uint32_t* target_get_proto2_gpio_cfg(uint32_t gpioc);

static bool use_proto2_pinconfig;
static bool board_rev_determined;

const uint32_t *
target_get_default_gpio_cfg (uint32_t gpioc)
{
    uint32_t brev;

    // HACK: The first call into target_get_default_gpio_cfg() reads
    // and attempts to restore BOARD_REV[3:0] to GPIO_CFG_DEFAULT.
    //
    // This will result in a recursive call back into
    // target_get_default_gpio_cfg. To break the recursion, while determining
    // board  rev  we can reply with any pinconfig since it's only being used
    // for BOARD_REV[3:0] and it's CFG_DISABLED in all pinconfigs.
    //
    // Remove this after we no longer have BOARD_REV-dependent pinconfigs.
    
    if (use_proto2_pinconfig) {
        return (target_get_proto2_gpio_cfg(gpioc));
    }

    if (!board_rev_determined) {
        use_proto2_pinconfig = true;

        (void)ipad6b_get_board_rev();

        use_proto2_pinconfig = false;
        board_rev_determined = true;
    }

    switch ((brev = ipad6b_get_board_rev())) {
    
    default:
        return (target_get_proto2_gpio_cfg(gpioc));
    }
}
