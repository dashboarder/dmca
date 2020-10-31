/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <drivers/mcu.h>
#include <drivers/uart.h>
#include <target/uartconfig.h>
#include <limits.h>

//	TI MSP430F2350 MCU Serial Packet Format
//
//	0                   1
//	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +0
//	|      'S'      |      't'      |
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +2
//	|    OpCode     |    Seq No     |
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +4
//	| Length High   |  Length Low   |
//	+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+  +6
//	|                               |
//	|           <payload>           |
//	|                               |
//	+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//	|  CRC-16 High  |  CRC-16 Low   |
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Sequence number is even for MCU->CPU packets and odd for CPU->MCU packets.
// CRC is a CRC-16 CCITT (polynomial 0x1021, init value is 0xFFFF).

typedef uint8_t		MCUOpCode;

// Commands requiring acks.

#define kMCUOpCode_NoOp					0x00
#define kMCUOpCode_PowerOnReset			0x01
#define kMCUOpCode_GetVersion			0x02 // Response: <2:major>.<2:minor>[.<4:revision>]
#define kMCUOpCode_SILControl			0x04 // Request:  <1:action>
	
	#define kMCUSILAction_Invalid			0xFF
	
	#define kMCUSILAction_Off				0
	#define kMCUSILAction_OffStr			"off"
	
	#define kMCUSILAction_On				1
	#define kMCUSILAction_OnStr				"on"
	
	#define kMCUSILAction_SlowFlash			2
	#define kMCUSILAction_SlowStr			"slow"
	
	#define kMCUSILAction_FastFlash			3
	#define kMCUSILAction_FastStr			"fast"
	
	#define kMCUSILAction_1Blink			4
	#define kMCUSILAction_1BlinkStr			"1blink"
	
	#define kMCUSILAction_3Blinks			5
	#define kMCUSILAction_3BlinksStr		"3blinks"

#define kMCUOpCode_PairControl			0x05 // Request: <1:0=unpair, 1=pair> <1:remote UID>

	#define kMCUPairAction_Unpair			0x00
	#define kMCUPairAction_Pair				0x01

#define kMCUOpCode_BlueSteelControl		0x06 // Request: <1:mode> <1:subMode>

	#define kMCUBSControl_Invalid			0xff

	#define kMCUBSControl_Reset			0
	#define kMCUBSControl_ResetStr			"reset"
	#define kMCUBSControl_EnterTestMode		1
	#define kMCUBSControl_EnterTestModeStr		"testmode"
	#define kMCUBSControl_PassthroughMode		2
	#define kMCUBSControl_PassthroughModeStr	"passthru"
	#define kMCUBSControl_InfoFrameEnable		5
	#define kMCUBSControl_InfoFrameEnableStr	"info"

#define kMCUOpCode_SystemState			0x07 // Request: <1:state>
	
	#define kMCUSystemState_NoChange		0x00
	#define kMCUSystemState_Standby			0x01
	#define kMCUSystemState_Normal			0x02

#define kMCUOpCode_AppleIRData			0x10
#define kMCUOpCode_Repeat				0x11
#define kMCUOpCode_3rdPartyIRData		0x12

// Acks to commands.

#define kMCUOpCode_Ack					0x80
#define kMCUOpCode_NakUnknown			0x81
#define kMCUOpCode_NakUnsupported		0x82
#define kMCUOpCode_NakBadCRC			0x83
#define kMCUOpCode_NakNotNow			0x84
#define kMCUOpCode_NakMessageTooLong	0x85
#define kMCUOpCode_NakOther				0x9F

#define MCUOpCode_IsAckOrNak( X )					\
	( ( (X) == kMCUOpCode_Ack )					||	\
	  ( (X) == kMCUOpCode_NakUnknown )			||	\
	  ( (X) == kMCUOpCode_NakUnsupported )		||	\
	  ( (X) == kMCUOpCode_NakBadCRC )			||	\
	  ( (X) == kMCUOpCode_NakNotNow )			||	\
	  ( (X) == kMCUOpCode_NakMessageTooLong )	||	\
	  ( (X) == kMCUOpCode_NakOther ) )

// No response necessary.

#define kMCUOpCode_DebugData			0xFF

// Set to 1 to print text contents of MCU debug packets
#define PRINT_MCU_DEBUG				0

// Constants

#define kMCUResponseTimeout			4000000
#define kMCUTaskStackSize			8192
#define kMCUMaxRecvSize				32

#define kMCUBaudRate				250000
#define kMCUWordPeriodUS			(1000000 / (kMCUBaudRate/10))
#define kMCUFifoSize				1024

#define kMCUNotNowDelayUS			100000
#define kMCUNotNowAttempts			100
#define kMCUReadTimeoutUS			1000000
#define kMCUSendAttempts			5

#define kMCUFifoLatencyUS			(kMCUWordPeriodUS * kMCUFifoSize)
#define kMCUReadSleepUS				(kMCUFifoLatencyUS / 4)

struct McuRecvPacket {
    uint8_t opcode;
    uint8_t buf[kMCUMaxRecvSize];
    size_t len;
};

static bool gIsMCUInitialized = false;
static struct task_event mcu_gate;
static struct task_event mcu_packet_arrived;
static struct task_event mcu_packet_consumed;
static bool mcu_recv_valid;
static struct McuRecvPacket mcu_recv;

static int mcu_send_command(int opcode, uint8_t *data, size_t data_len);
static int mcu_set_sil(int action);
static int mcu_set_bs(int data0, int data1); // data1 == -1 if unused.
static int mcu_get_version(char *version, size_t length);

static int mcu_task(void *arg);
static int mcu_send_packet(MCUOpCode opcode, uint8_t *data, size_t length);
static int mcu_read_packet(MCUOpCode *outOpcode, uint8_t *outData, size_t maxLen, size_t *outLen);
static int mcu_read_response(MCUOpCode *outOpcode, uint8_t *outData, size_t maxLen, size_t *outLen);
static unsigned long crc_ccitt(unsigned long crc, unsigned char* p, unsigned long len);

int mcu_init(void)
{
    int ret = -1;
    char version[kMCUMaxRecvSize];
    
    uart_hw_init_extended(MCU_SERIAL_PORT, kMCUBaudRate, 8, PARITY_NONE, 1);
    ret = uart_hw_set_rx_buf(MCU_SERIAL_PORT, true, kMCUFifoSize);
    if (ret != 0) panic("mcu_init() uart_hw_set_rx_buf failed");

    // send break to resync stream
    uart_send_break(MCU_SERIAL_PORT, true);
    task_sleep(kMCUWordPeriodUS * 2);	// at least one word
    uart_send_break(MCU_SERIAL_PORT, false);

    event_init(&mcu_gate, EVENT_FLAG_AUTO_UNSIGNAL, true);
    event_init(&mcu_packet_arrived, EVENT_FLAG_AUTO_UNSIGNAL, false);
    event_init(&mcu_packet_consumed, EVENT_FLAG_AUTO_UNSIGNAL, false);
    task_start(task_create("mcu", &mcu_task, NULL, kMCUTaskStackSize));
    
    // first, retrieve MCU version string
    ret = mcu_get_version(version, sizeof(version));

    // constant slow-blink: device is booting
    if (ret == 0) {
        ret = mcu_set_sil(kMCUSILAction_SlowFlash);
    }

    // record and report whether MCU was successfully initialized
    gIsMCUInitialized = (ret == 0);
    if (gIsMCUInitialized) {
        dprintf(DEBUG_INFO, "MCU version: %s\n", version);
    } else {
        dprintf(DEBUG_CRITICAL, "mcu_init() failed\n");
    }
        
    return ret;
}

int mcu_start_recover(void)
{
    int ret = -1;

    event_wait(&mcu_gate);
    // constant fast-blink: plug into iTunes
    if (gIsMCUInitialized) {
        ret = mcu_set_sil(kMCUSILAction_FastFlash);
    }
    event_signal(&mcu_gate);
    
    return ret;
}

int mcu_start_boot(void)
{
    int ret = -1;

    event_wait(&mcu_gate);
    // constant slow-blink: device is booting
    if (gIsMCUInitialized) {
        ret = mcu_set_sil(kMCUSILAction_SlowFlash);
    }
    event_signal(&mcu_gate);

    return ret;
}

int mcu_set_passthrough_mode(bool on)
{
    int ret = -1;

    if (gIsMCUInitialized) {
	event_wait(&mcu_gate);
        if (on) {
            ret = mcu_set_bs(kMCUBSControl_EnterTestMode, -1);
	    if (ret != 0) goto exit;
	    ret = mcu_set_bs(kMCUBSControl_PassthroughMode, -1);
	    if (ret != 0) goto exit;
        } else {
            ret = mcu_set_bs(kMCUBSControl_Reset, -1);
        }
    exit:
	if (ret != 0) {
	    dprintf(DEBUG_CRITICAL, "mcu_set_passthrough_mode %d failed\n", on);
	}
	event_signal(&mcu_gate);
    }

    return ret;
}

int mcu_send_info_frames(bool on)
{
    int ret = -1;

    if (gIsMCUInitialized) {
	event_wait(&mcu_gate);
	ret = mcu_set_bs(kMCUBSControl_InfoFrameEnable, on ? 1 : 0);
	event_signal(&mcu_gate);
	if (ret != 0) {
	    dprintf(DEBUG_CRITICAL, "mcu_send_info_farmes %d failed\n", on);
	}
    }

    return ret;
}

int mcu_quiesce_uart(void)
{
    int ret = -1;

    ret = uart_hw_set_rx_buf(MCU_SERIAL_PORT, false, kMCUFifoSize);
    if (ret != 0)
        panic("uart can not be put into polling mode!");
    return ret;
}
static int mcu_send_command(int command, uint8_t *data, size_t data_len)
{
    int retries = kMCUSendAttempts;
    int not_now_retries = kMCUNotNowAttempts;
    int ret = -1;

    do {
	ret = mcu_send_packet(command, data, data_len);
	if (ret != 0) continue;
	
	uint8_t opcode;
	ret = mcu_read_response(&opcode, NULL, 0, NULL);
	if (ret != 0) continue;
	if (opcode != kMCUOpCode_Ack) ret = -1;
	if (opcode == kMCUOpCode_NakNotNow) {
	    // MCU isn't in the right state for this command. Allow
	    // more time than usual for this transient state to pass.
	    if (--not_now_retries > 0) {
		task_sleep(kMCUNotNowDelayUS);
		continue;
	    } else {
		break;
	    }
	}
    } while ((ret != 0) && (--retries > 0));

    return ret;
}

static int mcu_set_sil(int action)
{
    uint8_t data = action;
    return mcu_send_command(kMCUOpCode_SILControl, &data, 1);
}

static int mcu_set_bs(int data0, int data1)
{
    uint8_t buf[2] = { data0, data1 };
    return mcu_send_command(kMCUOpCode_BlueSteelControl, buf, data1 >= 0 ? 2 : 1);
}

static int mcu_get_version(char *version, size_t length)
{
    int ret;
    
    ret = mcu_send_packet(kMCUOpCode_GetVersion, NULL, 0);
    if (ret != 0) return ret;
    
    uint8_t opcode;
    size_t outLength;
    ret = mcu_read_response(&opcode, (uint8_t *)version, length, &outLength);
    if (ret != 0) return ret;
    if (opcode != kMCUOpCode_Ack) return -1;

    if (version) version[outLength-1] = 0;
    
    return 0;
}

static int mcu_task(void *arg)
{
    struct McuRecvPacket new_recv;
    bzero(&new_recv, sizeof(new_recv));
    for (;;) {
	int ret = mcu_read_packet(&new_recv.opcode,
				  new_recv.buf,
				  sizeof(new_recv.buf),
				  &new_recv.len);
	if (ret != 0) continue;
	// Process uninteresting packets here.
	if (new_recv.opcode == kMCUOpCode_DebugData) {
#if PRINT_MCU_DEBUG
	    size_t i;
	    dprintf(DEBUG_INFO, "mcu: \"");
	    for (i = 0; i < new_recv.len; ++i)
		dprintf(DEBUG_INFO, "%c", new_recv.buf[i]);
	    dprintf(DEBUG_INFO, "\"\n");
#endif
	    continue;
	}
	if (!MCUOpCode_IsAckOrNak(new_recv.opcode)) continue;
	// Signal packet arrival.
	memcpy(&mcu_recv, &new_recv, sizeof(mcu_recv));
	mcu_recv_valid = true;
	event_signal(&mcu_packet_arrived);
	// Wait for packet consumption.
	while (mcu_recv_valid) {
	    event_wait(&mcu_packet_consumed);
	}
    }
    return 0;
}

static int mcu_send_packet(MCUOpCode opcode, uint8_t *payload, size_t length)
{
    if (length > 0xFFFF) return -1;

    uint16_t crc = 0xffff;
    uint8_t header[6] = { 'S', 't', opcode, 0x01, (length >> 8) & 0xFF, length & 0xFF };

    dprintf(DEBUG_SPEW, "mcu_send_packet 0x%02x\n", opcode);

    for (unsigned i = 0; i < sizeof(header); i++) {
	uart_putc(MCU_SERIAL_PORT, header[i]);
    }
    for (unsigned i = 0; i < length; i++) {
	uart_putc(MCU_SERIAL_PORT, payload[i]);
    }

    crc = crc_ccitt(crc, header, sizeof(header));
    crc = crc_ccitt(crc, payload, length);

    uart_putc(MCU_SERIAL_PORT, (crc >> 8) & 0xFF);
    uart_putc(MCU_SERIAL_PORT, crc & 0xFF);

    return 0;
}

static int mcu_read_response(MCUOpCode *outOpcode, uint8_t *outData, size_t maxLen, size_t *outLen)
{
    // wait at most a second
    utime_t timeout = system_time() + kMCUResponseTimeout;

    // Wait for a packet.
    while (!mcu_recv_valid) {
	utime_t now = system_time();
	if (now >= timeout) {
	    dprintf(DEBUG_INFO, "mcu response timeout\n");
	    return -1;
	}
	event_wait_timeout(&mcu_packet_arrived, timeout - now);
    }

    dprintf(DEBUG_SPEW, "received mcu packet type %#x\n", mcu_recv.opcode);
    if (outOpcode != NULL) *outOpcode = mcu_recv.opcode;
    if (outData) memcpy(outData, mcu_recv.buf, maxLen < mcu_recv.len ? maxLen : mcu_recv.len);
    if (outLen) *outLen = mcu_recv.len;
    if (mcu_recv.len > maxLen) {
	dprintf(DEBUG_SPEW, "mcu packet exceeds maxLen (%d vs %d)\n",
		(int) mcu_recv.len, (int) maxLen);
    }
    mcu_recv_valid = false;
    event_signal(&mcu_packet_consumed);
    return 0;
}

static int mcu_read_packet(MCUOpCode *outOpcode, uint8_t *outData, size_t maxLen, size_t *outLen)
{
    utime_t timeout = system_time() + kMCUReadTimeoutUS;
    int ret = -1;
    size_t offset = 0;
    size_t length = 0;
    uint16_t crc = 0xffff;

    for (;;) {
	int ch;
	for (;;) {
	    ch = uart_getc(MCU_SERIAL_PORT, false);
	    if (ch >= 0) break;
	    if (system_time() >= timeout) goto exit;
	    task_sleep(kMCUReadSleepUS);
	}
	
	if (offset == 0) {
	    if (ch != 'S') continue;
	} else if (offset == 1) {
	    if (ch != 't') {
		offset = (ch == 'S') ? 1 : 0;
		continue;
	    }
	} else if (offset == 2) {
	    if (outOpcode != NULL) *outOpcode = ch;
	} else if (offset == 3) {
	    // ignore sequence number
	} else if (offset == 4) {
	    length = (ch & 0xFF) << 8;
	} else if (offset == 5) {
	    length |= (ch & 0xFF);
	} else if (offset < (length + 6)) {
	    if ((maxLen + 6) > offset) {
		outData[offset-6] = ch;
	    }
	} else if ((offset - (length + 6)) == 0) {
	    if (((crc >> 8) & 0xFF) != ch) {
		dprintf(DEBUG_INFO, "mcu checksum failure\n");
		goto exit;
	    }
	} else if ((offset - (length + 6)) == 1) {
	    if ((crc & 0xFF) != ch) {
		dprintf(DEBUG_INFO, "mcu checksum failure\n");
		goto exit;
	    }

	    if (outLen != NULL) *outLen = (maxLen < length) ? maxLen : length;
	    ret = 0;
	    goto exit;
	}

	if (offset < (length + 6)) {
	    uint8_t b = ch;
	    crc = crc_ccitt(crc, &b, sizeof(b));
	}
	offset++;
    }
    
    dprintf(DEBUG_INFO, "mcu read timeout\n");

 exit:
    return ret;
}

const int order = 16;
const unsigned long polynom = 0x1021;
 
static unsigned long crc_ccitt(unsigned long crc, unsigned char* p, unsigned long len)
{
	// fast bit by bit algorithm without augmented zero bytes.
	// does not use lookup table, suited for polynom orders between 1...32.
	
	unsigned long i, j, c, bit;

	unsigned long crchighbit = (unsigned long)1<<(order-1);
	
	for (i=0; i<len; i++) {
		
		c = (unsigned long)*p++;
		
		for (j=0x80; j; j>>=1) {
			
			bit = crc & crchighbit;
			crc<<= 1;
			if (c & j)
				bit ^= crchighbit;
			if (bit)
				crc ^= polynom;
		}
	}	
	
	return crc;
}

#if WITH_MENU

static int mcu_send_raw(int command, uint8_t *data, int data_len)
{
    int ret;
    uint8_t opcode;
    ret = mcu_send_packet(command, data, data_len);
    if (ret != 0) {
	printf("mcu_send_packet failed: %d\n", ret);
	return ret;
    }
    ret = mcu_read_response(&opcode, NULL, 0, NULL);
    if (ret != 0) {
	printf("mcu_read_response failed: %d\n", ret);
	return ret;
    }
    return 0;
}

static int do_mcu(int argc, struct cmd_arg *args)
{
    if (!gIsMCUInitialized) {
        if (0 == mcu_init()) {
            printf("WARNING: mcu not previously initialized");
        } else {
            printf("ERROR: unable to initialize mcu");
            return -1;
        }
    }

    if ((argc > 1) && !strcmp(args[1].str, "version"))
    {
	char version[32];
	if (mcu_get_version(version, sizeof(version)) == 0) {
	    printf("%s\n", version);
	}
	return 0;
    }

    if ((argc > 2) && !strcmp(args[1].str, "sil"))
    {
	int action = kMCUSILAction_Invalid;

	if (!strcmp(args[2].str, kMCUSILAction_OffStr)) action = kMCUSILAction_Off;
	else if (!strcmp(args[2].str, kMCUSILAction_OnStr)) action = kMCUSILAction_On;
	else if (!strcmp(args[2].str, kMCUSILAction_SlowStr)) action = kMCUSILAction_SlowFlash;
	else if (!strcmp(args[2].str, kMCUSILAction_FastStr)) action = kMCUSILAction_FastFlash;
	else if (!strcmp(args[2].str, kMCUSILAction_1BlinkStr)) action = kMCUSILAction_1Blink;
	else if (!strcmp(args[2].str, kMCUSILAction_3BlinksStr)) action = kMCUSILAction_3Blinks;

	if (action != kMCUSILAction_Invalid)
	{
	    return mcu_set_sil(action);
	}
    }

    if ((argc > 2) && !strcmp(args[1].str, "bs"))
    {
	int action = kMCUBSControl_Invalid;
	int extra = -1;

	if (!strcmp(args[2].str, kMCUBSControl_ResetStr)) action = kMCUBSControl_Reset;
	else if (!strcmp(args[2].str, kMCUBSControl_EnterTestModeStr)) action = kMCUBSControl_EnterTestMode;
	else if (!strcmp(args[2].str, kMCUBSControl_PassthroughModeStr)) action = kMCUBSControl_PassthroughMode;
	else if (!strcmp(args[2].str, kMCUBSControl_InfoFrameEnableStr) && argc > 3) {
	    action = kMCUBSControl_InfoFrameEnable;
	    extra = args[3].u;
	}

	if (action != kMCUBSControl_Invalid)
	{
	    return mcu_set_bs(action, extra);
	}
	
    }

    if ((argc > 2) && !strcmp(args[1].str, "raw"))
    {
	int i;
	uint8_t command = args[2].u;
	uint8_t data[8];
	size_t data_len = argc - 3;
	if (data_len > sizeof(data)) {
	    printf("Max %zu raw bytes\n", sizeof(data) + 1);
	    return -1;
	}
	for (i = 3; i < argc; ++i) data[i - 3] = args[i].u;
	return mcu_send_raw(command, data, data_len);
    }

    printf("%s version\n", args[0].str);
    printf("%s sil [%s|%s|%s|%s|%s|%s]\n", args[0].str,
	   kMCUSILAction_OffStr, kMCUSILAction_OnStr, kMCUSILAction_SlowStr, kMCUSILAction_FastStr,
	   kMCUSILAction_1BlinkStr, kMCUSILAction_3BlinksStr);
    printf("%s bs [%s|%s|%s|%s]\n", args[0].str,
	   kMCUBSControl_ResetStr, kMCUBSControl_EnterTestModeStr, kMCUBSControl_PassthroughModeStr, kMCUBSControl_InfoFrameEnableStr);
    printf("%s raw <byte> ...\n", args[0].str);
    
    return -1;
}

MENU_COMMAND_DEVELOPMENT(mcu, do_mcu, "MCU control", NULL);

#endif
