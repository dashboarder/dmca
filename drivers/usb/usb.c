/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <string.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usb/usb_controller.h>
#include <drivers/usb/usb_core.h>

#define require(assertion, exception_label) \
        do {                                            \
                if (__builtin_expect(!(assertion), 0))  \
                {                                       \
                        goto exception_label;           \
                }                                       \
        } while (0)



#define DFU_MODE_CONFIG_STRING          "Apple Mobile Device (DFU Mode)"
#define RECOVERY_MODE_CONFIG_STRING     "Apple Mobile Device (Recovery Mode)"

///////////////////////////////////
// USB DFU mode - SecureROM only
// USB Recovery mode - iBoot only
///////////////////////////////////

static bool usb_inited;

static void usb_free()
{
	if(!usb_inited) goto early_inited_free;
    
#if WITH_USB_MODE_DFU
    
    usb_dfu_exit();
    
#elif WITH_USB_MODE_RECOVERY
    
    usb_transfer_exit();
    usb_serial_exit();
    
#endif
    
	return;
    
early_inited_free:
    
#if WITH_USB_MODE_RECOVERY
	usb_serial_exit();
#endif
	return;
}

int usb_early_init()
{
#if WITH_USB_MODE_RECOVERY
    if(usb_serial_early_init() != 0) {
		usb_free();
		return -1;
	}
#endif
    
	return 0;
}

int usb_init_with_controller(struct usb_controller_functions *controller_funcs)
{
	if(usb_inited) return 0;

	usb_controller_register(controller_funcs);

#if WITH_USB_MODE_DFU

    require((usb_core_init(DFU_MODE_CONFIG_STRING) != -1), error);
    require((usb_dfu_init() != -1), error);

#elif WITH_USB_MODE_RECOVERY

    require((usb_core_init(RECOVERY_MODE_CONFIG_STRING) != -1), error);
    require((usb_transfer_init() != -1), error);
    require((usb_serial_init() != -1), error);

#endif
    
    require((usb_core_start() != -1), error);
    
	usb_inited = true;
    
	dprintf(DEBUG_INFO, "USB inited \n");
    return 0;
	
error:
    dprintf(DEBUG_INFO, "failed to start the core\n");
	usb_free();
	return -1;
}

int usb_init()
{
	// default to Synopsys OTG controller
#if WITH_HW_SYNOPSYS_OTG
	extern struct usb_controller_functions *synopsys_otg_controller_init();

	return usb_init_with_controller(synopsys_otg_controller_init());
#else
# error "usb_init used without defined USB controller";
#endif
}

void usb_quiesce()
{
	if (usb_inited == false) return;
    
    usb_core_stop();
	usb_free();
	usb_controller_register(NULL);

	usb_inited = false;
}
