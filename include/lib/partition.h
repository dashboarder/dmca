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
#ifndef __LIB_PARTITION_H
#define __LIB_PARTITION_H

#include <sys/types.h>
#include <lib/blockdev.h>

__BEGIN_DECLS

int partition_scan_and_publish_subdevices(const char *dev_name);

__END_DECLS

#endif
