//
//  L2V_Dump_Prefix.h
//  IOFlashStorage
//
//  Created by John Vink on 7/29/11.
//  Copyright (c) 2011 Apple, Inc. All rights reserved.
//

#ifdef __OBJC__
	#import <Foundation/Foundation.h>
#endif

// include these files so that L2V_Print.c will build
#include <stdio.h>
#include <stdlib.h>

#define _WMR_ASSERT(e, file, line)	{ printf("%s:%u: failed assertion '%s'\nWaiting for debugger...\n", file, line, e); abort(); }
#define WMR_ASSERT(e)				if (!(e)) _WMR_ASSERT(#e, __FILE__, __LINE__)
#define WMR_PANIC(fmt, ...)			{ printf(fmt, ## __VA_ARGS__); abort(); }
