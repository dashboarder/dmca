/*
 * Copyright (C) 2012-2013, 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __APPLE_CCC_H
#define __APPLE_CCC_H

#include <platform/soc/ccc.h>

void ccc_override_and_lock_iorvbar(addr_t base_address);
void ccc_handle_asynchronous_exception(void);
void ccc_enable_custom_errors(void);
void ccc_disable_custom_errors(void);
void ccc_dump_registers(void);

#endif /* __APPLE_CCC_H */
