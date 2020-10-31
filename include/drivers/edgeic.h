/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __DRIVERS_EDGEIC_H
#define __DRIVERS_EDGEIC_H

#include <sys/types.h>

__BEGIN_DECLS

void edgeic_reset(void);
void edgeic_select_edge(u_int32_t vector, bool edge);
void edgeic_clear_interrupt(u_int32_t vector);

__END_DECLS

#endif /* __DRIVERS_EDGEIC_H */
