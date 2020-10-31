/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef TARGET_BOARDID_H
#define TARGET_BOARDID_H

#define TARGET_BOARD_ID_J98AP		(12)
#define TARGET_BOARD_ID_J98DEV		(13)
#define TARGET_BOARD_ID_J99AP		(4)
#define TARGET_BOARD_ID_J99DEV		(5)

// Board revs
#define	J98_DEV1_BOARD_REV		(0x0)
#define	J98_DEV2_BOARD_REV		(0x1)
// board revs on AP are reversed
// <rdar://problem/16314440> J99: Display timing selection on Form Factor is incorrect
#define	J99_PROTO0_BOARD_REV_LOCAL	(0xE)
#define	J99_PROTO0_BOARD_REV_CHINA	(0xD)
#define	J99_PROTO1_BOARD_REV_LOCAL	(0xC)
#define	J99_PROTO1_BOARD_REV_CHINA	(0xB)
#define	J99_PROTO2_BOARD_REV		(0xA)
#define	J99_PROTO3_BOARD_REV		(0x9)
#define	J99_PROTO3_P9_BOARD_REV		(0x8)
#define	J99_EVT_BOARD_REV		(0x7)
#define	J99_EVT_PLUS_BOARD_REV		(0x6)
#define	J99_EVT2_BOARD_REV		(0x5)

#endif
