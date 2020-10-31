/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/* GIReturnString.c - convert a GIReturn to a string */

#include <libGiantsUtils/GIReturnString.h>
#include <stdio.h>

const char *GIReturnString(GIReturn grtn)
{
	static char unknown[100];
	
	switch(grtn) {
		case GR_Success: return "GR_Success";
		case GR_IllegalArg: return "GR_IllegalArg";
		case GR_Unimplemented: return "GR_Unimplemented";
		case GR_Internal: return "GR_Internal";
		case GR_Overflow: return "GR_Overflow";
		default:
			sprintf(unknown, "Uknown Error (%d)", (int)grtn);
			return unknown;
	}
}



