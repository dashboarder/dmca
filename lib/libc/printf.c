/*
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <lib/libc.h>

#if defined(snprintf)
	#undef snprintf
#endif
#if defined(vsnprintf)
	#undef vsnprintf
#endif

typedef void (* printf_outfunc_t)(void *arg, char c);

typedef struct {
	printf_outfunc_t func;
	size_t	size;
	char	*buf;
	size_t	offset;
} printf_outfunc_state_t;

/* Make external libraries which uses AssertMacros header to link successfully */
void * __stderrp = NULL;

static int do_printf(printf_outfunc_state_t *state, const char *fmt, va_list ap);

static void printf_outfunc_state_init(printf_outfunc_state_t *state, printf_outfunc_t func, size_t size, char *buf)
{
	state->func = func;
	state->size = size;
	state->buf = buf;
	state->offset = 0;

	/* Terminate the start of the buffer but not the end.
	 * See <rdar://problem/8964339> 3.1.10 snprintf unsafe buffer write.
	 * This guards against an attacker theoretically tampering with 'size' and
	 * having the ability to place a zero at any memory location. */
	if ((state->buf != NULL) && (state->size > 0))
		state->buf[0] = '\0';
}

static void stdout_outfunc(void *arg __unused, char c)
{
	/* Don't emit the trailing NUL, as it's only applicable to sprintf and
	 * friends
	 */
	if (c != '\0')
		LIBC_FUNC(putchar)(c);
}

static void string_outfunc(void *arg, char c)
{
	printf_outfunc_state_t	*state = (printf_outfunc_state_t *)arg;
	if ((state->buf != NULL) && (state->offset < (state->size - 1))) {
		state->buf[state->offset++] = c;
		/* String will never be unterminated. */
		if (c != '\0') {
			state->buf[state->offset] = '\0';
		}
	}
}


int LIBC_FUNC(fprintf)(void *stream __unused, const char *fmt, ...)
{
	int err;

	va_list ap;
	va_start(ap, fmt);
	err = LIBC_FUNC(vprintf)(fmt, ap);
	va_end(ap);

	return err;
}

int LIBC_FUNC(printf)(const char *fmt, ...)
{
	int err;

	va_list ap;
	va_start(ap, fmt);
	err = LIBC_FUNC(vprintf)(fmt, ap);
	va_end(ap);

	return err;
}

int LIBC_FUNC(vprintf)(const char *fmt, va_list ap)
{
	printf_outfunc_state_t state;

	printf_outfunc_state_init(&state, stdout_outfunc, 0, NULL);
	return do_printf(&state, fmt, ap);
}

int LIBC_FUNC(snprintf)(char *str, size_t size, const char *fmt, ...)
{
	int err;

	va_list ap;
	va_start(ap, fmt);
	err = LIBC_FUNC(vsnprintf)(str, size, fmt, ap);
	va_end(ap);

	return err;
}

int LIBC_FUNC(vsnprintf)(char *str, size_t size, const char *fmt, va_list ap)
{
	printf_outfunc_state_t	state;
	int result;
	char dummy[2];

	if (size == 0) {
		/* string_outfunc doesn't handle zero sized buffers gracefully,
		 * write into a small safe buffer instead */
		str = dummy;
		size = 2;
	}

	printf_outfunc_state_init(&state, string_outfunc, size, str);
	result = do_printf(&state, fmt, ap);
	/* String is always terminated by each call to string_outfunc. */

	return(result);
}


static char *longlong_to_string(char *buf, unsigned long long n, int len, int sign, int showsign, int zeropad)
{
	int pos = len;
	int negative = 0;

	if(sign && (long long)n < 0) {
		negative = 1;
		n = -n;
	}

	buf[--pos] = 0;
	
	/* only do the math if the number is >= 10 */
	while(n >= 10) {
		int digit = n % 10;

		n /= 10;

		buf[--pos] = digit + '0';
	}
	buf[--pos] = n + '0';

	/* see if we need to zero pad this */
	while ((len - pos - 1) < zeropad) {
		buf[--pos] = '0';
	}
	
	if(negative)
		buf[--pos] = '-';
	else if(showsign)
		buf[--pos] = '+';

	return &buf[pos];
}

static char *longlong_to_hexstring(char *buf, unsigned long long u, int len, int caps)
{
	int pos = len;
	static const char hextable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static const char hextable_caps[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	const char *table;

	if(caps)
		table = hextable_caps;
	else
		table = hextable;

	buf[--pos] = 0;
	do {
		unsigned int digit = u % 16;
		u /= 16;
	
		buf[--pos] = table[digit];
	} while(u != 0);

	return &buf[pos];
}

#define LONGFLAG     0x00000001
#define LONGLONGFLAG 0x00000002
#define HALFFLAG     0x00000004
#define HALFHALFFLAG 0x00000008
#define SIZETFLAG    0x00000010
#define ALTFLAG      0x00000020
#define CAPSFLAG     0x00000040
#define SIGNFLAG     0x00000080
#define LEADZERO     0x00000100
#define SEENDOT      0x00000200
#define PADRIGHT     0x00000400
#define NUMERIC      0x00000800


static int do_printf(printf_outfunc_state_t *state, const char *fmt, va_list ap)
{
	char c;
	unsigned char uc;
	const char *s;
	const char *prefix_string;
	unsigned long long n;
	int flags;
	size_t chars_written = 0;
	char num_buffer[32];
	int nummod[2];
	int curr_mod;

#define OUTPUT_CHAR(c) do { state->func(state, c); chars_written++; } while(0)

	for(;;) {	
		/* handle regular chars that aren't format related */
		while((c = *fmt++) != 0) {
			if(c == '%')
				break; /* we saw a '%', break and start parsing format */
			OUTPUT_CHAR(c);
		}

		/* make sure we haven't just hit the end of the string */
		if(c == 0)
			break;

		/* reset the format state */
		curr_mod = nummod[0] = nummod[1] = flags = 0;
		prefix_string = NULL;

next_format:
		/* grab the next format character */
		c = *fmt++;
		if(c == 0)
			break;
					
		switch(c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (c == '0' && nummod[0] == 0) {
					flags |= LEADZERO;
					goto next_format;
				}
				nummod[curr_mod] *= 10;
				nummod[curr_mod] += (c - '0');
				goto next_format;
			case '*':
				nummod[curr_mod] = va_arg(ap, int);
				goto next_format;
			case '.':
				if (curr_mod == 0)
					curr_mod = 1;
				goto next_format;
			case '-':
				flags |= PADRIGHT;
				flags &= ~LEADZERO;
				goto next_format;
			case '%':
				OUTPUT_CHAR('%');
				break;
			case 'c':
				uc = va_arg(ap, unsigned int);
				OUTPUT_CHAR(uc);
				break;
			case 's':
				s = va_arg(ap, const char *);
				if(s == 0)
					s = "<null>";
				goto _output_string;
			case '+':
				flags |= SIGNFLAG;
				goto next_format;
			case '#':
				flags |= ALTFLAG;
				goto next_format;
			case 'l':
				if(flags & LONGFLAG)
					flags |= LONGLONGFLAG;
				flags |= LONGFLAG;
				goto next_format;
			case 'h':
				if(flags & HALFFLAG)
					flags |= HALFHALFFLAG;
				flags |= HALFFLAG;
				goto next_format;
			case 'z':
				flags |= SIZETFLAG;
				goto next_format;
			case 'i':
			case 'd':
				flags |= NUMERIC;
				n = (flags & LONGLONGFLAG) ? va_arg(ap, long long) :
					(flags & LONGFLAG) ? va_arg(ap, long) : 
					(flags & HALFHALFFLAG) ? (signed char)va_arg(ap, int) :
					(flags & HALFFLAG) ? (short)va_arg(ap, int) :
					(flags & SIZETFLAG) ? va_arg(ap, ssize_t) :
					va_arg(ap, int);
				s = longlong_to_string(num_buffer, n, sizeof(num_buffer), 1, flags & SIGNFLAG, 0);
				goto _output_string;
			case 'u':
				flags |= NUMERIC;
				n = (flags & LONGLONGFLAG) ? va_arg(ap, unsigned long long) :
					(flags & LONGFLAG) ? va_arg(ap, unsigned long) : 
					(flags & HALFHALFFLAG) ? (unsigned char)va_arg(ap, unsigned int) :
					(flags & HALFFLAG) ? (unsigned short)va_arg(ap, unsigned int) :
					(flags & SIZETFLAG) ? va_arg(ap, size_t) :
					va_arg(ap, unsigned int);
				s = longlong_to_string(num_buffer, n, sizeof(num_buffer), 0, flags & SIGNFLAG, 0);
				goto _output_string;
			case 'p':
#if !RELEASE_BUILD
				flags |= LONGFLAG | ALTFLAG;
				goto hex;
#else
				// need to consume the argument
				n = (unsigned)va_arg(ap, void *);
				// but we don't want to print the pointer
				s = "<ptr>";
				goto _output_string;
#endif
			case 'X':
				flags |= CAPSFLAG;
				goto hex;
hex:
			case 'x':
				flags |= NUMERIC;
				n = (flags & LONGLONGFLAG) ? va_arg(ap, unsigned long long) :
				    (flags & LONGFLAG) ? va_arg(ap, unsigned long) : 
					(flags & HALFHALFFLAG) ? (unsigned char)va_arg(ap, unsigned int) :
					(flags & HALFFLAG) ? (unsigned short)va_arg(ap, unsigned int) :
					(flags & SIZETFLAG) ? va_arg(ap, size_t) :
					va_arg(ap, unsigned int);
				s = longlong_to_hexstring(num_buffer, n, sizeof(num_buffer), flags & CAPSFLAG);
				if(flags & ALTFLAG)
					prefix_string = "0x";
				goto _output_string;
			case 'I':
			{
				unsigned char *ip, lip[4];
				char ipbuf[4];
				int i;
			
				/* IP address */
				ip = (unsigned char *)va_arg(ap, void *);
				num_buffer[0] = 0;
				if (flags & ALTFLAG) {
					lip[0] = ip[3];
					lip[1] = ip[2];
					lip[2] = ip[1];
					lip[3] = ip[0];
				} else {
					lip[0] = ip[0];
					lip[1] = ip[1];
					lip[2] = ip[2];
					lip[3] = ip[3];
				}
				ipbuf[0] = 0;
				for (i = 0; i < 4; i++) {
					s = longlong_to_string(ipbuf, lip[i], sizeof(ipbuf), 0, 0, 0);
					strlcat(num_buffer, s, sizeof(num_buffer));
					if (i < 3)
						strlcat(num_buffer, ".", sizeof(num_buffer));
				}
				s = num_buffer;
				goto _output_string;

			}
			case 'M':
			{
				unsigned char *mac;

				/* ethernet MAC address */
				mac = (unsigned char *)va_arg(ap, void *);
				char macbuf[4];
				int i;

				num_buffer[0] = 0;
			
				for (i = 0; i < 6; i++) {
					s = longlong_to_hexstring(macbuf, mac[i], sizeof(macbuf), 0);
					strlcat(num_buffer, s, sizeof(num_buffer));
					if (i < 5)
						strlcat(num_buffer, ":", sizeof(num_buffer));
				}
				s = num_buffer;
				goto _output_string;
				
			}
			case 'n':
				/* 
				 * XXX Do not implement %n, as it is the main 
				 * basis for printf format-string attacks.
				 */
				break;
			default:
				OUTPUT_CHAR('%');
				OUTPUT_CHAR(c);
				break;
		}

		/* move on to the next field */
		continue;

		/* shared output code */
_output_string:
		if (nummod[0] > 0) {
			int output_len = (int)strlen(s);
			int prefix_len = prefix_string == NULL ? 0 : (int)strlen(prefix_string);
			if (nummod[0] < (output_len + prefix_len))
				nummod[0] = 0;
		}

		if (nummod[0] > 0) {
			if (!(flags & PADRIGHT)) {
				/* lead pad the string with either zero or space */
				char pad = (flags & LEADZERO) ? '0' : ' ';
				int len = strlen(s);
				if (prefix_string)
					len += strlen(prefix_string);

				if (nummod[0] > len) {
					len = nummod[0] - len;
					nummod[0] -= len;

					// sign bit goes before 0-padding
					if ((flags & NUMERIC) && pad == '0' &&
					    (s[0] == '-' || s[0] == '+')) {
						OUTPUT_CHAR(*s++);
						nummod[0]--;
					}

					if (prefix_string && pad == '0') {
						while(*prefix_string != 0)
							OUTPUT_CHAR(*prefix_string++);
						prefix_string = NULL;
					}
				
					while (len-- > 0)
						OUTPUT_CHAR(pad);
				}
			}
		} else if (nummod[0] == 0) {
			nummod[0] = INT_MAX;
			flags &= ~PADRIGHT;	/* otherwise we will print a lot of spaces */
		}
		if (prefix_string) {
			while(*prefix_string != 0 && (nummod[0]-- > 0))
				OUTPUT_CHAR(*prefix_string++);
		}
		while(*s != 0 && (nummod[0]-- > 0))
			OUTPUT_CHAR(*s++);
		if (flags & PADRIGHT) {
			while (nummod[0]-- > 0)
				OUTPUT_CHAR(' ');
		}
		continue;
	}

	return chars_written;
}
