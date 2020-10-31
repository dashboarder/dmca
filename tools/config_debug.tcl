#!/usr/local/bin/astris
# -*-Tcl-*-

# Read out cfg engine state and debug registers

set soc_cfg_error [mem -memap 4 0x210480000]
puts [format "SOC CFG ERROR(0x210480000)      0x%x" $soc_cfg_error]

set wr_resp_err_in_flight [expr ($soc_cfg_error>>24) & 0x3F]
set wr_resp_err [expr ($soc_cfg_error>>20) & 0x3]
set rd_timeout_err [expr ($soc_cfg_error>>16) & 0x1]
set rd_resp_err [expr ($soc_cfg_error>>12) & 0x3]
set unknown_cmd_err [expr ($soc_cfg_error>>8) & 0x1]
set spec_access_err [expr ($soc_cfg_error>>4) & 0x3]
set table_access_err [expr $soc_cfg_error & 0x3]

if {$wr_resp_err_in_flight} {puts [format "\twr_resp_err_in_flight: 0x%x" $wr_resp_err_in_flight]}
if {$wr_resp_err} {puts [format "\twr_resp_err:           0x%x" $wr_resp_err]}
if {$rd_timeout_err} {puts [format "\trd_timeout_err:        0x%x" $rd_timeout_err]}
if {$rd_resp_err} {puts [format "\trd_resp_err:           0x%x" $rd_resp_err]}
if {$unknown_cmd_err} {puts [format "\tunknown_cmd_err:       0x%x" $unknown_cmd_err]}
if {$spec_access_err} {puts [format "\tspec_access_err:       0x%x" $spec_access_err]}
if {$table_access_err} {puts [format "\ttable_access_err:      0x%x" $table_access_err]}

set last_fetch_rd_addr [mem -memap 4 0x21048000c]
set last_proc_rd_addr [mem -memap 4 0x210480010]
set last_proc_wr_addr [mem -memap 4 0x210480014]
puts [format "last_fetch_rd_addr(0x21048000c)  0x%x" $last_fetch_rd_addr]
puts [format "last_proc_rd_addr(0x210480010)  0x%x" $last_proc_rd_addr]
puts [format "last_proc_wr_addr(0x210480014)  0x%x" $last_proc_wr_addr]


set aop_state [mem -memap 4 0x210000008]
puts [format "AOP STATE(0x210000008)          0x%x" $aop_state]
set pmu_if [expr $aop_state & 0x1F]
if {$pmu_if == 0x0} {puts "\tRESET - cold boot wait state until minipmgr_aop_awake_ack_sync"}
if {$pmu_if == 0x1} {puts "\tAWAKE - in the AWAKE state"}
if {$pmu_if == 0x2} {puts "\tAWAKE_exit_delay - counter delay before exiting from AWAKE"}
if {$pmu_if == 0x3} {puts "\tAWAKE_exit_pre - executing preamble"}
if {$pmu_if == 0x4} {puts "\tAWAKE_exit_HS - handshake with miniPMGR"}
if {$pmu_if == 0x5} {puts "\tAWAKE_exit_post - executing postamble"}
if {$pmu_if == 0x6} {puts "\tAWAKE_exit_PMU - handshake with PMU"}
if {$pmu_if == 0x7} {puts "\tOFF - in the OFF state"}
if {$pmu_if == 0x8} {puts "\tS2R_NOAOP - in the S2R_NOAOP state"}
if {$pmu_if == 0x9} {puts "\tAOP_DDR - in the AOP_DDR state"}
if {$pmu_if == 0xA} {puts "\tS2R_AOP - in the S2R_AOP state"}
if {$pmu_if == 0xB} {puts "\tgoAWAKE_rst - heading from AOP_DDR to AWAKE, waiting for AWAKE_RESETN"}
if {$pmu_if == 0xC} {puts "\tgoAWAKE_pre - preamble for AOP_DDR to AWAKE"}
if {$pmu_if == 0xD} {puts "\tgoAWAKE_HS - handshake with miniPMGR"}
if {$pmu_if == 0xE} {puts "\tgoAWAKE_post - postamble for AOP_DDR to AWAKE"}
if {$pmu_if == 0xF} {puts "\tgoS2R_pre - preamble for AOP_DDR to S2R_AOP"}
if {$pmu_if == 0x10} {puts "\tgoS2R_HS - handshake with miniPMGR"}
if {$pmu_if == 0x11} {puts "\tgoS2R_post - postamble for AOP_DDR to S2R_AOP"}
if {$pmu_if == 0x12} {puts "\tgoS2R_PMU - handshake with PMU"}
if {$pmu_if == 0x13} {puts "\tgoDDR_pre - preamble for S2R_AOP to AOP_DDR"}
if {$pmu_if == 0x14} {puts "\tgoDDR_PMU - handshake with PMU"}
if {$pmu_if == 0x15} {puts "\tgoDDR_HS - handshake with miniPMGR"}
if {$pmu_if == 0x16} {puts "\tgoDDR_post - postamble for S2R_AOP to AOP_DDR"}

#soc cfg
set table_base [mem -memap 4 0x210000200]
set cfg_table_address [expr $table_base | 0x210e00000]

set s2r_aop_to_aop_ddr [mem -memap 4 0x210000204]
set s2r_aop_to_aop_ddr_pre_enable [expr ($s2r_aop_to_aop_ddr>>8) & 0x1]
set s2r_aop_to_aop_ddr_pre_index [expr ($s2r_aop_to_aop_ddr>>0) & 0x1F]
set s2r_aop_to_aop_ddr_post_enable [expr ($s2r_aop_to_aop_ddr>>24) & 0x1]
set s2r_aop_to_aop_ddr_post_index [expr ($s2r_aop_to_aop_ddr>>16) & 0x1F]

set aop_ddr_to_s2r_aop [mem -memap 4 0x210000208]
set aop_ddr_to_s2r_aop_pre_enable [expr ($aop_ddr_to_s2r_aop>>8) & 0x1]
set aop_ddr_to_s2r_aop_pre_index [expr ($aop_ddr_to_s2r_aop>>0) & 0x1F]
set aop_ddr_to_s2r_aop_post_enable [expr ($aop_ddr_to_s2r_aop>>24) & 0x1]
set aop_ddr_to_s2r_aop_post_index [expr ($aop_ddr_to_s2r_aop>>16) & 0x1F]

set aop_ddr_to_awake [mem -memap 4 0x21000020c]
set aop_ddr_to_awake_pre_enable [expr ($aop_ddr_to_awake>>8) & 0x1]
set aop_ddr_to_awake_pre_index [expr ($aop_ddr_to_awake>>0) & 0x1F]
set aop_ddr_to_awake_post_enable [expr ($aop_ddr_to_awake>>24) & 0x1]
set aop_ddr_to_awake_post_index [expr ($aop_ddr_to_awake>>16) & 0x1F]

set awake_to_aop_ddr [mem -memap 4 0x210000210]
set awake_to_aop_ddr_pre_enable [expr ($awake_to_aop_ddr>>8) & 0x1]
set awake_to_aop_ddr_pre_index [expr ($awake_to_aop_ddr>>0) & 0x1F]
set awake_to_aop_ddr_post_enable [expr ($awake_to_aop_ddr>>24) & 0x1]
set awake_to_aop_ddr_post_index [expr ($awake_to_aop_ddr>>16) & 0x1F]

puts [format "table_base(0x210000200)         0x%x" $table_base]
puts [format "\tConfig Table Address:         0x%x" $cfg_table_address]	
puts "Config table entries"
md32 -memap 4 $cfg_table_address 40

puts [format "awake_to_aop_ddr(0x210000210)   0x%x" $awake_to_aop_ddr]
puts [format "\tpre enable                    0x%x" $awake_to_aop_ddr_pre_enable]
puts [format "\tpre index                     0x%x" $awake_to_aop_ddr_pre_index]
puts [format "\tpost enable                   0x%x" $awake_to_aop_ddr_post_enable]
puts [format "\tpost index                    0x%x" $awake_to_aop_ddr_post_index]
puts [format "aop_ddr_to_s2r_aop(0x210000208) 0x%x" $aop_ddr_to_s2r_aop]
puts [format "\tpre enable                    0x%x" $aop_ddr_to_s2r_aop_pre_enable]
puts [format "\tpre index                     0x%x" $aop_ddr_to_s2r_aop_pre_index]
puts [format "\tpost enable                   0x%x" $aop_ddr_to_s2r_aop_post_enable]
puts [format "\tpost index                    0x%x" $aop_ddr_to_s2r_aop_post_index]
puts [format "s2r_aop_to_aop_ddr(0x210000204) 0x%x" $s2r_aop_to_aop_ddr]
puts [format "\tpre enable                    0x%x" $s2r_aop_to_aop_ddr_pre_enable]
puts [format "\tpre index                     0x%x" $s2r_aop_to_aop_ddr_pre_index]
puts [format "\tpost enable                   0x%x" $s2r_aop_to_aop_ddr_post_enable]
puts [format "\tpost index                    0x%x" $s2r_aop_to_aop_ddr_post_index]
puts [format "aop_ddr_to_awake(0x21000020c)   0x%x" $aop_ddr_to_awake]
puts [format "\tpre enable                    0x%x" $aop_ddr_to_awake_pre_enable]
puts [format "\tpre index                     0x%x" $aop_ddr_to_awake_pre_index]
puts [format "\tpost enable                   0x%x" $aop_ddr_to_awake_post_enable]
puts [format "\tpost index                    0x%x" $aop_ddr_to_awake_post_index]

puts "Trace Entry"
mem -memap 4 0x210E9E500
