/*
 * Copyright (C) 2012, 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/a7iop/a7iop.h>
#include <drivers/sep/sep_client.h>
#include <sys/task.h>

#ifdef DEBUG
#define print(x,y...)	dprintf(DEBUG_INFO,x,##y)
#else
#define print(x,y...)	(void)0
#endif

#define PING_TIMEOUT		(1 * 1000 * 1000)	/* in usecs */
#define GEN_NONC_TIMEOUT	(1 * 1000 * 1000)	/* in usecs */
#define GET_NONC_TIMEOUT	(1 * 1000 * 1000)	/* in usecs */
#define NONC_HSH_TIMEOUT	(2 * 1000 * 1000)	/* in usecs */
#define GEN_DPA_TIMEOUT		(2 * 1000 * 1000)	/* in usecs */

/* XXX Endpoint and Opcode defines goes away once we have a header in SDK */
#define kEndpoint_SEPROM	(255)

enum {
	kOpCode_Ping = 1,
	kOpCode_GenerateNonce = 3,
	kOpCode_GetNonceWord = 4,
	kOpCode_SendDpa = 15,
	kOpCode_Ack = 101,
	kOpCode_ReportGeneratedNonce = 103,
	kOpCode_ReportNonceWord = 104,
	kOpCode_ReportSentDpa = 115,
	kOpCode_LogRaw = 201,
	kOpCode_LogPrintable = 202,
	kOpCode_AnnouceStatus = 210,
	kOpCode_ReportPanic = 255
};

enum {
	kMsg_Ping = 0,
	kMsg_Ack,
	kMsg_GenerateNonce,
	kMsg_ReportGeneratedNonce,
	kMsg_GetNonceWord,
	kMsg_ReportNonceWord,
	kMsg_LogRaw,
	kMsg_LogPrintable,
	kMsg_AnnounceStatus,
	kMsg_ReportPanic,
	kMsg_SendDpa,
	kMsg_ReportSentDpa,
};

static const struct sep_message_info {
	uint8_t		opcode;
	uint8_t		tag;
	const char	opcode_string[255];
} _sep_msgs_info[] = {
	[kMsg_Ping]			=	{ kOpCode_Ping,			'p',	"Ping" },
	[kMsg_Ack]			=	{ kOpCode_Ack,			'a',	"Ack" },
	[kMsg_GenerateNonce]		=	{ kOpCode_GenerateNonce,	'g',	"GenerateNonce" },
	[kMsg_ReportGeneratedNonce]	=	{ kOpCode_ReportGeneratedNonce,	'r',	"ReportGeneratedNonce" },
	[kMsg_GetNonceWord]		=	{ kOpCode_GetNonceWord,		'n',	"GetNonceWord" },
	[kMsg_ReportNonceWord]		=	{ kOpCode_ReportNonceWord,	0,	"ReportNonceWord" },
	[kMsg_LogRaw]			=	{ kOpCode_LogRaw,		0,	"LogRaw" },
	[kMsg_LogPrintable]		=	{ kOpCode_LogPrintable,		0,	"LogPrintable" },
	[kMsg_AnnounceStatus]		=	{ kOpCode_AnnouceStatus,	0,	"AnnouceStatus" },
	[kMsg_ReportPanic]		=	{ kOpCode_ReportPanic,		0,	"ReportPanic" },
	[kMsg_SendDpa]			=	{ kOpCode_SendDpa,		0,	"SendDpa" },
	[kMsg_ReportSentDpa]		=	{ kOpCode_ReportSentDpa,	0,	"ReportSentDpa" },
};

/* SEP mbox message format */
struct sep_message {
	uint8_t		endpoint;
	uint8_t		tag;
	uint8_t		opcode;
	uint8_t		param;
	uint32_t	data;
} __attribute__((packed));

static struct sep_message _sep_recv_msg;
static struct sep_message _sep_send_msg;
static struct sep_message _sep_pending_recv_msg;
static uint8_t _sep_nonce_buffer[SEP_NONCE_SIZE] __attribute__((aligned(4)));
static int _sep_client_reader(uint32_t wait_timeout);
static int _sep_client_seed_aes();

static const char * _sep_get_opcode_string(uint8_t opcode)
{
	uint32_t i;

	for(i = 0; i < sizeof(_sep_msgs_info)/sizeof(_sep_msgs_info[0]); i++) {
		if (opcode == _sep_msgs_info[i].opcode) return _sep_msgs_info[i].opcode_string;
	}

	return NULL;
}

static void _sep_create_message(struct sep_message *msg, uint8_t msg_type, uint8_t tag, uint8_t param) 
{
	msg->endpoint = kEndpoint_SEPROM;
	msg->opcode = _sep_msgs_info[msg_type].opcode;
	msg->tag = ((tag != 0) ? tag : _sep_msgs_info[msg_type].tag);
	msg->param = param;
	msg->data = 0;
	print("%s: msg %x %s(%d) %d\n", __FUNCTION__, msg->endpoint, _sep_get_opcode_string(msg->opcode), msg->opcode, msg->param);
}

static int _sep_client_get_nonce()
{
	uint32_t idx;
	uint32_t zero = 0;
	int ret = -1;
	bool is_zero = true;

	/* Setup AKF SEP */
	print("sep_client: starting akf\n");
	akf_start_sep();

	/* Generate SEP Nonce */
	print("sep_client: sending generate nonce\n");
	_sep_create_message(&_sep_send_msg, kMsg_GenerateNonce, 0, 0);
	if (akf_send_mbox(KFW_SEP, *((uint64_t *)&_sep_send_msg), A7IOP_NO_WAIT)) {
		dprintf(DEBUG_CRITICAL, "Unable to send GEN_NONCE to SEP mailbox, abandoning...\n");
		goto exit;
	}
	if (_sep_client_reader(GEN_NONC_TIMEOUT)) {
		dprintf(DEBUG_CRITICAL, "SEP not respoding to GEN_NONCE, abandoning...\n");
		goto exit;
	}

	print("sep_client: nonce size: %d\n", _sep_pending_recv_msg.data);
	RELEASE_ASSERT(_sep_pending_recv_msg.data == (SEP_NONCE_SIZE * 8));
	if (_sep_pending_recv_msg.data != (SEP_NONCE_SIZE * 8)) {
		dprintf(DEBUG_CRITICAL, "Invalid nonce size: %d, abandoning...\n", _sep_pending_recv_msg.data);
		goto exit;
	}

	/* Retrieve SEP Nonce, word by word */
	for (idx = 0; idx < (SEP_NONCE_SIZE/sizeof(uint32_t)); idx++) {
		print("sep_client: sending get nonce word %d\n", idx);
		_sep_create_message(&_sep_send_msg, kMsg_GetNonceWord, 0, idx);
		if (akf_send_mbox(KFW_SEP, *((uint64_t *)&_sep_send_msg), A7IOP_NO_WAIT)) {
			dprintf(DEBUG_CRITICAL, "Unable to send GET_NONCE to SEP mailbox, abandoning...\n");
			goto exit;
		}
		if (_sep_client_reader(GET_NONC_TIMEOUT)) {
			dprintf(DEBUG_CRITICAL, "SEP not respoding to GET_NONCE, abandoning...\n");
			goto exit;
		}
		print("sep_client: got nonce word %d: %08x\n", idx, _sep_pending_recv_msg.data);
		memcpy(&_sep_nonce_buffer[idx * sizeof(_sep_pending_recv_msg.data)], (void *)&_sep_pending_recv_msg.data, sizeof(_sep_pending_recv_msg.data));
		if (memcmp(&_sep_pending_recv_msg.data, &zero, sizeof(zero))) {
			is_zero = false;
		}
	}

	if (is_zero) {
		panic("SEP sent all 0's as nonce");
	}

	ret = 0;
exit:
	print("sep_client: stopping akf\n");

	/* Cleanup */
	akf_stop(KFW_SEP);

	print("sep_client: finished\n");

	return ret;
}

static int _sep_client_seed_aes()
{
	int ret = -1;
	/* Setup AKF SEP */
	print("sep_client: starting akf\n");
	akf_start_sep();

	/* Send Dpa*/
	print("sep_client: sending dpa\n");
	_sep_create_message(&_sep_send_msg, kMsg_SendDpa, 0, 0);
	if (akf_send_mbox(KFW_SEP, *((uint64_t *)&_sep_send_msg), A7IOP_NO_WAIT)) {
		dprintf(DEBUG_CRITICAL, "Unable to send SendDpa to SEP mailbox, abandoning...\n");
		goto exit;
	}
	if (_sep_client_reader(GEN_DPA_TIMEOUT)) {
		dprintf(DEBUG_CRITICAL, "SEP not respoding to SendDpa, abandoning...\n");
		goto exit;
	}

	ret = 0;
exit:
	print("sep_client: stopping akf\n");

	/* Cleanup */
	akf_stop(KFW_SEP);

	print("sep_client: finished\n");

	return ret;
}

static int _sep_client_reader(uint32_t wait_timeout)
{
	uint32_t ret;
	bool received_message = false;

	for (;;) {
		ret = akf_recv_mbox(KFW_SEP, (uint64_t *)&_sep_recv_msg, wait_timeout);

		print("%s: msg %x %s(%d) %d 0x%08x\n", __FUNCTION__, _sep_recv_msg.endpoint, _sep_get_opcode_string(_sep_recv_msg.opcode), _sep_recv_msg.opcode, _sep_recv_msg.param, _sep_recv_msg.data);
		
		switch(_sep_recv_msg.opcode) {
			case kOpCode_LogRaw:
			case kOpCode_LogPrintable:
			case kOpCode_AnnouceStatus:
			case kOpCode_ReportPanic:
				break;

			default:
				memcpy(&_sep_pending_recv_msg, &_sep_recv_msg, sizeof(_sep_pending_recv_msg));
				received_message = true;
			break; 
		}
		bzero(&_sep_recv_msg, sizeof(_sep_recv_msg));
		
		if (received_message)
			break;
	}

	return ret;
}


/* sep_client_get_nonce
 *
 * Sends a mbox to SEP to generate a nonce.
 * It then sends multile mboxs to retrieve nonce (word by word)
 *
 * Return 0 for success, -1 for fail
 */
int32_t sep_client_get_nonce(uint8_t *buffer)
{
	RELEASE_ASSERT(buffer != NULL);

	if (_sep_client_get_nonce()) {
		dprintf(DEBUG_CRITICAL, "Failed to retrieve SEP nonce\n");
		return -1;
	}

	memcpy(buffer, _sep_nonce_buffer, SEP_NONCE_SIZE);

	return 0;
}

/* sep_client_seed_aes
 *
 * Send DPA command to SEP in order to seed the various AES blocks
 *
 * Return 0 for success, -1 for fail
 */
int32_t sep_client_seed_aes()
{
	if (_sep_client_seed_aes()) {
		dprintf(DEBUG_CRITICAL, "Failed to seed AES block\n");
		return -1;
	}

	return 0;
}
