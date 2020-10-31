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

#ifndef _APPLE_EFFACEABLE_STORAGE_H
#define _APPLE_EFFACEABLE_STORAGE_H

// =============================================================================

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>

#include "AppleEffaceableStorageKeys.h"
#include "effaceable_contract.h"

// =============================================================================

class AppleEffaceableStorage : public IOService
{
    OSDeclareAbstractStructors(AppleEffaceableStorage);

public:
    /* client API */
    IOReturn getLocker(UInt32 type_id, void *data, IOByteCount *data_size, bool untrusted = false);
    IOReturn setLocker(UInt32 type_id, const void *data, IOByteCount data_size, bool untrusted = false);
    IOReturn spaceForLocker(UInt32 type_id, IOByteCount *data_space);
    IOReturn effaceLocker(UInt32 type_id, bool untrusted = false);
    IOReturn generateNonce(void * hash);

    /* additional public functions intended for manufacturing and/or diagnostics only */
    IOReturn getBytes(void * client_buf, IOByteCount offset, IOByteCount count);
    IOReturn wipeStorage(void);
    IOByteCount getCapacity(void);
    bool isFormatted(void);
    IOReturn formatStorage(void);

protected:
    /* subclass SPI */
    effaceable_storage_t * storage(void);
    effaceable_device_t * device(void);
    effaceable_system_t * system(void);

    EffaceableReturn efReturn(IOReturn ret);
    IOReturn ioReturn(EffaceableReturn ret);

    int logf(void * ignored, const char * fmt, ...);

    virtual void setupDeviceContract(void) = 0;

public:
    /* IOKit hooks */
    virtual bool start(IOService * provider);
    virtual void registerService(IOOptionBits options = 0);
    virtual IOWorkLoop * getWorkLoop() const;
    virtual IOReturn callPlatformFunction(const OSSymbol *functionSymbol, bool waitForFunction,
                                          void *param1, void *param2, void *param3, void *param4);

private:
    /* gated versions of public methods */
    IOReturn wipeStorageGated(void * tmp_buf);
    IOReturn formatStorageGated(void);
    IOReturn getBytesGated(void * client_buf, IOByteCount offset, IOByteCount count);
    IOReturn getLockerGated(UInt32 type_id, void *data, IOByteCount *data_size, bool untrusted);
    IOReturn setLockerGated(UInt32 type_id, const void *data, IOByteCount data_size, bool untrusted);
    IOReturn spaceForLockerGated(UInt32 type_id, IOByteCount *data_spage);
    IOReturn effaceLockerGated(UInt32 type_id, bool untrusted);
    IOReturn generateNonceGated(void * hash);

    /* methods to help get portable core setup and configured */
    void setupSystemContract(void);
    void handleBootArgs(void);

    /* implementation of portable core system services contract */
    void * allocMemSys(uint32_t size);
    void freeMemSys(void * buf, uint32_t size);
    void * setMemSys(void * buf, uint8_t val, uint32_t size);
    void * moveMemSys(void * dst, const void * src, uint32_t size);
    int cmpMemSys(const void * lhs, const void * rhs, uint32_t size);
    void readRandomSys(void * buf, uint32_t size);
    void calcSHA1Sys(const void * buf, uint32_t size, void * hash);
    uint32_t crc32Sys(uint32_t crc, const void * buf, uint32_t size);
    bool setPropertySys(const char * key, uint32_t value);
    void panicSys(const char * msg);
    int vlogfSys(const char * fmt, va_list ap);

private:
    /* friend C function hooks implementing trampoline back to C++ for portable core system contract */
    friend void * allocMemHook(effaceable_system_t * system, uint32_t size);
    friend void freeMemHook(effaceable_system_t * system, void * buf, uint32_t size);
    friend void * setMemHook(effaceable_system_t * system, void * buf, uint8_t val, uint32_t size);
    friend void * moveMemHook(effaceable_system_t * system, void * dst, const void * src, uint32_t size);
    friend int cmpMemHook(effaceable_system_t * system, const void * lhs, const void * rhs, uint32_t size);
    friend void readRandomHook(effaceable_system_t * system, void * buf, uint32_t size);
    friend void calcSHA1Hook(effaceable_system_t * system, const void * buf, uint32_t size, void * hash);
    friend uint32_t crc32Hook(effaceable_system_t * system, uint32_t crc, const void * buf, uint32_t size, bool ignored);
    friend bool setPropertyHook(effaceable_system_t * system, const char * key, uint32_t value);
    friend void panicHook(effaceable_system_t * system, const char * msg);
    friend int vlogfHook(effaceable_system_t * system, const char * fmt, va_list ap);

private:
    /* implementation data */
    IOCommandGate * _command_gate;
    IOWorkLoop * _work_loop;
    const OSSymbol * _function_get;
    const OSSymbol * _function_set;
    const OSSymbol * _function_space;
    const OSSymbol * _function_efface;
    const OSSymbol * _function_gen_nonce;
    IOCommandGate::Action _get_action;
    IOCommandGate::Action _set_action;
    IOCommandGate::Action _space_action;
    IOCommandGate::Action _efface_action;
    IOCommandGate::Action _gen_nonce_action;
    IOCommandGate::Action _format_action;
    IOCommandGate::Action _wipe_action;
    IOCommandGate::Action _get_bytes_action;
    effaceable_storage_t _storage;
    effaceable_system_t _system;
    effaceable_device_t _device;
};

// =============================================================================

#endif /* _APPLE_EFFACEABLE_STORAGE_H */
