/*
 *  Copyright (C) 2009  Apple, Inc. All rights reserved.
 *  
 *  This document is the property of Apple Inc.
 *  It is considered confidential and proprietary.
 *
 *  This document may not be reproduced or transmitted in any form,
 *  in whole or in part, without the express written permission of
 *  Apple Inc.
 */

#ifndef KERNELCACHE_H
#define KERNELCACHE_H	1

class Buffer;

bool DecompressKernelcache(const Buffer &kernelcache, Buffer *ret_macho);

#endif  // KERNELCACHE_H
