/*
 * Copyright (c) 2010-11 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _APPLE_EFFACEABLE_USER_CLIENT_H
#define _APPLE_EFFACEABLE_USER_CLIENT_H

#include <IOKit/IOUserClient.h>
#include "AppleEffaceableStorage.h"

class AppleEffaceableStorageUserClient : public IOUserClient
{
    OSDeclareAbstractStructors(AppleEffaceableStorageUserClient);

public:
    virtual bool start(IOService * provider);
    virtual IOReturn clientClose(void);
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments * arguments,
                                    IOExternalMethodDispatch * dispatch, OSObject * target,
                                    void * reference);

private:
    AppleEffaceableStorage      * _provider;
};

#endif /* _APPLE_EFFACEABLE_USER_CLIENT_H */
