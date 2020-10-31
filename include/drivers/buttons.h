/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_BUTTONS_H
#define __DRIVERS_BUTTONS_H

#include <sys/types.h>

__BEGIN_DECLS

enum buttons {
	MENU_KEY,
	VOLUP_KEY,
	VOLDOWN_KEY,
};

typedef void (*button_callback)(bool state);

int buttons_init(void);

/* Callback is executed in interrupt context */
void button_callback_register(enum buttons button, button_callback callback);
void button_callback_unregister(enum buttons button, button_callback callback);

__END_DECLS

#endif /* __DRIVERS_BUTTONS_H */
