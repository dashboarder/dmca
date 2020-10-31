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
#ifndef __ICMP_H___
#define __ICMP_H__

#include <sys/types.h>
#include "callbacks.h"

__BEGIN_DECLS

#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

int registerICMPHandler(int messagetype,callback_handler cb);
int unregisterICMPHandler(int id);

int icmp_layer(bool on);

__END_DECLS

#endif
