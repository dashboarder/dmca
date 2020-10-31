/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __IOSTREAM_H
#define __IOSTREAM_H

#include <sys/types.h>
#include <lib/libc.h>

struct ostream
{
	ostream & operator << (const char *str)
	{
		printf("%s", str);
		return(*this);
	}

	ostream & operator << (char *str)
	{
		printf("%s", str);
		return(*this);
	}

	ostream & operator << (int x)
	{
		printf("%d", x);
		return(*this);
	}

	ostream & operator << (unsigned int x)
	{
		printf("%u", x);
		return(*this);
	}

	ostream & operator << (long long int x)
	{
		printf("%lld", x);
		return(*this);
	}

	ostream & operator << (unsigned long long int x)
	{
		printf("%llu", x);
		return(*this);
	}

	ostream & operator << (char ch)
	{
		printf("%c", ch);
		return(*this);
	}
};

struct istream
{
	istream & operator << (char &ch)
	{
		ch = getchar();
		return(*this);
	}

};

extern const char	*endl;
extern ostream		cout;
extern ostream		cerr;
extern istream		cin;

#endif /* __IOSTREAM_H */
