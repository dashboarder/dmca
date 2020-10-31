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
#ifndef __DRIVERS_IIS_H
#define __DRIVERS_IIS_H

#include <sys/types.h>
#include <sys.h>
#include <platform.h>

__BEGIN_DECLS

void iis_set_master(int iis, bool master);
int iis_set_bcpf(int iis, int bcpf);
int iis_set_usb(int iis, bool usb);
void iis_set_bclk_scaler(int iis, int bclk_scaler);

void iis_tx(int iis, void *buf, size_t len);

bool iis_is_done(int iis);
bool iis_wait_done(int iis, int ms);

__END_DECLS

#endif /* __DRIVERS_IIS_H */
