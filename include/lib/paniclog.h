/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_PANICLOG_H
#define __LIB_PANICLOG_H

#include <sys/types.h>

__BEGIN_DECLS

void clear_panic_region(unsigned char pattern);
int save_panic_log(void); // move panic log from RAM to NVRAM

__END_DECLS

#endif

