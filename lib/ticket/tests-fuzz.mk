# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

TEST_NAME := ticket-fuzz

TEST_OBJS := \
	$(LOCAL_DIR)/ticket.o \
	$(LOCAL_DIR)/DERApTicket.o \
	lib/image/image.o \
	lib/image/image3/Image3.o \
	lib/image/image3/image3_wrapper.o \
	lib/pki/chain-validation.o \
	lib/pki/libDER/DER_CertCrl.o \
	lib/pki/libDER/DER_Decode.o \
	lib/pki/libDER/DER_Digest.o \
	lib/pki/libDER/DER_Encode.o \
	lib/pki/libDER/DER_Keys.o \
	lib/pki/libDER/oids.o \
	lib/pki/libGiants/giantExternal.o \
	lib/pki/libGiants/giantIntegers.o \
	lib/pki/libGiants/giantMemutils.o \
	lib/pki/libGiants/giantPort_C.o \
	lib/pki/libGiants/giantMod.o \
	lib/pki/libgRSA/rsaGiantKey.o \
	lib/pki/libgRSA/libgRSA.o \
	lib/pki/libgRSA/rsaPadding.o \
	lib/pki/libgRSA/libgRSA_priv.o \
	lib/pki/libgRSA/libgRSA_DER.o \
	lib/pki/libgRSA/rsaRawKey.o \
	drivers/sha1/sha1.o \
	drivers/sha1/mozilla_sha.o

TEST_SUPPORT_OBJS += \
	lib/blockdev/blockdev.o \
	lib/blockdev/mem_blockdev.o \
	lib/cksum/crc32.o \
	lib/env/env.o \
	lib/mib/mib.o \
	lib/libc/misc.o \
	lib/libc/log2.o \
	tests/fuzz-main.o \
	$(LOCAL_DIR)/fuzz.o

TEST_INCLUDES += \
	lib/pki

TEST_CFLAGS += \
	-DTICKET_UNITTEST=1 \
	-DWITH_PKI=1 \
	-DPKI_CHECK_ANCHOR_BY_SHA1=0 \
	-DPKI_APPLE_ROOT_CA=1 \
	-DPKI_CHECK_KEY_IDS=1 \
	-DWITH_IMAGE3=1 \
	-DIMAGE_MAX_COUNT=1 \
	-DCPU_CACHELINE_SIZE=64 \
	-DIMAGE3_NO_CRC=1

