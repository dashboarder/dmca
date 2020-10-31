# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

# Print the function field from lines with tags matching TAG

BEGIN {
	FS = "[ \t]*:[ \t]*"
}

$1==TAG {
	printf("%s ", $3)
}
