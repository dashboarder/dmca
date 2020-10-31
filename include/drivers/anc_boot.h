/*                                                                              
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef ANC_BOOT_H
#define ANC_BOOT_H

#define ANC_BOOT_MODE_RESET_ALL_CONTROLLERS			0
#define ANC_BOOT_MODE_RESET_ONE_CONTROLLER			1
#define ANC_MIN_ALIGNMENT					16

extern bool anc_reset(int resetMode);
extern bool anc_firmware_init(void);
extern size_t anc_read_llb(void* data, size_t size);

#endif

