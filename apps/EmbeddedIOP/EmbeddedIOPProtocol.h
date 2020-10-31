/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


/*
 * This file defines constants and data structures that comprise the interface protocol between the
 * OS driver and the generic IOP firmware.
 */

/*
 * Patch structure.
 *
 * The IOP firmware contains the original version of this structure.  The host driver
 * patches it to inform the IOP firmware of its runtime environment before starting it.
 *
 */

struct iop_channel_config {
	uint32_t	ring_base;
	uint32_t	ring_size;
};
	
struct iop_configuration {
	uint32_t	magic;
#define IOP_CONFIG_MAGIC	'cnfg'

	uint32_t	options;
#define IOP_OPTION_NO_IDLE		(1<<0)	/* don't idle */
#define IOP_OPTION_DO_CLOCK_MGMNT	(1<<1)	/* do clock management */
	
	/* console message buffer */
	uint32_t	message_buffer;
	
	/* (optional) task deep idle timeout */
	uint32_t	deep_idle_us;

	/* control channels */
#define IOP_MAX_CHANNELS		10
	struct iop_channel_config channel[IOP_MAX_CHANNELS];

#define IOP_CONTROL_CHANNEL		0
#define IOP_CONTROL_CHANNEL_SIZE	2

#define IOP_MESSAGE_CHANNEL		1
/* note the message channel size is defined on a per-firmware-image basis */

#define SDIO_CONTROL_CHANNEL		3
#define SDIO_CONTROL_CHANNEL_SIZE	2

#define FMI_CONTROL_CHANNEL0		5
#define FMI_CONTROL_CHANNEL1		6
#define FMI_CONTROL_CHANNEL_SIZE	3

#define AUDIO_CONTROL_CHANNEL		7
#define AUDIO_CONTROL_CHANNEL_SIZE	2
	
#define AUDIODSP_TIMER_CHANNEL		2
#define AUDIODSP_TIMER_CHANNEL_SIZE	2

#define AUDIODSP_CONTROL_CHANNEL	4
#define AUDIODSP_CONTROL_CHANNEL_SIZE	2

#define AE2_WFI_ENDPOINT_CHANNEL	8
#define AE2_WFI_ENDPOINT_CHANNEL_SIZE	2

};

/*
 * Default channel definitions; these channels are prototyped by
 * the endpoint driver before starting the IOP, so they are
 * safe for the IOP firmware to touch at startup.
 */
static struct {
	uint32_t	channel_index;
	uint32_t	channel_size;
	uint32_t	producer;
} iop_default_channels[] __attribute__ ((unused)) = {
	{ SDIO_CONTROL_CHANNEL,		SDIO_CONTROL_CHANNEL_SIZE,	true },
	{ FMI_CONTROL_CHANNEL0,		FMI_CONTROL_CHANNEL_SIZE,	true },
	{ FMI_CONTROL_CHANNEL1,		FMI_CONTROL_CHANNEL_SIZE,	true },
	{ AUDIO_CONTROL_CHANNEL,	AUDIO_CONTROL_CHANNEL_SIZE,	true },
	{ AUDIODSP_CONTROL_CHANNEL,	AUDIODSP_CONTROL_CHANNEL_SIZE,	true },
	{ AUDIODSP_TIMER_CHANNEL,	AUDIODSP_TIMER_CHANNEL_SIZE,	false },
	{ AE2_WFI_ENDPOINT_CHANNEL,	AE2_WFI_ENDPOINT_CHANNEL_SIZE,	true },
	{ 0,				0,				0 }
};

/*
 * Host control pipe commands.
 */
#define IOP_CMD_NOP			'nop '
#define IOP_CMD_TERMINATE		'halt'
#define IOP_CMD_TTYIN			'ttin'
#define IOP_CMD_SLEEP			'slep'
#define IOP_CMD_SUSPEND			'spnd'
#define IOP_CMD_RESUME			'rsum'
#define IOP_CMD_INSTRUMENT		'inst'

#define IOP_RESULT_SUCCESS		0
#define IOP_RESULT_ERROR		1
#define IOP_RESULT_IN_PROGRESS		~0

struct iop_command_generic {
	uint32_t			opcode;
	uint32_t			result;
};

/*
 * Send a character to the debug input routine.
 */
struct iop_command_ttyin {
	struct iop_command_generic	gen;

	uint32_t			c;
};

/* Instrumentation */
struct iop_command_instrument {
        struct iop_command_generic	gen;

	UInt64				uptime_ticks;
	UInt64				deep_idle_ticks;
	UInt64				deep_idles;
	UInt64				idle_ticks;
	UInt64				idles;
	UInt32				threshold_us;
	UInt32				ticksHz;
};

/* Ping command */
struct iop_command_ping {
	uint32_t			opcode;
	uint32_t			result;
	uint32_t			ping_id;
};

/*
 * Command union must be a multiple of the IOP cacheline size
 * because the IOP will invalidate it out of the cache before
 * processing it.
 */
#define IOP_ROUNDUP(k,b)		(((k) + ((b) - 1)) & ~((b) - 1))
#if defined(CPU_CACHELINE_SIZE) && (CPU_CACHELINE_SIZE > 32)
#define IOP_COMMAND_MAX			IOP_ROUNDUP(sizeof(struct iop_command_instrument), CPU_CACHELINE_SIZE)
#else
#define IOP_COMMAND_MAX			IOP_ROUNDUP(sizeof(struct iop_command_instrument), 32)
#endif

union iop_command {
	struct iop_command_generic	generic;
	struct iop_command_ttyin	ttyin;
	struct iop_command_instrument   instr;
	struct iop_command_ping 	ping;

	uint8_t				_pad[IOP_COMMAND_MAX];
};

/*
 * IOP to host notifications.
 */
struct iop_message_generic {
	uint32_t			opcode;
	uint32_t			size;
};

#define IOP_MSG_TTY			'tty '		/* console tty output */
#define IOP_MSG_PANIC			'pnic'		/* firmware panic message */
#define IOP_MSG_TRACE			'trce'		/* trace message */

#define IOP_MESSAGE_MAX			128
#define IOP_MSG_TTY_MAXLEN		(IOP_MESSAGE_MAX - sizeof(struct iop_message_generic))

/*
 * Console output message.
 */
struct iop_message_tty {
	struct	iop_message_generic	gen;

	char				bytes[];
};

/*
 * Trace message.
 */
struct iop_message_trace {
	struct iop_message_generic	gen;

	uint32_t			ident;
	uint32_t			arg[4];
	uint64_t			timestamp;
};

union iop_message {
	struct iop_message_generic	gen;
	struct iop_message_tty		tty;
	struct iop_message_trace	trace;

	uint8_t				_pad[IOP_MESSAGE_MAX];
};

struct iop_ping_tracker {
    struct iop_command_ping		record;
    uint64_t				timestamp;
};
