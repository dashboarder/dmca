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

static inline
void dcs_init_config_fixup_params(dcs_config_params_t *dcs_params, dcs_target_t targtype)
{
	dcs_params->chnldec              = 0x00050110;
};
