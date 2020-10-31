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
#if CONFIG_FPGA
#include <target/pinconfig_fpga.h>
#elif CONFIG_SIM
#include <target/pinconfig_sim.h>
#else
#include <target/pinconfig_product.h>
#endif
