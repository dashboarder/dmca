/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DOCKCHANNEL_CONFIG_H
#define __DOCKCHANNEL_CONFIG_H

#include <drivers/dockchannel/dockchannel.h>
#include <platform/soc/hwisr.h>

// These values can also be read out from DC_HWCFG register but is fixed for a particular Dockchannel instance
#define DOCKCHANNEL_MAX_AGENTS								(4)
#define DOCKCHANNEL_MAX_CHANNELS							(5)

// Allocation of agents as per AOP Spec
// https://seg-docs.ecs.apple.com/projects/m8//release/specs/Apple/AOP/M8%20AOP%20Microarchitecture%20Specification.pdf
#define DOCKCHANNEL_DOCK_AGENT_ID							(0)
#define DOCKCHANNEL_AP_AGENT_ID								(1)
#define DOCKCHANNEL_AOP_AGENT_ID							(2)
#define DOCKCHANNEL_RTP_AGENT_ID							(3)

// Software allocation of channels. UART channel is 0 and needs to be the same as what kanzi firmware
// uses. Kanzi firmware separates out SWD communication and UART data, and presents a virtual uart device
// to users. 
// The channels are decided by us and we need to make sure the consumers, in this case kanzi or OS driver
// for DockChannel is setup as per these assignments
#define DOCKCHANNEL_UART								(0)
#define DOCKCHANNEL_DIAGS								(1)
#define DOCKCHANNEL_UNUSED_1								(2)
#define DOCKCHANNEL_UNUSED_2								(3)
#define DOCKCHANNEL_RTP									(4)

#endif /* __DOCKCHANNEL_CONFIG_H */
