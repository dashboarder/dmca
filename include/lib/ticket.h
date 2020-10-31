/*
 * Copyright (C) 2011, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef TICKET_H
#define TICKET_H

#include <sys/types.h>

#if 1
#define TICKETLOG(x, ...)
#else
#define TICKETLOG_ENABLED	1
#define TICKETLOG(x, ...) printf("%s: " x "\n", __FUNCTION__, ## __VA_ARGS__ )
#endif

bool ticket_validation_required();

bool ticket_get_hash( uint8_t *buffer, size_t length );

int ticket_load();

int ticket_load_file( const char *filePath, uintptr_t bufferAddress, size_t bufferLength );

bool ticket_validate_image3( uint32_t type, uint32_t expectedType, uint8_t *hashBuffer, size_t hashBufferSize, bool *outIsUntrusted );

int ticket_set( const uint8_t *buffer, size_t length, bool isRestore, size_t *outTicketLength );

#endif /* TICKET_H */
