/*                                                                              
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef ASPsim_utilBoot_h
#define ASPsim_utilBoot_h



#define UTIL_NUM_ELEMENT    (12)
#define UTIL_NUM_PSLOTS     (128)
#define UTIL_NO_DEFECT      (0xff)
typedef struct {
    uint8_t         utilMajor;
    uint8_t         utilMinor;
    uint16_t        eSize[UTIL_NUM_ELEMENT];    // element size in sectors, 0 if never written.
    uint8_t         defects[UTIL_NUM_PSLOTS];
    uint16_t        numDefects;
    uint16_t        numPages;
    uint8_t         secPerPage;
    uint8_t         numPlanes;
    uint8_t         numDie;
    uint8_t         numSlots;
    uint8_t         sizeFWslots;            // size of the firmware element in slot granularity
    uint8_t         RFU[8];                 // reserved for future use
}UtilBootDM_t;

#define BOOT_NO_DEFECT      (0xff)

#define     BOOT_LBA_TOKEN_BASE      0xFF00000
#define     BOOT_LBA_TOKEN_UTILNOP  (BOOT_LBA_TOKEN_BASE + 1)
#define     BOOT_LBA_TOKEN_ELEMENTS (BOOT_LBA_TOKEN_BASE + 2)
#define     BOOT_LBA_TOKEN_LLB      (BOOT_LBA_TOKEN_BASE + 2)
#define     BOOT_LBA_TOKEN_FW       (BOOT_LBA_TOKEN_BASE + 3)
#define     BOOT_LBA_TOKEN_UTILDM   (BOOT_LBA_TOKEN_BASE + 4)
#define     BOOT_LBA_TOKEN_DM          (BOOT_LBA_TOKEN_BASE + 0x05)
#define     BOOT_LBA_TOKEN_CTRLBITS    (BOOT_LBA_TOKEN_BASE + 0x06)
#define     BOOT_LBA_TOKEN_EFFACEABLE  (BOOT_LBA_TOKEN_BASE + 0x07)
#define     BOOT_LBA_TOKEN_NVRAM       (BOOT_LBA_TOKEN_BASE + 0x08)
#define     BOOT_LBA_TOKEN_SYSCFG      (BOOT_LBA_TOKEN_BASE + 0x09)
#define     BOOT_LBA_TOKEN_PANICLOG    (BOOT_LBA_TOKEN_BASE + 0x0A)
#define     BOOT_LBA_TOKEN_UNKNOWN     (BOOT_LBA_TOKEN_BASE + 0x10)

#define     BOOT_LBA_TOKEN_FAST_FIRST  BOOT_LBA_TOKEN_UTILDM
#define     BOOT_LBA_TOKEN_FAST_LAST   BOOT_LBA_TOKEN_PANICLOG


#define     BOOT_ELEMENT_UTILDM     (2)
#define     BOOT_BAND_UTILDM        (0)
#define     BOOT_DIP_UTILDM         (0)
#define     BOOT_PAGE_UTILDM        (0)
#define     BOOT_SIZE_UTILDM        (1)

#define     BOOT_ELEMENT_LLB        (0)
#define     BOOT_BAND_LLB           (0)
#define     BOOT_DIP_LLB            (0)
#define     BOOT_PAGE_LLB           (1)
#define     BOOT_SIZE_LLB           (UtilDM->s.eSize[BOOT_ELEMENT_LLB])

#define     BOOT_ELEMENT_FW         (1)
#define     BOOT_SLOT_FW            (1)
#define     BOOT_PAGE_FW            (0)
#define     BOOT_SIZE_FW            (UtilDM->s.eSize[BOOT_ELEMENT_FW])

#define     BOOT_ELEMENT_NVRAM      (6)
#define     BOOT_BYTES_PER_SEC      (4096)

#define     BOOT_NO_ERROR           (0)
#define     BOOT_ERR_ABORT          (10)
#define     BOOT_UTIL_MAJOR         (8) // MUST match AppleStorageProcessor/src/aspcore/core/format.h

#define     BOOT_ELEMENT_FAST_FIRST BOOT_ELEMENT_UTILDM
#define     BOOT_ELEMENT_FAST_LAST  BOOT_ELEMENT_NVRAM

#define     BOOT_SLOT_LLB        0

#define     BOOT_SLOT_FW_A_FIRST 1
#define     BOOT_SLOT_FW_A_LAST  (BOOT_SLOT_FW_A_FIRST + (1 * UtilDM->s.sizeFWslots) - 1)
#define     BOOT_SLOT_FW_B_LAST  (BOOT_SLOT_FW_A_FIRST + (2 * UtilDM->s.sizeFWslots) - 1)


#define     BOOT_SLOT_SYSCFG_A   (BOOT_SLOT_FW_B_LAST + 1)
#define     BOOT_SLOT_SYSCFG_B   (BOOT_SLOT_SYSCFG_A + 1)

#define     BOOT_SLOT_FAST_A     (BOOT_SLOT_SYSCFG_B + 1)
#define     BOOT_SLOT_FAST_B     (BOOT_SLOT_FAST_A + 1)

#define     BOOT_SLOT_PANICLOG   (BOOT_SLOT_FAST_B + 1)

#define     BOOT_ERR_BLANK       0xffffffff
#endif

