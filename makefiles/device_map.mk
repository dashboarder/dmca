# Copyright (C) 2010-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

################################################################################
# Macros for interacting with the device map database.
#

ifeq ($(DEVICEMAP),)
$(error Cannot locate embedded_device_map - device map queries will fail.)
endif

# Oh, makefile! This is the only way to escape a comma in some cases.
COMMA				:= ,

# Transform a whitespace separated list to a comma separated list of
# double-quoted items.
# Example input:	iBoot iBSS iBEC iBoot
# Example output:	"iBoot", "iBSS", "iBEC", "iBoot"
# Method:
# 1) Enclose each item in <"item",>
# 2) Terminate with <X>
# 3) Remove trailing <,X>
MAKE_SQL_LIST			= \
	$(subst $(COMMA)X,,$(foreach i,$(1),\"$(i)\"$(COMMA))X)

#
# Locate the device map database; this prevents EDM having to locate the 
# platform every time it runs.
#
DEVICEMAP_DATABASE		?=	$(PLATFORMROOT)/usr/local/standalone/firmware/device_map.db
DEVICEMAP_DBOPT			:=	-db $(DEVICEMAP_DATABASE)

#
# Returns the set of TARGET values known to the map.
#
DEVICEMAP_TARGETS		= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT TargetType		\
						FROM Targets			\
						GROUP BY TargetType)

#
# Returns the set of PLATFORM values associated with at least one TARGET.
#
DEVICEMAP_PLATFORMS		= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT DISTINCT Platform		\
						FROM Targets)

#
# Returns the set of PRODUCT values defined for the TARGET passed in $1
#
# Note that this may include product types that we can't generate, so it
# needs to be post-filtered to remove unknown products.
#
DEVICEMAP_PRODUCTS_FOR_TARGET	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT FileType			\
						FROM Files			\
						  INNER JOIN Manifests USING \(manifestID\)	\
						  INNER JOIN Targets USING \(Target\)		\
						WHERE TargetType == \"$(1)\"	\
						GROUP BY FileType)

#
# Returns the chip ID value defined for the TARGET passed in $1
#
DEVICEMAP_CHIPID_FOR_TARGET	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT ChipID			\
						FROM Targets			\
						WHERE TargetType == \"$(1)\"	\
						GROUP BY ChipID)

#
# Returns the epoch value defined for the TARGET passed in $1
#
DEVICEMAP_EPOCH_FOR_TARGET	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT SecurityEpoch		\
						FROM Targets			\
						WHERE TargetType == \"$(1)\"	\
						GROUP BY SecurityEpoch)

#
# Returns the crypto hash method  defined for the TARGET passed in $1
#
DEVICEMAP_CRYPTO_HASH_FOR_TARGET = $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT CryptoHashMethod		\
						FROM Targets			\
						WHERE TargetType == \"$(1)\"	\
						GROUP BY CryptoHashMethod)

#
# Returns the epoch value defined for the TARGET passed in $1
#
DEVICEMAP_EPOCH_FOR_PLATFORM	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT DISTINCT SecurityEpoch	\
						FROM Targets			\
						WHERE Platform == \"$(1)\"	\
						GROUP BY SecurityEpoch)

#
# Returns the crypto hash method defined for the TARGET passed in $1
#
DEVICEMAP_CRYPTO_HASH_FOR_PLATFORM = $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT DISTINCT CryptoHashMethod	\
						FROM Targets			\
						WHERE Platform == \"$(1)\"	\
						GROUP BY CryptoHashMethod)

#
# Returns the product id value defined for the TARGET passed in $1
#
DEVICEMAP_PRODUCTID_FOR_TARGET	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT ProductID			\
						FROM Targets			\
						WHERE TargetType == \"$(1)\"	\
						GROUP BY ProductID)

#
# Returns the image format defined for the PLATFORM passed in $1
#
DEVICEMAP_IMGFMT_FOR_PLATFORM	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT DISTINCT ImageFormat	\
						FROM Targets			\
						WHERE Platform == \"$(1)\"	\
				   )

#
# Returns the image format defined for the TARGET passed in $1
#
DEVICEMAP_IMGFMT_FOR_TARGET	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-query SELECT DISTINCT ImageFormat	\
						FROM Targets			\
						WHERE TargetType == \"$(1)\"	\
				   )

#
# Returns a series for target:imageformat:epoch:productid sequences for
# all targets, use the macros below to look up keys in the result
#
DEVICEMAP_TGT_DICT		= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-separator :				\
					-query SELECT DISTINCT			\
						TargetType, ImageFormat, SecurityEpoch, ProductID, CryptoHashMethod \
						FROM Targets			\
				   )

DEVICEMAP_TGT_DICT_IMGFMT	= $(word 2,$(subst :, ,$(filter $(2):%,$(1))))
DEVICEMAP_TGT_DICT_EPOCH	= $(word 3,$(subst :, ,$(filter $(2):%,$(1))))
DEVICEMAP_TGT_DICT_PRODUCTID	= $(word 4,$(subst :, ,$(filter $(2):%,$(1))))
DEVICEMAP_TGT_DICT_CRYPTOHASH	= $(word 5,$(subst :, ,$(filter $(2):%,$(1))))

#
# Returns a series for target:product:payloadtype tuples for
# all target/product pairs, use the macro below to look up keys in the result
#
DEVICEMAP_TGT_PRODUCT_DICT	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
					-separator :				\
					-query SELECT DISTINCT			\
					TargetType, fileType, PayloadType	\
					FROM Files				\
					INNER JOIN Manifests USING \(manifestID\)	\
					INNER JOIN Targets USING \(Target\)	\
					WHERE fileType IN \($(call MAKE_SQL_LIST,$(1))\) \
				   )

DEVICEMAP_TGT_PRODUCT_DICT_PAYLOADTYPE	= $(word 3,$(subst :, ,$(filter $(2):$(3):%,$(1))))

#
# Returns the PayloadType value defined for object specified by
#  PRODUCT passed in $1
#  TARGET passed in $2
#
DEVICEMAP_PAYLOADTYPE_FOR_OBJECT	= $(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT)	\
						-query SELECT DISTINCT PayloadType	\
							FROM Files			\
							  INNER JOIN Manifests USING \(manifestID\)	\
							  INNER JOIN Targets USING \(Target\)		\
							WHERE fileType == \"$(1)\"	\
							AND TargetType == \"$(2)\"	\
							GROUP BY PayloadType		\
					   )

#
# Returns the set of TARGET values which have at least one firmware file.
# This can be used to elide targets with no firmware.
#
# PRODUCT list is passed in $1, e.g iBoot iBSS iBEC iBoot
#
DEVICEMAP_TARGETS_FOR_PRODUCTS	= \
	$(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT) -query		\
		SELECT DISTINCT TargetType			\
		FROM Files					\
		  INNER JOIN Manifests USING \(manifestID\)	\
		  INNER JOIN Targets USING \(Target\)		\
		WHERE fileType IN \($(call MAKE_SQL_LIST,$(1))\))

#
# Returns all TARGET-PRODUCT-BUILD tuples where:
#
# TargetType is in list $1 (TARGETS)
# FileType is in list $2 (PRODUCTS)
# BuildStyle is in list $3 (BUILDS)
#
DEVICE_MAP_TPB_FILTERED		= \
	$(shell $(DEVICEMAP) $(DEVICEMAP_DBOPT) -query		\
		SELECT DISTINCT TargetType, FileType, BuildStyle \
		FROM Files					\
		  INNER JOIN Manifests USING \(manifestID\)	\
		  INNER JOIN Targets USING \(Target\)		\
		WHERE \
		  TargetType IN \($(call MAKE_SQL_LIST,$(1))\) AND \
		  FileType IN \($(call MAKE_SQL_LIST,$(2))\) AND \
		  BuildStyle IN \($(call MAKE_SQL_LIST,$(3))\) \
		| sed "s/|/-/g")
