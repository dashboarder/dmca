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

#ifndef _EFFACEABLE_H
#define _EFFACEABLE_H

#include <IOKit/IOTypes.h>

bool connectClient(const char * name, io_service_t * service, io_connect_t * connection);

bool wipeStorage(io_connect_t connection);
bool getCapacity(io_connect_t connection, IOByteCount * capacity);
bool getBytes(io_connect_t connection, void * buf, IOByteCount offset, IOByteCount count);
bool setBytes(io_connect_t connection, const void * buf, IOByteCount offset, IOByteCount count);
bool isFormatted(io_connect_t connection, bool * is_formatted);
bool formatStorage(io_connect_t connection);

int main(int argc, char * argv[]);
void printUsage(int argc, char *argv[]);

void hexdump(const void * buf, IOByteCount count);
void quickTest(io_connect_t connection);
void lockerList(io_connect_t connection);
void lockerSet(io_connect_t connection, char *tag, char *file);
void lockerGet(io_connect_t connection, char *tag, char *file);
void lockerEfface(io_connect_t connection, char *tag);
void generateNonce(io_connect_t connection);


#endif /* _EFFACEABLE_H */
