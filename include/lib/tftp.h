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
#ifndef __LIB_TFTP_H
#define __LIB_TFTP_H

#include <sys/types.h>

__BEGIN_DECLS

int tftp_transfer(const char *filename, uint32_t ipaddr, char *buffer, int *maxlen, bool write);

__END_DECLS

#endif

