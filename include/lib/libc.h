/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_LIBC_H
#define __LIB_LIBC_H

#include <sys/types.h>
#include <compiler.h>

#if !WITH_HOST_LIBC

#include <limits.h>
#include <stdarg.h>
#include <lib/heap.h>

#define LIBC_FUNC(x) x

// for compatibility with code that gets is_pow2, ROUNDUP, etc through
// the standard lib headers. new/updated code should include libiboot.h
// directly instead of getting it through libc.h
#include <lib/libiboot.h>

#else // WITH_HOST_LIBC

// Prefix function names with libc_ in order to not collide with the real functions
#define LIBC_FUNC(x) libc_##x

#endif

__BEGIN_DECLS

/* stdio stuff */
int LIBC_FUNC(puts)(const char *str);
int LIBC_FUNC(getchar)(void);
int LIBC_FUNC(putchar)(int c);

int LIBC_FUNC(printf)(const char *fmt, ...) __printflike(1, 2);
int LIBC_FUNC(vprintf)(const char *fmt, va_list ap);
int LIBC_FUNC(snprintf)(char *str, size_t size, const char *fmt, ...) __printflike(3, 4);
int LIBC_FUNC(vsnprintf)(char *str, size_t size, const char *fmt, va_list ap);

/* fake fprintf to accomodate AssertMacros.h */
#if !WITH_HOST_LIBC
#define stdout	0
#define stderr	0	
#endif
int LIBC_FUNC(fprintf)(void *stream, const char *fmt, ...) __printflike(2, 3); 

/* string */
void * LIBC_FUNC(memchr)(void const *, int, size_t);
int    LIBC_FUNC(memcmp)(void const *, const void *, size_t);
int    LIBC_FUNC(memcmp_secure)(void const *, const void *, size_t);
void * LIBC_FUNC(memcpy)(void *, void const *, size_t);
void * LIBC_FUNC(memmove)(void *, void const *, size_t);
void * LIBC_FUNC(memset)(void *, int, size_t);

char       * LIBC_FUNC(strchr)(char const *, int);
int          LIBC_FUNC(strcmp)(char const *, char const *);
size_t       LIBC_FUNC(strlen)(char const *);
int          LIBC_FUNC(strncmp)(char const *, char const *, size_t);
char       * LIBC_FUNC(strpbrk)(char const *, char const *);
char       * LIBC_FUNC(strrchr)(char const *, int);
size_t       LIBC_FUNC(strspn)(char const *, char const *);
size_t       LIBC_FUNC(strcspn)(const char *s, const char *);
char       * LIBC_FUNC(strstr)(char const *, char const *);
char       * LIBC_FUNC(strtok)(char *, char const *);
int          LIBC_FUNC(strcoll)(const char *s1, const char *s2);
size_t       LIBC_FUNC(strxfrm)(char *dest, const char *src, size_t n);
char       * LIBC_FUNC(strdup)(const char *str);

/* non standard string */
void  * LIBC_FUNC(bcopy)(void const *, void *, size_t);
void    LIBC_FUNC(bzero)(void *, size_t);
size_t  LIBC_FUNC(strlcat)(char *, char const *, size_t);
size_t  LIBC_FUNC(strlcpy)(char *, char const *, size_t);
int     LIBC_FUNC(strncasecmp)(char const *, char const *, size_t);
int     LIBC_FUNC(strnicmp)(char const *, char const *, size_t);
size_t  LIBC_FUNC(strnlen)(char const *s, size_t count);
char *  LIBC_FUNC(strsep)(char **stringp, const char *delim);

/* secure string functions */
#if !WITH_HOST_LIBC
#if !defined(__cplusplus)
#if __has_builtin(__builtin___memcpy_chk)
	#define memcpy(dest, src, len) __builtin___memcpy_chk(dest, src, len, __builtin_object_size(dest, 0))
#endif
#if __has_builtin(__builtin___memmove_chk)
	#define memmove(dest, src, len) __builtin___memmove_chk(dest, src, len, __builtin_object_size(dest, 0))
#endif
#if __has_builtin(__builtin___memset_chk)
	#define memset(dest, c, len) __builtin___memset_chk(dest, c, len, __builtin_object_size(dest, 0))
#endif

#define bcopy(src, dest, len) memcpy(dest, src, len)
#define bzero(dest, len) memset(dest, 0, len)
#if __has_builtin(__builtin___strlcat_chk)
	#define strlcat(dest, src, len) __builtin___strlcat_chk(dest, src, len, __builtin_object_size(dest, 1))
#endif
#if __has_builtin(__builtin___strlcpy_chk)
	#define strlcpy(dest, src, len) __builtin___strlcpy_chk(dest, src, len, __builtin_object_size(dest, 1))
#endif

#if __has_builtin(__builtin___snprintf_chk)
	#define snprintf(dst, size, ...) __builtin___snprintf_chk(dst, size, 0, __builtin_object_size(dst, 1), __VA_ARGS__)
#endif
#if __has_builtin(__builtin___vsnprintf_chk)
	#define vsnprintf(dst, size, fmt, ap) __builtin___vsnprintf_chk(dst, size, 0, __builtin_object_size(dst, 1), fmt, ap)
#endif

#endif // __cplusplus
#endif // !WITH_HOST_LIBC

/* ctype */
int LIBC_FUNC(isalnum)(int c);
int LIBC_FUNC(isalpha)(int c);
int LIBC_FUNC(iscntrl)(int c);
int LIBC_FUNC(isdigit)(int c);
int LIBC_FUNC(isgraph)(int c);
int LIBC_FUNC(islower)(int c);
int LIBC_FUNC(isprint)(int c);
int LIBC_FUNC(ispunct)(int c);
int LIBC_FUNC(isspace)(int c);
int LIBC_FUNC(isupper)(int c);
int LIBC_FUNC(isxdigit)(int c);
int LIBC_FUNC(isascii)(int c);
int LIBC_FUNC(toascii)(int c);

unsigned char LIBC_FUNC(tolower)(unsigned char c);
unsigned char LIBC_FUNC(toupper)(unsigned char c);

/* errno */
#if !WITH_HOST_LIBC
extern int errno;
#include <errno.h>
#endif

/* numbers */
long LIBC_FUNC(strtol)(const char *nptr, char **endptr, int base);
unsigned long LIBC_FUNC(strtoul)(const char *nptr, char **endptr, int base);
unsigned long long LIBC_FUNC(strtoull)(const char * __restrict nptr, char ** __restrict endptr, int base);
int LIBC_FUNC(atoi)(const char *nptr);

/* stdlib */
int LIBC_FUNC(system)(const char *command);
void LIBC_FUNC(exit)(int status);
void LIBC_FUNC(qsort)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void LIBC_FUNC(srand)(unsigned int seed);
int LIBC_FUNC(rand)(void);

#if !WITH_HOST_LIBC
uint32_t swap32(uint32_t x);
uint16_t swap16(uint16_t x);

/* ntohl and friends */
uint32_t LIBC_FUNC(htonl)(uint32_t hostlong);
uint16_t LIBC_FUNC(htons)(uint16_t hostshort);
uint32_t LIBC_FUNC(ntohl)(uint32_t netlong);
uint16_t LIBC_FUNC(ntohs)(uint16_t netshort);
#endif

__END_DECLS
#endif
