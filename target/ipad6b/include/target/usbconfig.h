/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_USBCONFIG_H
#define __TARGET_USBCONFIG_H

#ifdef USBPHY_OTGTUNE0

#undef USBPHY_OTGTUNE0
#define USBPHY_OTGTUNE0		(0x37377B93)

#endif /* USBPHY_OTGTUNE0 */

#ifdef USBPHY_OTGTUNE1

#undef USBPHY_OTGTUNE1
#define USBPHY_OTGTUNE1		(0x20C04)

#endif /* USBPHY_OTGTUNE1 */


#endif /* __TARGET_USBCONFIG_H */
