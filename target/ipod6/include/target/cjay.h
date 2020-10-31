/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _Cjay_h_
#define _Cjay_h_

// Seajay calibration
typedef struct __attribute__((packed)) {
    uint8_t  Version;
    uint16_t Magic;
    uint8_t  IsNotCalibrated;
    uint32_t NumOfItems;
    uint32_t SizeOfItems;
} SeajayCalHdr;

typedef struct __attribute__((packed)) {
    uint32_t Status;
    uint32_t Offset; // Q15.16 fixed point, 4 bytes
    uint32_t pad1;
    uint32_t Gain;   // Q15.16 fixed point, 4 bytes
    uint32_t pad2;
} SeajayCalItem;

typedef struct __attribute__((packed)) {
    SeajayCalHdr    Hdr;
    SeajayCalItem   Item;
} SeajayCal;

enum {
    kCalStatus_NoCal  = 0x0,   /* This means that we haven't done any calibration */
    kCalStatus_Offset = 0x1,   /* Offset has been calibrated */
    kCalStatus_Gain   = 0x2,   /* Gain has been calibrated  */
    kCalStatus_All    = 0x3,   /* Both gain & offset have been calibrated */
};

#define SEAJAY_CAL_VERSION      0x2
#define SEAJAY_CAL_MAGIC        0xabcd

#endif /* _Cjay_h_ */
