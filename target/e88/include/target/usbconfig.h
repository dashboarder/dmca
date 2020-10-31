/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#ifdef USBPHY_UOTGTUNE1

#undef USBPHY_UOTGTUNE1
#define USBPHY_UOTGTUNE1		(0x749)

#endif /* USBPHY_UOTGTUNE1 */

#ifdef USBPHY_UOTGTUNE2

#undef USBPHY_UOTGTUNE2
#define USBPHY_UOTGTUNE2		(0x2FF3)

#endif /* USBPHY_UOTGTUNE2 */


#endif /* __TARGET_USBCONFIG_H */
