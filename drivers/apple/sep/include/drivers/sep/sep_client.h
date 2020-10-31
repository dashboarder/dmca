/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _SEP_CLIENT_H
#define _SEP_CLIENT_H

#define SEP_NONCE_SIZE		(20)			/* in bytes */

int32_t sep_client_get_nonce(uint8_t *buffer);
int32_t sep_client_seed_aes();

#endif /* _SEP_CLIENT_H */
