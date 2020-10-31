# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

# Format a trivial function
#
# Input records are formatted as:
#
# $2 : return type
# $3 : function name
# $4 : argument list
# $5 : return value
#
# Caller is expected to set FUNCTION and TEMPLATE
#

BEGIN {
	FS = "[ \t]*:[ \t]*"
}

$1=="#" {
	next
}

$3==FUNCTION {
    if ($2 == "void") {
	cmd = sprintf("sed -e \'s/TYPE/%s/g\' -e \'s/FUNCTION/%s/g\' -e \'s/ARGLIST/%s/g\' -e \'s/RETVAL/%s/g\' -e \'s/RET//g\'  %s", $2, $3, $4, $5, TEMPLATE)
    } else {
	cmd = sprintf("sed -e \'s/TYPE/%s/g\' -e \'s/FUNCTION/%s/g\' -e \'s/ARGLIST/%s/g\' -e \'s/RETVAL/%s/g\' -e \'s/RET/return/g\' %s", $2, $3, $4, $5, TEMPLATE)
    }
    system(cmd)
}
