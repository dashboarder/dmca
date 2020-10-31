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
#include <platform/timer.h>
#include <arch.h>
#include <debug.h>
#include <sys/task.h>
#include <drivers/csi.h>
#include <drivers/a7iop/a7iop.h>

#include "csi_private.h"
#include "endpoints/crashlog_ep.h"

#include <csi_ipc_protocol.h>
#include <csi_frame.h>
#include <csi_crashlog_defines.h>


//
// Global Data
//

static ep_crashlog_desc_t *crashlog_descs[CSI_COPROC_MAX] = {NULL};

//
// Private functions
//

int   ep_crashlog_task     (void *arg);
void  crashlog_msg_handler (ep_crashlog_desc_t *clog, msg_payload_t msg);

csi_status_t  cl_print                (ep_crashlog_desc_t *clog);
void          cl_print_register_frame (CrashLogDataHeader_t *section);
void          cl_print_os_info        (CrashLogDataHeader_t *section);
void          cl_print_string         (CrashLogDataHeader_t *section);
void          cl_print_mailbox        (CrashLogDataHeader_t *section);
void          cl_print_call_stack     (CrashLogDataHeader_t *section);

static const char* get_signature_string (CrashLogSignature_t sig);


//
// bi_ep_crashlog_create
//
void bi_ep_crashlog_create (csi_coproc_t which_coproc)
{
  // enforce single instance
  REQUIRE(crashlog_descs[which_coproc]==NULL);
  crashlog_descs[which_coproc] = malloc(sizeof(ep_crashlog_desc_t));
  REQUIRE(crashlog_descs[which_coproc]!=NULL);

  crashlog_descs[which_coproc]->coproc = which_coproc;

  task_start(task_create("csi ep crashlog", ep_crashlog_task, crashlog_descs[which_coproc], kTaskStackSize_endpoints));
}


csi_status_t
crashlog_quiesce (csi_coproc_t which_coproc, csi_power_state_t ps)
{
  ep_crashlog_desc_t *clog = crashlog_descs[which_coproc];

  // free all shared memory if we are not preserving it
  if (!(ps & CSI_PS_SHARED_PRESERVED) && (NULL != clog->buffer)) {
    csi_free_shared_memory(clog->buffer);
    clog->buffer    = NULL;
    clog->allocated = 0;
  }

  return CSI_STATUS_OK;
}


//
// bi_ep_crashlog_force_event
//
// force a crashlog generation through various means
//
csi_status_t
bi_ep_crashlog_force_event (csi_coproc_t which_coproc, csi_crashlog_t crashlog_type)
{
  if (crashlog_descs[which_coproc]==NULL) {
    return CSI_STATUS_UNAVAILABLE;
  }

  switch (crashlog_type) {
    default:
      return CSI_STATUS_ERROR;

    case CSI_CRASHLOG_TYPE_NMI:
      akf_send_nmi(KFW(which_coproc));
      break;

    case CSI_CRASHLOG_TYPE_MSG_CRASH:
    case CSI_CRASHLOG_TYPE_MSG_LOG:
      return bi_ep_send_message (crashlog_descs[which_coproc]->csi_token, (msg_payload_t)((crashlog_type==CSI_CRASHLOG_TYPE_MSG_CRASH) ? CL_NMI_MSG_CRASH : CL_NMI_MSG_LOG));
      break;
  }

  return CSI_STATUS_OK;
}


//
// Crashlog Message Handler
//

int
ep_crashlog_task(void *arg)
{
  csi_status_t    result;
  msg_payload_t   rcv_msg;

  ep_crashlog_desc_t *clog = (ep_crashlog_desc_t*)arg;

  event_init(&clog->msg_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

  result = csi_register_endpoint(clog->coproc, IPC_ENDPOINT_BUILT_IN_CRASHLOG, &clog->msg_event, &clog->csi_token, &clog->name);
  REQUIRE(result==CSI_STATUS_OK);

  clog->buffer = NULL;
  clog->allocated = 0;

  for (;;) {
    event_wait(&clog->msg_event);

    // empty the rcv queue
    while(csi_receive_message(clog->csi_token, &rcv_msg) == CSI_STATUS_OK) {
      crashlog_msg_handler(clog, rcv_msg);
    }
  }

  return 0;
}


//
// crashlog_msg_handler
//
// handle an asynchronous message from the coproc
//
// All message are using the buffer protocol.
// If a zero address is received this is a shared buffer allocation request and a new buffer
// physical address is returned.
// Otherwise this is interpreted as a crash log and it is displayed on the screen/saved for
// future use as appropriate
//
void
crashlog_msg_handler (ep_crashlog_desc_t *clog, msg_payload_t msg)
{
  uint64_t      paddr;
  ipc_msg_t     resp;
  csi_status_t  status;

  // the first message is a buffer request, identified by a NULL address in the upper portion
  // of the payload
  if (ipc_buff_msg_get_address(msg) == 0x00) {

    // allocate a console buffer of the requested size.
    clog->allocated = ipc_buff_msg_get_size(msg);
    clog->buffer = csi_allocate_shared_memory(clog->csi_token, clog->allocated);
    REQUIRE(clog->buffer!=NULL);
    paddr = mem_static_map_physical((uintptr_t)clog->buffer);
    CSI_EP_LOG(DEBUG_SPEW, clog->coproc, clog->name, "buffer request of size %u bytes, allocated @ physical 0x%llx (AP %p)\n",
               clog->allocated, paddr, clog->buffer);

    // send the response back to the IOP
    ipc_buff_create_msg(&resp, paddr, clog->allocated);
    status = bi_ep_send_message (clog->csi_token, resp);
    if (CSI_STATUS_OK != status) {

    }

  } else {
    // A crash log has been received, decode it
    maybe_do_cache_operation(CACHE_INVALIDATE, clog->buffer, clog->allocated);
    status = cl_print(clog);
    if (CSI_STATUS_PANIC_RECOVER == status) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "Recoverable panic, reseting\n");
      status = csi_panic_recover (clog->coproc);
    }
    else if (status != CSI_STATUS_OK) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "data is corrupt or unrecognizable (%x)\n", status);
    }
  }
}


//
// *** Crashlog Decoder ***
//

#define RUN_IF_COMPATIBLE(version, version_expected, expr) \
    do { \
      if(LOG_MAJOR_VERSION(version) == LOG_MAJOR_VERSION(version_expected)) \
        expr; \
      else \
        CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "Not executing " #expr " because version %u.%u is not compatible with expected version %u.%u\n", \
                   LOG_PRINT_VERSION(version), LOG_PRINT_VERSION(version_expected)); \
    } while(0)

static const char*
get_exc_type_str(uint16_t exc_type)
{
  switch (exc_type & CL_TYPE_MASK) {
    case CL_ARM_EXCEPTION_UNDEF:            return "UNDEF exception";
    case CL_ARM_EXCEPTION_PREFETCH_ABORT:   return "PREFETCH ABORT";
    case CL_ARM_EXCEPTION_DATA_ABORT:       return "DATA ABORT";
    case CL_KERNEL_PANIC:                   return "PANIC";
    case CL_NMI:                            return "NMI FIQ";
    case CL_NMI_MSG_LOG:                    return "forced log msg";
    case CL_NMI_MSG_CRASH:                  return "forced crash msg";
    case CL_STACK_GUARD_FAULT:              return "Stack guard fault";
    case CL_FLOW_ERROR:                     return "Mailbox error";
    case CL_PANIC_RECOVERY:                 return "Recoverable PANIC";
  }

  return NULL;
}

//
// cl_print
//
// Decode and print the crash log using the registered section decoders.
// Make sure the section version match the supported version before
// decoding them.
//
csi_status_t
cl_print (ep_crashlog_desc_t *clog)
{
    uint8_t           *cl_buffer;
    CrashLogHeader_t  *cl_header;
    uint32_t          cl_size;
    CrashLogHeader_t  *end_header;
    const char        *cl_type_str;
    uint8_t           *log;

    cl_buffer = clog->buffer;
    cl_header = (CrashLogHeader_t*)cl_buffer;
    cl_size    = cl_header->size - sizeof (CrashLogHeader_t);

    // validate the signature
    if (CRASH_LOG_HEADER_SIGNATURE != cl_header->signature) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "invalid log signature: 0x%x, should be 0x%x.\n", cl_header->signature, CRASH_LOG_HEADER_SIGNATURE);
      return CSI_STATUS_ERROR;
    }

    // ensure the version is compatible
    if (LOG_MAJOR_VERSION(CRASH_LOG_VERSION) != LOG_MAJOR_VERSION(cl_header->version)) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "invalid log major version: %u, should be %u.\n",
                 LOG_MAJOR_VERSION(cl_header->version), LOG_MAJOR_VERSION(CRASH_LOG_VERSION));
      return CSI_STATUS_ERROR;
    }

    // If this is neither a partial nor nested log make sure the header at
    // the beginning of the log match the one at the end.
    // Issue a warning if they don't and still try to decode the log.
    end_header = (CrashLogHeader_t*)(cl_buffer + cl_size);

    if (cl_header->exception & CL_LOG_INITIAL_FLAG) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "partial log, log might be incomplete\n");
      // revert size to the one in the header, as we don't know what is at the end.
      cl_size = cl_header->size;
    }
    else if (memcmp (cl_header, end_header, sizeof (CrashLogHeader_t))) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "end header does not match log header, log might be corrupted (cl_size=%d %p %p)\n", cl_size, cl_header, end_header);
      // revert size to the one in the header, as we don't know what is at the end.
      cl_size = cl_header->size;
    }

    // validate the checksum
    // TBD

#if !RELEASE_BUILD
    // Print the log
    cl_type_str = get_exc_type_str(cl_header->exception);
    if (cl_type_str != NULL) {
      CSI_EP_LOG(CRASHLOG, clog->coproc, clog->name, "%s\n", cl_type_str);
    } else {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "crash log of unexpected type 0x%0x\n", cl_header->exception);
    }
    if (cl_header->exception & CL_LOG_NESTED_FLAG) {
      CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "a nested %s occured @ 0x%08x\n",
                 get_exc_type_str(cl_header->nestedException), cl_header->nestedLr);
    }

    // Decode the data section
    // A duplicate log header is placed at the end of the data section, ignore it.
    log = (cl_buffer + sizeof (CrashLogHeader_t));

    while (log < (uint8_t*)end_header) {
      CrashLogDataHeader_t *header = (CrashLogDataHeader_t*)log;
      switch (header->signature) {
        case CRASH_LOG_DATA_REGISTER_SIGNATURE:
          RUN_IF_COMPATIBLE(header->version, CRASH_LOG_DATA_REGISTER_VERSION, cl_print_register_frame(header));
          break;
        case CRASH_LOG_DATA_OS_INFO_SIGNATURE:
          RUN_IF_COMPATIBLE(header->version, CRASH_LOG_DATA_OS_INFO_VERSION, cl_print_os_info(header));
          break;
        case CRASH_LOG_DATA_STRING_SIGNATURE:
          RUN_IF_COMPATIBLE(header->version, CRASH_LOG_DATA_STRING_VERSION, cl_print_string(header));
          break;
        case CRASH_LOG_DATA_MAILBOX_SIGNATURE:
          RUN_IF_COMPATIBLE(header->version, CRASH_LOG_DATA_MAILBOX_VERSION, cl_print_mailbox(header));
          break;
        case CRASH_LOG_DATA_CALL_STACK_SIGNATURE:
          RUN_IF_COMPATIBLE(header->version, CRASH_LOG_DATA_CALL_STACK_VERSION, cl_print_call_stack(header));
          break;
        default:
          CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "found unknown %s section of size %d, version %x.%x\n",
                  get_signature_string(header->signature), header->size, LOG_PRINT_VERSION(header->version));
          break;
      }
      log += header->size;
    }
#else
    // silence all warnings
    (void)cl_type_str;
    (void)log;
#endif

    // if we received a recoverable panic reset the IOP
    if (CL_PANIC_RECOVERY == cl_header->exception) {
      // if we are not already in panic recovery and one was requested try it
      if (!csi_is_panic_recovery(clog->coproc)) {
        csi_set_feature (clog->coproc, COPROC_FEATURE_PANIC_RECOVERY);
        return CSI_STATUS_PANIC_RECOVER;
      } else {
        CSI_EP_LOG(CRITICAL, clog->coproc, clog->name, "failed panic recovery, hanging...\n");
      }
    }
    return CSI_STATUS_OK;
}


//
// *** Register section ***
//

typedef struct _fsr_entry_t {
    const uint32_t value;
    const char* const reason;
} fsr_entry_t;

static const fsr_entry_t fsr_short_descriptor_table[] = { // From DDI0406C Table B3-23: VMSAv7 Short-descriptor format FSR encodings
    {1, "Alignment fault"},
    {4, "Instruction cache maintenance fault"},
    {12, "Translation table walk synchronous external abort (1st level)"},
    {14, "Translation table walk synchronous external abort (2nd level)"},
    {28, "Translation table walk synchronous parity error (1st level)"},
    {30, "Translation table walk synchronous parity error (2nd level)"},
    {5, "Translation fault (section)"},
    {7, "Translation fault (page)"},
    {3, "Access Flag fault (section)"},
    {6, "Access Flag fault (page)"},
    {9, "Domain fault (section)"},
    {11, "Domain fault (page)"},
    {13, "Permission fault (section)"},
    {15, "Permission fault (page)"},
    {2, "Debug event"},
    {8, "Synchronous external abort"},
    {16, "TLB conflict abort"},
    {20, "Implementation defined (Lockdown)"},
    {26, "Implementation defined (Coprocessor abort)"},
    {25, "Memory access synchronous parity error"},
    {22, "Asynchronous external abort"},
    {24, "Memory access asynchronous parity error"},
    {(uint32_t)-1, "Unknown"} // Terminated by value = (unsigned int)-1, which is never a valid encoding
};

static const fsr_entry_t fsr_long_descriptor_table[] = { // From DDI0406C Table B3-24: VMSAv7 Long-descriptor format FSR encodings
    {4, "Translation fault level %u"},
    {8, "Access flag fault level %u"},
    {12, "Permission fault level %u"},
    {20, "Synchronous external abort on translation table walk level %u"},
    {28, "Synchronous parity error on memory access on translation table walk level %u"},
    {60, "Domain fault level %u"},
    {16, "Synchronous external abort"},
    {24, "Synchronous parity error on memory access"},
    {17, "Asynchronous external abort"},
    {25, "Asynchronous parity error on memory access"},
    {33, "Alignment fault"},
    {34, "Debug event"},
    {48, "TLB conflict abort"},
    {52, "Implementation defined (Lockdown)"},
    {58, "Implementation defined (Coprocessor abort)"},
    {(uint32_t)-1, "Unknown"} // Terminated by value = (unsigned int)-1, which is never a valid encoding
};


static const char*
cl_decode_fsr(uint32_t fsr_val)
{
    // holder for the processed lookup level FSR entries
    static char fsr_status_string[80];

    uint32_t fault_status;
    uint32_t fault_lookup_level = 0;
    uint32_t fault_status_ll = 0;

    const fsr_entry_t *fsr_table;

    // Check the LPAE bit to see if this is short or long descriptor
    if ((fsr_val & (1u<<9)) == 0 )
    {
        // Non-LPAE, using short-descriptor format
        fault_status = ((fsr_val & (1u<<10)) >> 6 ) | (fsr_val & 0xf); // [10, 3:0]
        fsr_table = fsr_short_descriptor_table;
    }
    else
    {
        // LPAE, using long-descriptor format
        fault_status = (fsr_val & 0x3f);        // [5:0]
        fault_lookup_level = (fsr_val & 0x03);  // [1:0]
        fault_status_ll = (fsr_val & 0x3c);     // [5:2]
        fsr_table = fsr_long_descriptor_table;
    }

    while (1)
    {
        // if a match for non-lookup level entry or the terminator
        if( (fault_status == fsr_table->value)
            || (fault_status_ll == fsr_table->value)
            || (fsr_table->value == (uint32_t)-1) )
        {
            snprintf(fsr_status_string, sizeof(fsr_status_string), fsr_table->reason, fault_lookup_level);
            return fsr_status_string;
        }

        fsr_table++;
    }

    // Control should never reach here since fsr tables are terminated by value = -1
    return NULL;
}


//
// cl_print_register_frame
//
// Decode and print the register frame section
//
void
cl_print_register_frame (CrashLogDataHeader_t *section)
{
  CrashLogRegisterSection_t  *regSection = (CrashLogRegisterSection_t*) section;

  CSI_LOG_RAW(CRASHLOG, "Failing task register frame:\n");
  CSI_LOG_RAW(CRASHLOG, "  Faulting PC=0x%08x\n", regSection->registerFrame.pc);

  // print additional information register data
  switch (regSection->exception) {
      case CL_ARM_EXCEPTION_PREFETCH_ABORT:
        CSI_LOG_RAW(CRASHLOG, "  ifsr=0x%08x (%s)  ifar=0x%08x\n",
                              regSection->registerFrame.extra_info1, cl_decode_fsr(regSection->registerFrame.extra_info1),
                              regSection->registerFrame.extra_info2);
          break;

      case CL_STACK_GUARD_FAULT:
      case CL_ARM_EXCEPTION_DATA_ABORT:
        CSI_LOG_RAW(CRASHLOG, "  dfsr=0x%08x (%s)  dfar=0x%08x\n",
                              regSection->registerFrame.extra_info1, cl_decode_fsr(regSection->registerFrame.extra_info1),
                              regSection->registerFrame.extra_info2);
          break;

      default:
          break;
  }

  // print faulting task registers
  CSI_LOG_RAW(CRASHLOG, "  r00=0x%08x  r01=0x%08x  r02=0x%08x  r03=0x%08x\n"
                        "  r04=0x%08x  r05=0x%08x  r06=0x%08x  r07=0x%08x\n"
                        "  r08=0x%08x  r09=0x%08x  r10=0x%08x  r11=0x%08x\n"
                        "  r12=0x%08x   sp=0x%08x   lr=0x%08x   pc=0x%08x\n"
                        "  spsr=0x%08x\n",
                        regSection->registerFrame.r0, regSection->registerFrame.r1, regSection->registerFrame.r2, regSection->registerFrame.r3,
                        regSection->registerFrame.r4, regSection->registerFrame.r5, regSection->registerFrame.r6, regSection->registerFrame.r7,
                        regSection->registerFrame.r8, regSection->registerFrame.r9, regSection->registerFrame.r10, regSection->registerFrame.r11,
                        regSection->registerFrame.r12, regSection->registerFrame.sp, regSection->registerFrame.lr, regSection->registerFrame.pc,
                        regSection->registerFrame.spsr);
}


//
// *** OS Section ***
//

const char*
cl_decode_task_status(uint32_t status)
{
    if (status & 0x100) return "INACTIVE";
    if (status & 0x080) return "QUEUE_WAIT";
    if (status & 0x040) return "SEMAPHORE_WAIT";
    if (status & 0x020) return "MSG_WAIT";
    if (status & 0x010) return "BLOCK_WAIT";
    if (status & 0x008) return "RESOURCE_WAIT";
    if (status & 0x004) return "DELAY_WAIT";
    if (status & 0x002) return "PARTITION_WAIT";
    if ((status & 0xffe) == 0x000) return "READY";
    if ((status & 0x001) == 0x001) return "SUSPENDED";
    return "unknown";
}

//
// cl_print_os_info
//
// Decode the OS section
//
void
cl_print_os_info (CrashLogDataHeader_t *section)
{
  CrashLogRtxcSection_t  *rtxcSection = (CrashLogRtxcSection_t*) section;

  // display the current task list
  CSI_LOG_RAW(CRASHLOG, "RTXC Task List:\n"
                        "        name        | pri |     lr     | stack use | status\n");

  CrashLogRtxcTask_t  *taskLog = (CrashLogRtxcTask_t*)((uint8_t*)(section) + sizeof (CrashLogRtxcSection_t));

  for (int i = 0; i < (int)rtxcSection->numTask; i++) {
    // add a '*' in front of the hipri task
    if (taskLog->id == rtxcSection->taskId) {
      CSI_LOG_RAW(CRASHLOG, "*");
    } else {
      CSI_LOG_RAW(CRASHLOG, " ");
    }

    CSI_LOG_RAW(CRASHLOG, "%2d %-15s | %03d | 0x%08x | %04u/%04u | %s\n",
                          taskLog->id, taskLog->name, taskLog->priority,
                          taskLog->lr, taskLog->stackUsed, taskLog->stackSize,
                          cl_decode_task_status(taskLog->status));
    taskLog++;
  }
}


//
// *** string section ***
//

void
cl_print_string (CrashLogDataHeader_t *section)
{
  // this section is a header with a NULL terminated string
  CrashLogStringSection_t  *stringSection = (CrashLogStringSection_t*) section;

  // make sure the string is NULL terminated, force it if not
  char   *str = &(stringSection->data.string);
  size_t len  = section->size - sizeof (CrashLogStringSection_t) - 1; // length of the string without the null termination

  if ('\0' != str[len]) {
      str[len] = '\0';
  }

  // print the section
  CSI_LOG_RAW(CRASHLOG, "%s\n", str);
}


//
// *** Mailbox Section ***
//

void
cl_print_mailbox (CrashLogDataHeader_t *section)
{
  CrashLogMailboxData_t *mailSection = &(((CrashLogMailboxSection_t*)section)->data);

  if (CL_MAILBOX_NO_ERROR == mailSection->error) {
    CSI_LOG_RAW(CRASHLOG, "Mailbox status:\n");
  } else {
    CSI_LOG_RAW(CRASHLOG, "Mailbox status: (error = 0x%x\n", mailSection->error);
  }
  CSI_LOG_RAW(CRASHLOG, "  Inbox  AKF_KIC_INBOX_CTRL = 0x%08x, AKF_KIC_MAILBOX_SET = 0x%08x\n"
                        "  Outbox AKF_AP_OUTBOX_CTRL = 0x%08x, AKF_AP_MAILBOX_SET  = 0x%08x\n",
                        mailSection->inboxCtrl, mailSection->inboxInt,
                        mailSection->outboxCtrl, mailSection->outboxInt);
}


//
// *** Call Stack section ***
//

static const char *
get_call_stack_error_string(uint32_t error_code)
{
  const char *error_string;

  switch (error_code) {
    default:
    case CL_CALL_STACK_NO_ERROR:
      error_string = "";
      break;
    case CL_CALL_STACK_OVERFLOW:
      error_string = "error: stack too big for crash log";
      break;
    case CL_CALL_STACK_FP_NULL:
      error_string = "error: fp is null => possible stack corruption";
      break;
    case CL_CALL_STACK_FP_INVALID:
      error_string = "error: fp is invalid => possible stack corruption";
      break;
  }

  return error_string;
}


//
// cl_print_call_stack
//
// Print the call stack of a task.
// There is one of these section per task.
//
void
cl_print_call_stack (CrashLogDataHeader_t *section)
{
  bool                        isAPanic;
  CrashLogCallStackSection_t  *callStackSection;
  uint32_t                    *callStack;

  callStackSection = (CrashLogCallStackSection_t*) section;
  isAPanic = ((callStackSection->panickingFuncIndex>0) && (callStackSection->panickingFuncIndex<callStackSection->stackDepth));
  callStack = (uint32_t*)(callStackSection+1);

  CSI_LOG_RAW(CRASHLOG, "Task %2u Call Stack:%s%s - ",
                        callStackSection->taskId,
                        (isAPanic?" (@=>panicking function)":""),
                        get_call_stack_error_string(callStackSection->error));

  for ( uint32_t j=0; j<callStackSection->stackDepth; j++ ) {
    CSI_LOG_RAW(CRASHLOG, " %s0x%08x", ((isAPanic&&(j==callStackSection->panickingFuncIndex))?"@":" "), *callStack++);
  }

  CSI_LOG_RAW(CRASHLOG, "\n");
}


//
// Private utility function
//

const char*
get_signature_string (CrashLogSignature_t sig)
{
  static char  sig_str[5];
  // signature are 4 ascii bytes packed into a 32 bit word
  // extract and print
  sig_str[0] = (char)((sig >> 24) & 0x00FF);
  sig_str[1] = (char)((sig >> 16) & 0x00FF);
  sig_str[2] = (char)((sig >> 8) & 0x00FF);
  sig_str[3] = (char)(sig & 0x00FF);
  sig_str[4] = (char)('\0');

  return sig_str;
}
