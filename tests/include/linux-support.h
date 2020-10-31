/*
* Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef IBOOT_LINUX_H
#define IBOOT_LINUX_H

#include <sys/types.h>

size_t strlcpy(char * restrict dst, const char * restrict src, size_t size);
size_t strlcat(char * restrict dst, const char * restrict src, size_t size);

#endif
