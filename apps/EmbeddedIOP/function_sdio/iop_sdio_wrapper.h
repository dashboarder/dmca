/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _IOP_SDIO_WRAPPER_H
#define _IOP_SDIO_WRAPPER_H


#include "iop_sdio_protocol.h"


IOPSDIO_status_t iopsdio_init(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOInitCmd *initCmd);

IOPSDIO_status_t iopsdio_free(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOFreeCmd *freeCmd);

IOPSDIO_status_t iopsdio_reset(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOResetCmd *resetCmd);

IOPSDIO_status_t iopsdio_setBusConfig(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOSetBusParamCmd *busParamCmd);

IOPSDIO_status_t iopsdio_sendSDIOCmd(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOCommandCmd *commandCmd);

IOPSDIO_status_t iopsdio_transferSDIOData(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOTransferCmd *transferCmd);



#endif // _IOP_SDIO_WRAPPER_H


