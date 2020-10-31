# Copyright (C) 2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

#
# C++ support is enabled by including this module.  It affects both
# the library dependency list (by adding libc++) and also the build
# of the actual application.
#

WITH_CPLUSPLUS		:=	true
OPTIONS			+=	WITH_CPLUSPLUS=1
LIBRARY_MODULES		+=	lib/libc++
