# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

# Makefile for build/install-time tools

include makefiles/macros.mk

HOST_ARCH	:=	$(shell uname -m)
BHC		:=	$(call TOTOOLSDIR,bhc)
MACHO2BIN	:=	$(call TOTOOLSDIR,Macho2Bin)
BOOTX2BIN	:=	$(call TOTOOLSDIR,BootX2Bin)

# Host libImg4Decode.a is *sometimes* installed to your root filesystem.
# Sometimes it's in your platform. Who knows. Who cares. Let's try a
# few places because I'm sick of filling in TPS reports to get it fixed.

HOST_IMG4_SEARCH := \
		$(PLATFORMROOT)/usr/local \
		$(PLATFORMROOT)/../../../usr/local \
		/usr/local

HOST_IMG4_FOUND := \
		$(firstword $(foreach path,\
			$(HOST_IMG4_SEARCH),\
			$(if $(realpath $(path)/include/amd/Img4Decode.h),\
				$(realpath $(path)))))

HOST_IMG4_INCLUDE	:= $(HOST_IMG4_FOUND)/include/amd/Img4Decode.h
HOST_IMG4_LIB		:= $(HOST_IMG4_FOUND)/lib/amd/libImg4Decode.a
ifeq ($(and $(realpath $(HOST_IMG4_INCLUDE)),$(realpath $(HOST_IMG4_LIB))),)
$(info Host Image4 installation broken. Something is missing.)
HOST_IMG4_FOUND :=
endif

ifneq ($(HOST_IMG4_FOUND),)
$(info Host Image4 installation found under $(HOST_IMG4_FOUND))
else
$(info Host Image4 installation not found, omitting support)
endif

BOOTX2BIN_INCLUDES	:= . include lib/pki
BOOTX2BIN_PREBUILT	:=
BOOTX2BIN_FLAGS		:= -Wall -O -g -Wformat -DDEBUG

# Optional Image4 support.
ifneq ($(HOST_IMG4_FOUND),)
BOOTX2BIN_FLAGS		+= -DBOOTX2BIN_WITH_IMAGE4=1
BOOTX2BIN_INCLUDES	+= $(HOST_IMG4_FOUND)/include/amd
BOOTX2BIN_PREBUILT	+= $(HOST_IMG4_LIB)
endif

# Add includes to flags.
BOOTX2BIN_FLAGS		+= $(foreach inc,$(BOOTX2BIN_INCLUDES),-I$(inc))

build-tools:	destdir $(BHC) $(MACHO2BIN) $(BOOTX2BIN)

install-tools:	destdir $(BHC) $(MACHO2BIN) $(BOOTX2BIN)

clean-tools:
	@rm -f $(BHC)
	@rm -f $(MACHO2BIN)
	@rm -f $(BOOTX2BIN)
	@rm -rf $(TOOLS_BIN)

destdir:
	$(v_)-mkdir -p $(TOOLS_BIN)


$(BHC):		$(SRCROOT)/tools/bhc.c
	@echo HOST_CC $@
	$(_v)${HOST_CC} -isysroot $(HOST_SDKROOT) -o $@ $<

$(MACHO2BIN):	$(SRCROOT)/tools/Macho2Bin.c
	@echo HOST_CC $@
	$(_v)${HOST_CC} -isysroot $(HOST_SDKROOT) -o $@ $<

$(call TOTOOLSDIR,%.o):	$(SRCROOT)/%.c
	@echo HOST_CC $@
	@mkdir -p $(dir $@)
	$(_v)${HOST_CC} -isysroot $(HOST_SDKROOT) -c -o $@ $< $(BOOTX2BIN_FLAGS)

$(call TOTOOLSDIR,%.o):	$(SRCROOT)/%.cpp
	@echo HOST_CPP $@
	@mkdir -p $(dir $@)
	$(_v)${HOST_CPP} -isysroot $(HOST_SDKROOT) -c -o $@ $< $(BOOTX2BIN_FLAGS)

LIBDER_SRCS	:= \
		lib/pki/libDER/DER_CertCrl.c \
		lib/pki/libDER/DER_Decode.c \
		lib/pki/libDER/DER_Digest.c \
		lib/pki/libDER/DER_Encode.c \
		lib/pki/libDER/DER_Keys.c \
		lib/pki/libDER/oids.c

BOOTX2BIN_SRCS	:= \
		tools/BootX2Bin.cpp \
		tools/Buffer.cpp \
		tools/DeviceTreePatcher.cpp \
		tools/Kernelcache.cpp \
		tools/LoadMachO.cpp \
		drivers/sha1/mozilla_sha.c \
		drivers/sha1/sha1.c \
		lib/cksum/adler32.c \
		lib/devicetree/devicetree.c \
		lib/image/image3/Image3.c \
		lib/lzss/lzss.c \
		$(LIBDER_SRCS) \
		$(NULL)
BOOTX2BIN_OBJS	:= $(patsubst %.c,$(call TOTOOLSDIR,%.o),$(BOOTX2BIN_SRCS))
BOOTX2BIN_OBJS	:= $(patsubst %.cpp,$(call TOTOOLSDIR,%.o),$(BOOTX2BIN_OBJS))

$(BOOTX2BIN):	$(BOOTX2BIN_OBJS) $(BOOTX2BIN_PREBUILT)
	@echo HOST_LD $@
	$(_v)${HOST_CPP} -isysroot $(HOST_SDKROOT) -o $@ $^
