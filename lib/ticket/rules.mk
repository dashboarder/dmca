# Copyright (C) 2010-2011, 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

OPTIONS		+= WITH_TICKET=1
LIBRARY_MODULES	+= lib/ticket
MODULES		+= lib/blockdev \
		   lib/env \
		   lib/image/image3 \
		   lib/pki

