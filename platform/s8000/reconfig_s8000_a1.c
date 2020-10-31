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

// Gross hack to get around the fact that the header files from DV don't have unique
// names for the A1 and B0 versions of the sequence
#define RECONFIG_S8000_A1 1
#include "reconfig.c"
