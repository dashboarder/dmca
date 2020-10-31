# Copyright (C) 2008, 2014 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

OPTIONS		+= 	WITH_NVRAM=1

MODULES		+= 	\
			lib/cksum \
			lib/env \
			lib/heap

LIBRARY_MODULES	+=	lib/nvram

