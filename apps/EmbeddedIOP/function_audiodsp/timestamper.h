/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _TIMESTAMPER_H_
#define _TIMESTAMPER_H_

#include <platform.h>

#include "iop_audiodsp_protocol.h"

void set_timestamper_message_buffer(IOPAUDIODSP_Command *buffer);

void send_timestamp(uint32_t samplesTransferred, uint64_t timestamp);

#endif // _TIMESTAMPER_H_

