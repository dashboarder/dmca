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
#ifndef __LIB_NET_H
#define __LIB_NET_H

#include <sys/types.h>

__BEGIN_DECLS

int start_network_stack(void); 
int stop_network_stack(void);

/* ip and mac address string manipulation routines */
void mac2str(uint8_t mac[6], char *ascii, size_t size);
void ip2str(const uint8_t *ip, char *str, size_t size);

__END_DECLS

#endif

