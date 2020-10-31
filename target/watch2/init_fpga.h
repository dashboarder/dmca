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
#ifndef _TARGET_INIT_FPGA_H
#define _TARGET_INIT_FPGA_H

bool fpga_target_should_recover(void);
bool fpga_target_should_poweron(bool *cold_button_boot);
bool fpga_target_should_poweroff(bool at_boot);
void fpga_target_setup_default_environment(void);
int fpga_target_update_device_tree(void);

#endif /* ! _TARGET_INIT_FPGA_H */
