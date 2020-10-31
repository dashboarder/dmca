/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#ifndef __S_WRITE_H__
#define __S_WRITE_H__

#include "s_internal.h"

// Init/close
extern BOOL32 s_write_init(void);
extern void   s_write_close(void);

// Multi-stream
extern void   s_write_switch(UInt32 stream);
extern void   s_write_host_switch(UInt32 stream);
extern void   s_write_gc_switch(UInt32 stream);

// Draining
extern void   s_write_push_full_buf(BOOL32 doGC);
extern BOOL32 sftl_shutdown_notify(BOOL32 ignoredOption);
extern void   s_drain_stream_cur(BOOL32 doGC);
extern void   s_drain_stream_all(BOOL32 doGC);
extern BOOL32 s_drain_stream(UInt32 stream, BOOL32 doGC);

// External write
extern Int32  sftl_write(UInt32 lba, UInt32 count, UInt8 *buf, BOOL32 isStatic);

// Internal write
extern BOOL32 s_write_multi_internal(s_write_multi_t *wm); // false=pfail

extern BOOL32 s_pad_block(UInt32 block);

// Pfail handling
extern void   s_write_reseq(void);
extern void   s_write_handle_pfail(void);

#endif // __S_WRITE_H__

