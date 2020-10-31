# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBPKI_DIR	:=	$(GET_LOCAL_DIR)
LIBPKI_BUILD	:=	$(call TOLIBDIR,$(LIBPKI_DIR)/LIBPKI.a)
COMMONLIBS	+=	LIBPKI

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

GLOBAL_INCLUDES += $(LIBPKI_DIR)

LIBPKI_OBJS	:= \
	$(LIBPKI_DIR)/libDER/DER_CertCrl.o \
	$(LIBPKI_DIR)/libDER/DER_Decode.o \
	$(LIBPKI_DIR)/libDER/DER_Digest.o \
	$(LIBPKI_DIR)/libDER/DER_Encode.o \
	$(LIBPKI_DIR)/libDER/DER_Keys.o \
	$(LIBPKI_DIR)/libDER/oids.o \
	$(LIBPKI_DIR)/libGiants/giantExternal.o \
	$(LIBPKI_DIR)/libGiants/giantIntegers.o \
	$(LIBPKI_DIR)/libGiants/giantMemutils.o \
	$(LIBPKI_DIR)/libGiants/giantPort_C.o \
	$(LIBPKI_DIR)/libGiants/giantMod.o \
	$(LIBPKI_DIR)/libgRSA/rsaGiantKey.o \
	$(LIBPKI_DIR)/libgRSA/libgRSA.o \
	$(LIBPKI_DIR)/libgRSA/rsaPadding.o \
	$(LIBPKI_DIR)/libgRSA/libgRSA_priv.o \
	$(LIBPKI_DIR)/libgRSA/libgRSA_DER.o \
	$(LIBPKI_DIR)/libgRSA/rsaRawKey.o


LIBPKI_OBJS	:=	$(call TOLIBOBJDIR,$(LIBPKI_OBJS))

$(LIBPKI_BUILD):	$(LIBPKI_OBJS)

ALL_DEPS	+=	$(LIBPKI_OBJS:%o=%d)

endif
