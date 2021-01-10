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
#ifndef TBT_PROTOCOL_H
#define TBT_PROTOCOL_H

#define TBT_CFG_SPACE_PATH			(0)
#define TBT_CFG_SPACE_PORT			(1)
#define TBT_CFG_SPACE_DEVICE			(2)
#define TBT_CFG_SPACE_COUNTERS			(3)

#define TBT_CFG_READ_PDF			(1)
#define TBT_CFG_WRITE_PDF			(2)
#define TBT_CFG_ERROR_PDF			(3)
#define TBT_CFG_NOTIFY_ACK_PDF			(4)
#define TBT_CFG_PLUG_EVENT_PDF			(5)
#define TBT_CFG_XDOMAIN_REQUEST_PDF		(6)
#define TBT_CFG_XDOMAIN_RESPONSE_PDF		(7)
#define TBT_CFG_CM_OVERRIDE_PDF			(8)
#define TBT_CFG_RESET_PDF			(9)
#define TBT_CFG_PREPARE_TO_SLEEP_PDF		(13)
#define TBT_CFG_MAX_PDF				(15)

#define TBT_CFG_HEADER_LEN			(8)
#define TBT_CFG_MAX_HEADER_AND_PAYLOAD		(252) // XXX: Check this. Does NHI give us the first 4 bytes or not?

#define TBT_CFG_ROUTE_CM_MASK			(1ULL << 63)
#define TBT_CFG_ROUTE_STRING_MASK		(~(3ULL << 62))

#define TBT_XD_REQUEST_HEADER_LEN		(TBT_CFG_HEADER_LEN + 4 + 16 + 4)
#define TBT_XD_RESPONSE_HEADER_LEN		(TBT_XD_REQUEST_HEADER_LEN)
#define TBT_XD_REQUEST_PROTOCOL_UUID_OFFSET	(TBT_CFG_HEADER_LEN + 4)
#define TBT_XD_REQUEST_PACKET_TYPE_OFFSET	(TBT_CFG_HEADER_LEN + 4 + 16)
#define TBT_XD_REQUEST_MAX_PAYLOAD		(TBT_CFG_MAX_HEADER_AND_PAYLOAD - TBT_XD_REQUEST_HEADER_LEN)
#define TBT_XD_RESPONSE_MAX_PAYLOAD		(TBT_XD_REQUEST_MAX_PAYLOAD)

#define TBT_XD_UUID_REQUEST			(1)
#define TBT_XD_UUID_RESPONSE			(2)
#define TBT_XD_ROM_READ_REQUEST			(3)
#define TBT_XD_ROM_READ_RESPONSE		(4)
#define TBT_XD_ROM_CHANGED_REQUEST		(5)
#define TBT_XD_ROM_CHANGED_RESPONSE		(6)
#define TBT_XD_ERROR_RESPONSE			(7)
#define TBT_XD_UUID_V2_REQUEST			(12)

#define TBT_XD_UUID_REQUEST_LEN			(TBT_XD_REQUEST_HEADER_LEN)
#define TBT_XD_UUID_RESPONSE_LEN		(TBT_XD_RESPONSE_HEADER_LEN + 16)
#define TBT_XD_ROM_CHANGED_REQUEST_LEN		(TBT_XD_REQUEST_HEADER_LEN + 16)
#define TBT_XD_ROM_CHANGED_RESPONSE_LEN		(TBT_XD_RESPONSE_HEADER_LEN)
#define TBT_XD_ROM_READ_REQUEST_LEN		(TBT_XD_REQUEST_HEADER_LEN + 16 + 16 + 4)
#define TBT_XD_ROM_READ_RESPONSE_DATA_OFFSET	(16 + 16 + 4 + 4)
#define TBT_XD_ROM_READ_RESPONSE_MAX_DATA	(TBT_CFG_MAX_HEADER_AND_PAYLOAD - TBT_XD_RESPONSE_HEADER_LEN - TBT_XD_ROM_READ_RESPONSE_DATA_OFFSET)

#endif