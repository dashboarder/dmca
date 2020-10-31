/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if SUB_PLATFORM_S8001
#include <target/dcsfixup_s8001.h>
#elif SUB_PLATFORM_S8003
#include <target/dcsfixup_s8003.h>
#else
#include <target/dcsfixup_s8000.h>
#endif
