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

#ifndef __AE2_I2S__
#define __AE2_I2S__

#include <stdint.h>
#include <platform/int.h>

#define rI2S_BASE	0x34190000
#define rI2S0_BASE  (rI2S_BASE         )
#define rI2S1_BASE  (rI2S_BASE + 0x1000)
#define rI2S2_BASE  (rI2S_BASE + 0x2000)
#define rI2S3_BASE  (rI2S_BASE + 0x3000)

#define rI2SCLKCON  0
#define rI2STXCON   0x4
#define rI2STXCOM   0x8
#define rI2STXDB    0x10
#define rI2SRXCON   0x30
#define rI2SRXCOM   0x34
#define rI2SRXDB    0x38
#define rI2SSTATUS  0x3C
#define rI2SBITCLK  0x40
#define rI2SVERSION 0x44

#define rI2S0_CLKCON  (rI2S0_BASE + rI2SCLKCON )
#define rI2S0_TXCON   (rI2S0_BASE + rI2STXCON  )
#define rI2S0_TXCOM   (rI2S0_BASE + rI2STXCOM  )
#define rI2S0_TXDB    (rI2S0_BASE + rI2STXDB   )
#define rI2S0_RXCON   (rI2S0_BASE + rI2SRXCON  )
#define rI2S0_RXCOM   (rI2S0_BASE + rI2SRXCOM  )
#define rI2S0_RXDB    (rI2S0_BASE + rI2SRXDB   )
#define rI2S0_STATUS  (rI2S0_BASE + rI2SSTATUS )
#define rI2S0_BITCLK  (rI2S0_BASE + rI2SBITCLK )
#define rI2S0_VERSION (rI2S0_BASE + rI2SVERSION)

#define rI2S1_CLKCON  (rI2S1_BASE + rI2SCLKCON )
#define rI2S1_TXCON   (rI2S1_BASE + rI2STXCON  )
#define rI2S1_TXCOM   (rI2S1_BASE + rI2STXCOM  )
#define rI2S1_TXDB    (rI2S1_BASE + rI2STXDB   )
#define rI2S1_RXCON   (rI2S1_BASE + rI2SRXCON  )
#define rI2S1_RXCOM   (rI2S1_BASE + rI2SRXCOM  )
#define rI2S1_RXDB    (rI2S1_BASE + rI2SRXDB   )
#define rI2S1_STATUS  (rI2S1_BASE + rI2SSTATUS )
#define rI2S1_BITCLK  (rI2S1_BASE + rI2SBITCLK )
#define rI2S1_VERSION (rI2S1_BASE + rI2SVERSION)

#define rI2S2_CLKCON  (rI2S2_BASE + rI2SCLKCON )
#define rI2S2_TXCON   (rI2S2_BASE + rI2STXCON  )
#define rI2S2_TXCOM   (rI2S2_BASE + rI2STXCOM  )
#define rI2S2_TXDB    (rI2S2_BASE + rI2STXDB   )
#define rI2S2_RXCON   (rI2S2_BASE + rI2SRXCON  )
#define rI2S2_RXCOM   (rI2S2_BASE + rI2SRXCOM  )
#define rI2S2_RXDB    (rI2S2_BASE + rI2SRXDB   )
#define rI2S2_STATUS  (rI2S2_BASE + rI2SSTATUS )
#define rI2S2_BITCLK  (rI2S2_BASE + rI2SBITCLK )
#define rI2S2_VERSION (rI2S2_BASE + rI2SVERSION)

#define rI2S3_CLKCON  (rI2S3_BASE + rI2SCLKCON )
#define rI2S3_TXCON   (rI2S3_BASE + rI2STXCON  )
#define rI2S3_TXCOM   (rI2S3_BASE + rI2STXCOM  )
#define rI2S3_TXDB    (rI2S3_BASE + rI2STXDB   )
#define rI2S3_RXCON   (rI2S3_BASE + rI2SRXCON  )
#define rI2S3_RXCOM   (rI2S3_BASE + rI2SRXCOM  )
#define rI2S3_RXDB    (rI2S3_BASE + rI2SRXDB   )
#define rI2S3_STATUS  (rI2S3_BASE + rI2SSTATUS )
#define rI2S3_BITCLK  (rI2S3_BASE + rI2SBITCLK )
#define rI2S3_VERSION (rI2S3_BASE + rI2SVERSION)

#define rI2SSTATUS_RXOVERRUN 20
#define rI2SSTATUS_TXOVERRUN 19
#define rI2SSTATUS_RXFIFOLVL 13
#define rI2SSTATUS_TXFIFOLVL 7
#define rI2SSTATUS_RXOVERRUN_MASK (   1 << 20)
#define rI2SSTATUS_TXOVERRUN_MASK (   1 << 19)
#define rI2SSTATUS_RXFIFOLVL_MASK (0x3F << 13)
#define rI2SSTATUS_TXFIFOLVL_MASK (0x3F << 7 )

#endif /* __AE2_I2S__ */