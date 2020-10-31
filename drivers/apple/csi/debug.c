/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <platform.h>
#include <arch.h>
#include <debug.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <drivers/csi.h>
#include <drivers/a7iop/a7iop.h>

#include <platform/timer.h>
#include <platform/soc/hwregbase.h>

#include "csi_private.h"
#include "endpoints/management_ep.h"
#include "endpoints/syslog_ep.h"
#include "endpoints/console_ep.h"
#include "endpoints/crashlog_ep.h"


#if defined(WITH_MENU) && WITH_MENU

static int csi_debug_main(int argc, struct cmd_arg *args);

MENU_COMMAND_DEBUG(csi, csi_debug_main, "Test CSI driver", NULL);

static void usage() {
  printf("usage: csi ans|<other iop> <command>\n"
         "  info\n"
         "  crashlog|cl nmi|ml|mc\n"
         "  ping [count]\n"
         "  syslog|sl [1|0|on|off]\n"
         "  state|st\n"
         "  quiesce|qs\n"
         "  hibernate|hb\n"
         "  resume|rs\n"
         "  powerstate|ps [ps]\n"
         "  console ...\n"
         "          ... v <verbosity>\n"
         "          ... c <cmd arg>\n"
         "  timebase|tb\n"
         "  stimeout|sto [timeout in us]\n");
}


static inline uint64_t read_akf_timebase(uint64_t regs_base)
{
  uint32_t hi1 = rAKF_KIC_GLB_TIME_BASE_HI(regs_base);
  uint32_t lo  = rAKF_KIC_GLB_TIME_BASE_LO(regs_base);
  uint32_t hi2 = rAKF_KIC_GLB_TIME_BASE_HI(regs_base);
  return ((((uint64_t)hi2) << 32) | ((hi1!=hi2) ? rAKF_KIC_GLB_TIME_BASE_LO(regs_base) : lo));
}


static int
csi_debug_main (int argc, struct cmd_arg *args)
{
  csi_coproc_t  which_coproc;
  const char    *s;
  csi_status_t  status;
  uint64_t      akf_base_addr = 0;

  if (argc < 3) {
    usage();
    return -1;
  }

  //
  // Find the targeted coproc
  //

  s = args[1].str;
  if (strnicmp(s, "ans", strlen(s)) == 0) {
    which_coproc = CSI_COPROC_ANS;
    akf_base_addr = ANS_AKF_BASE_ADDR;
  } else if (strnicmp(s, "sep", strlen(s)) == 0) {
    which_coproc = CSI_COPROC_SEP;
#if ASEP_AKF_BASE_ADDR
    akf_base_addr = ASEP_AKF_BASE_ADDR;
#endif
  } else if (strnicmp(s, "isp", strlen(s)) == 0) {
    which_coproc = CSI_COPROC_ISP;
#if ISP_AKF_BASE_ADDR
    akf_base_addr = ISP_AKF_BASE_ADDR;
#endif
  } else if (strnicmp(s, "sio", strlen(s)) == 0) {
    which_coproc = CSI_COPROC_SIO;
#if ASIO_AKF_BASE_ADDR
    akf_base_addr = ASIO_AKF_BASE_ADDR;
#endif
  } else {
    printf("csi: %s unknown\n", s);
    usage();
    return -1;
  }

  if (akf_base_addr == 0) {
    printf("csi: %s not supported\n", IOP_NAME(which_coproc));
    usage();
    return -1;
  }

  //
  // Parse command
  //

  status = CSI_STATUS_OK;
  s      = args[2].str;

  // information
  if (strnicmp(s, "info", strlen(s)) == 0) {
    csi_info (which_coproc);
    return 0;
  }

  // crashlog
  if ((strnicmp(s, "crashlog", strlen(s)) == 0) || (strcmp(s, "cl") == 0)) {
    csi_crashlog_t   cl;

    if (argc < 4) {
      usage();
      return -1;
    }

    s = args[3].str;
    if (strnicmp(s, "nmi", strlen(s)) == 0) {
      cl = CSI_CRASHLOG_TYPE_NMI;
    } else if (strnicmp(s, "ml", strlen(s)) == 0) {
      cl = CSI_CRASHLOG_TYPE_MSG_LOG;
    } else if (strnicmp(s, "mc", strlen(s)) == 0) {
      cl = CSI_CRASHLOG_TYPE_MSG_CRASH;
    } else {
      usage();
      return -1;
    }

    printf("csi: force crashlog from %s iop\n", IOP_NAME(which_coproc));
    status = bi_ep_crashlog_force_event(which_coproc, cl);

  }
  // ping
  else if (strnicmp(s, "ping", strlen(s)) == 0) {

    int   ping_count = 1;

    if (argc >= 4) {
      ping_count = atoi(args[3].str);
      if (ping_count<=0) {
        ping_count = 1;
      }
    }

    status = bi_ep_mgmt_send_ping(which_coproc, ping_count);

  }
  // syslog
  else if ((strnicmp(s, "syslog", strlen(s)) == 0) || (strcmp(s, "sl") == 0)) {

    if (argc < 4) {
      usage();
      return -1;
    }

    status = bi_ep_syslog_verbosity(which_coproc, atoi(args[3].str));

  }
  // state
  else if ((strnicmp(s, "state", strlen(s)) == 0) || (strcmp(s, "st") == 0)) {

    mgmt_state_t       state = bi_ep_mgmt_get_state(which_coproc);
    csi_power_state_t  ps    = bi_ep_mgmt_get_power_state(which_coproc);

    printf("csi: %s state - %s - ps=0x%x\n", IOP_NAME(which_coproc), COPROC_STATE(state), ps);

  }
  // quiesce
  else if ((strnicmp(s, "quiesce", strlen(s)) == 0) || (strcmp(s, "qs") == 0)) {

    status = quiesce_iop (which_coproc);

  }
  // start
  else if ((strnicmp(s, "hibernate", strlen(s)) == 0) || (strcmp(s, "hb") == 0)) {

    status = hibernate_iop (which_coproc);

  }
  // start
  else if ((strnicmp(s, "resume", strlen(s)) == 0) || (strcmp(s, "rs") == 0)) {

    // restart the IOP
    status = csi_start (which_coproc);

  }
  // powerstate
  else  if ((strnicmp(s, "powerstate", strlen(s)) == 0) || (strcmp(s, "ps") == 0)) {

    if (argc < 4) {
      printf("powerstate requires a powerstate argument\n");
      return -1;
    }

    csi_power_state_t  ps = strtol(args[3].str, (char **)NULL, 0);
    status = request_iop_power_state (which_coproc, ps);

  }
  // console
  else  if (strnicmp(s, "console", strlen(s)) == 0) {

    if (argc != 5) {
      usage();
      return -1;
    }

    if (args[3].str[0] == 'v') {
      status = bi_ep_console_set_verbosity(which_coproc, strtol(args[4].str, (char **)NULL, 0));
    } else if (args[3].str[0] == 'c') {
      status = bi_ep_console_send_custom_cmd(which_coproc, strtol(args[4].str, (char **)NULL, 0));
    } else {
      usage();
      return -1;
    }
  }
  // timebase
  else  if ((strnicmp(s, "timebase", strlen(s)) == 0) || (strcmp(s, "tb") == 0)) {

    uint64_t  tb_host_1, tb_host_2, tb_iop_1, tb_iop_2;

    tb_host_1 = timer_get_ticks();
    tb_iop_1  = read_akf_timebase(akf_base_addr);
    tb_iop_2  = read_akf_timebase(akf_base_addr);
    tb_host_2 = timer_get_ticks();

    printf("csi: host vs %s timebase\n", IOP_NAME(which_coproc));
    printf("\thost:\t 0x%llx\n", tb_host_1);
    printf("\tiop:\t 0x%llx (%llu)\n", tb_iop_1, (tb_iop_1-tb_host_1));
    printf("\tiop:\t 0x%llx (%llu)\n", tb_iop_2, (tb_iop_2-tb_iop_1));
    printf("\thost:\t 0x%llx (%llu)\n", tb_host_2, (tb_host_2-tb_iop_2));
  }
  // send timeout
  else if ((strnicmp(s, "stimeout", strlen(s)) == 0) || (strcmp(s, "sto") == 0)) {

    if (argc < 4) {
      printf("current send timeout for %s: %uus\n", IOP_NAME(which_coproc), get_send_timeout(which_coproc));
      return 0;
    }

    uint32_t  timeout = strtol(args[3].str, (char **)NULL, 0);
    if (0 == timeout) {
      // the A7 express no timeout as a wait forever.
      timeout = A7IOP_WAIT_FOREVER;
      printf("csi: %s send timeout disabled (%u)\n", IOP_NAME(which_coproc), timeout);
    } else {
      printf("csi: setting %s send timeout to %uus\n", IOP_NAME(which_coproc), timeout);
    }

    status = set_send_timeout (which_coproc, timeout);
  }
  // unknown command
  else {
    printf("unknown command %s\n", s);
    usage();
    return -1;
  }

  if (status != CSI_STATUS_OK) {
    printf("csi %s %s - error reported: %s\n", IOP_NAME(which_coproc), args[2].str, CSI_STATUS_STR(status));
  }

  return 0;
}

#endif // defined(WITH_MENU) && WITH_MENU
