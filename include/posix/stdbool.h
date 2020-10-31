/*
 * Copyright (C) 2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef _STDBOOL_H_
#define _STDBOOL_H_

#if !defined(__cplusplus)

#define false			0
#define true			1

#define bool			_Bool
#if __STDC_VERSION__ < 199901L && __GNUC__ < 3
typedef int			_Bool;
#endif

#endif /* !defined(__cplusplus) */

#endif /* _STDBOOL_H_ */