# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS += \
	WITH_NET=1

ALL_OBJS += \
	$(LOCAL_DIR)/arp.o \
	$(LOCAL_DIR)/callbacks.o \
	$(LOCAL_DIR)/ethernet.o \
	$(LOCAL_DIR)/icmp.o \
	$(LOCAL_DIR)/ipv4.o \
	$(LOCAL_DIR)/mbuf.o \
	$(LOCAL_DIR)/net.o \
	$(LOCAL_DIR)/udp.o \
	$(LOCAL_DIR)/xp.o \
	$(LOCAL_DIR)/debug.o
