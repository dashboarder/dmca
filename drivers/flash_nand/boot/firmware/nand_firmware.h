/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _NAND_FIRMWARE_H_
#define _NAND_FIRMWARE_H_

// The ability to erase firmware is only allowed for triage/development purposes.
#define ALLOW_NAND_FIRMWARE_ERASE (!RELEASE_BUILD && WITH_MENU && WITH_NAND_FIRMWARE)

__BEGIN_DECLS

bool nand_firmware_init(NandPartInterface *npi, uint32_t partIndex);

__END_DECLS

#endif // _NAND_FIRMWARE_H_
