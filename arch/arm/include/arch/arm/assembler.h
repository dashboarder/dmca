/*
 * Copyright (c) 2011 Apple Inc.  All rights reserved.	
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


/*
 * Declare a globally-visible ARM function
 */
.macro ARM_FUNCTION
	.text
	.arm
	.balign 4
	.global $0
$0:
.endmacro

/*
 * Declare a globally-visible THUMB function
 */
.macro THUMB_FUNCTION
	.text	
	.thumb_func $0
	.balign 2
	.global $0
$0:
.endmacro
	
