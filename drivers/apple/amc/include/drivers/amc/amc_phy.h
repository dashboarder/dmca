/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_AMC_PHY_H
#define __DRIVERS_AMC_PHY_H

#include <sys/types.h>

/* XXX TODO: cleanup 'int' to what's needed */
struct amc_phy_calibration_data {
	int		bit_time_ps;
        int		min_ca_window_width_ps;
        int		min_dq_window_width_ps;
        int		ca_pivot_step_size_ps;
        int		dq_pivot_step_size_ps;
        int		max_offset_setting;
        int		min_offset_factor;
        int		dll_f2_delay_fs;
        uint8_t		ca_pivots_list_size;
        int		*ca_pivot_ps;
        uint8_t		dq_pivots_list_size;
        int		*dq_pivot_ps;
        bool	use_resume_boot_flow;
};

void amc_phy_preinit(void);
void amc_phy_init(bool resume);
void amc_phy_pre_normal_speed_enable();
void amc_phy_scale_dll(int freqsel, int factor);
bool amc_phy_rddq_cal();
void amc_phy_bypass_prep(int step);
void amc_phy_finalize();
const struct amc_phy_calibration_data *amc_phy_get_calibration_data();
void amc_phy_calibration_init(uint32_t *fine_lock_ps);
void amc_phy_run_dll_update(uint8_t ch);
void amc_phy_set_addr_offset(uint8_t ch, uint8_t value);
void amc_phy_set_rd_offset(uint8_t ch, uint8_t byte, uint8_t value);
void amc_phy_set_wr_offset(uint8_t ch, uint8_t byte, uint8_t value);
void amc_phy_configure_dll_pulse_mode(uint8_t ch, bool enable);
void amc_phy_run_dq_training(uint8_t ch);
uint32_t amc_phy_get_dq_training_status(uint8_t ch, uint8_t byte, uint8_t num_bytes);
void amc_phy_reset_fifos();
void amc_phy_enable_dqs_pulldown(bool enable);

void amc_phy_restore_calibration_values(bool resume);
void amc_phy_calibration_restore_ca_offset(uint8_t ch);
void amc_phy_calibration_ca_rddq_cal(bool resume);
void amc_phy_calibration_wrdq_cal(bool resume);
void amc_phy_shift_dq_offset(int8_t *dq_offset, uint8_t num_bytes);

#endif /* __DRIVERS_AMC_PHY_H */
