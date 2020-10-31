# Copyright (C) 2007-2010 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBC_DIR	:= $(GET_LOCAL_DIR)
LIBC_BUILD	:= $(call TOLIBDIR,$(LIBC_DIR)/LIBC.a)
COMMONLIBS	+= LIBC

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

# base library files
LIBC_OBJS := \
	$(LIBC_DIR)/atoi.o \
	$(LIBC_DIR)/ctype.o \
	$(LIBC_DIR)/endian.o \
	$(LIBC_DIR)/log2.o \
	$(LIBC_DIR)/memcpy_chk.o \
	$(LIBC_DIR)/memmove_chk.o \
	$(LIBC_DIR)/memset_chk.o \
	$(LIBC_DIR)/misc.o \
	$(LIBC_DIR)/printf.o \
	$(LIBC_DIR)/printf_chk.o \
	$(LIBC_DIR)/qsort.o \
	$(LIBC_DIR)/stdio.o \
	$(LIBC_DIR)/stdlib.o \
	$(LIBC_DIR)/strlcat_chk.o \
	$(LIBC_DIR)/strlcpy_chk.o \
	$(LIBC_DIR)/strtol.o \
	$(LIBC_DIR)/strtoll.o \
	$(LIBC_DIR)/strtoul.o \
	$(LIBC_DIR)/strtoull.o

# handle architecture-specific overrides for string functions
ARCH_STROPS :=
-include $(LIBC_DIR)/$(ARCH)/rules.mk

STROPS := bcopy bzero memchr memcmp memcpy memmove memscan memset strchr\
	strcmp strcoll strdup strlcat strlcpy strlen strncmp \
	strnicmp strnlen strpbrk strrchr strsep strspn strstr strtok strxfrm

# filter out the strops that the arch code doesn't already specify
STROPS := $(filter-out $(ARCH_STROPS),$(STROPS))
STROPS_FILES := $(addsuffix .o,$(addprefix $(LIBC_DIR)/,$(STROPS)))

LIBC_OBJS += \
	$(STROPS_FILES)

LIBC_OBJS := $(call TOLIBOBJDIR,$(LIBC_OBJS))

$(LIBC_BUILD):	$(LIBC_OBJS)

ALL_DEPS += $(LIBC_OBJS:%o=%d)

endif
