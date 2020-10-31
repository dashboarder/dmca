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

#ifndef __AE2_MCA__
#define __AE2_MCA__

#include <stdint.h>
#include <platform/int.h>

#define rMCA_BASE	0x34196000
#define rMCA0_BASE    (rMCA_BASE         )
#define rMCA1_BASE    (rMCA_BASE + 0x1000)

#define rMCAVERSION   0
#define rMCAFIFOSIZE  0x4
#define rMCAFIFOSHIFT 0x8
#define rMCACFG       0xC
#define rMCAUNSCFG    0x10
#define rMCAIDLE      0x14
#define rMCATXCFG     0x18
#define rMCAUNSTXCFG  0x1C
#define rMCATXFIFOCFG 0x20
#define rMCATXMASK    0x24
#define rMCATXDATA    0x28
#define rMCARXCFG     0x2C
#define rMCAUNSRXCFG  0x30
#define rMCARXFIFOCFG 0x34
#define rMCARXMASK    0x38
#define rMCARXDATA    0x3C
#define rMCASTATUS    0x40
#define rMCACTL       0x44

#define rMCA0_VERSION   ( rMCA0_BASE + rMCAVERSION  )
#define rMCA0_FIFOSHIFT ( rMCA0_BASE + rMCAFIFOSHIFT)
#define rMCA0_CFG       ( rMCA0_BASE + rMCACFG      )
#define rMCA0_UNSCFG    ( rMCA0_BASE + rMCAUNSCFG   )
#define rMCA0_IDLE      ( rMCA0_BASE + rMCAIDLE     )
#define rMCA0_TXCFG     ( rMCA0_BASE + rMCATXCFG    )
#define rMCA0_UNSTXCFG  ( rMCA0_BASE + rMCAUNSTXCFG )
#define rMCA0_TXFIFOCFG ( rMCA0_BASE + rMCATXFIFOCFG)
#define rMCA0_TXMASK    ( rMCA0_BASE + rMCATXMASK   )
#define rMCA0_TXDATA    ( rMCA0_BASE + rMCATXDATA   )
#define rMCA0_RXCFG     ( rMCA0_BASE + rMCARXCFG    )
#define rMCA0_UNSRXCFG  ( rMCA0_BASE + rMCAUNSRXCFG )
#define rMCA0_RXFIFOCFG ( rMCA0_BASE + rMCARXFIFOCFG)
#define rMCA0_RXMASK    ( rMCA0_BASE + rMCARXMASK   )
#define rMCA0_RXDATA    ( rMCA0_BASE + rMCARXDATA   )
#define rMCA0_STATUS    ( rMCA0_BASE + rMCASTATUS   )
#define rMCA0_CTL       ( rMCA0_BASE + rMCACTL      )

#define rMCA1_VERSION   ( rMCA1_BASE + rMCAVERSION  )
#define rMCA1_FIFOSHIFT ( rMCA1_BASE + rMCAFIFOSHIFT)
#define rMCA1_CFG       ( rMCA1_BASE + rMCACFG      )
#define rMCA1_UNSCFG    ( rMCA1_BASE + rMCAUNSCFG   )
#define rMCA1_IDLE      ( rMCA1_BASE + rMCAIDLE     )
#define rMCA1_TXCFG     ( rMCA1_BASE + rMCATXCFG    )
#define rMCA1_UNSTXCFG  ( rMCA1_BASE + rMCAUNSTXCFG )
#define rMCA1_TXFIFOCFG ( rMCA1_BASE + rMCATXFIFOCFG)
#define rMCA1_TXMASK    ( rMCA1_BASE + rMCATXMASK   )
#define rMCA1_TXDATA    ( rMCA1_BASE + rMCATXDATA   )
#define rMCA1_RXCFG     ( rMCA1_BASE + rMCARXCFG    )
#define rMCA1_UNSRXCFG  ( rMCA1_BASE + rMCAUNSRXCFG )
#define rMCA1_RXFIFOCFG ( rMCA1_BASE + rMCARXFIFOCFG)
#define rMCA1_RXMASK    ( rMCA1_BASE + rMCARXMASK   )
#define rMCA1_RXDATA    ( rMCA1_BASE + rMCARXDATA   )
#define rMCA1_STATUS    ( rMCA1_BASE + rMCASTATUS   )
#define rMCA1_CTL       ( rMCA1_BASE + rMCACTL      )

#define rMCASTATUS_FRAMEERROR 31
#define rMCASTATUS_RXOVERRUN 29
#define rMCASTATUS_RXUNDERRUN 28
#define rMCASTATUS_RXHIGHWATER 27
#define rMCASTATUS_RXLOWWATER 26
#define rMCASTATUS_TXOVERRUN 13
#define rMCASTATUS_TXUNDERRUN 12
#define rMCASTATUS_TXHIGHWATER 11
#define rMCASTATUS_TXLOWWATER 10
#define rMCASTATUS_RXFIFOLVL 16
#define rMCASTATUS_TXFIFOLVL 0
#define rMCASTATUS_FRAMEEEROR_MASK (    1 << rMCASTATUS_FRAMEERROR)
#define rMCASTATUS_RXOVERRUN_MASK (    1 << rMCASTATUS_RXOVERRUN)
#define rMCASTATUS_RXUNDERRUN_MASK (    1 << rMCASTATUS_RXUNDERRUN)
#define rMCASTATUS_TXOVERRUN_MASK (    1 << rMCASTATUS_TXOVERRUN)
#define rMCASTATUS_TXUNDERRUN_MASK (    1 << rMCASTATUS_TXUNDERRUN)
#define rMCASTATUS_RXFIFOLVL_MASK (0x1FF << rMCASTATUS_RXFIFOLVL)
#define rMCASTATUS_TXFIFOLVL_MASK (0x1FF << rMCASTATUS_TXFIFOLVL)

#define rMCAUNSRXCFG_IRQ_EN 1

#define rMCAUNSTXCFG_IRQ_EN 1

#endif /* __AE2_MCA__ */
