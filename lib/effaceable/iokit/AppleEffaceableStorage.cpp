/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <sys/types.h>
#include <sys/random.h>

#include <libkern/zlib.h>
#include <libkern/crypto/sha1.h>

#include <IOKit/IOTypes.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOUserClient.h>

#include "AppleEffaceableStorageKeys.h"
#include "AppleEffaceableStorageFormat.h"
#include "AppleEffaceableStorage.h"

#include "effaceable_contract.h"
#include "effaceable_debug.h"

// =============================================================================

#define debug(...) dlogf((void*)0, __VA_ARGS__ )

// =============================================================================

#ifdef super
#undef super
#endif
#define super IOService

OSDefineMetaClassAndAbstractStructors(AppleEffaceableStorage, IOService);

// =============================================================================

bool AppleEffaceableStorage::start(IOService *provider)
{
    bool ok = false;

    debug(INFO, "starting");

    if (!super::start(provider)) {
        debug(ERR, "super did not start\n");
    } else if (NULL == (_work_loop = IOWorkLoop::workLoop())) {
        debug(ERR, "couldn't init work loop");
    } else if (NULL == (_command_gate = IOCommandGate::commandGate(this))) {
        debug(ERR, "couldn't acquire command gate");
    } else if ((NULL == (_function_get       = OSSymbol::withCString(kAppleEffaceableStorageFunctionGet))) ||
               (NULL == (_function_set       = OSSymbol::withCString(kAppleEffaceableStorageFunctionSet))) ||
               (NULL == (_function_space     = OSSymbol::withCString(kAppleEffaceableStorageFunctionSpace))) ||
               (NULL == (_function_efface    = OSSymbol::withCString(kAppleEffaceableStorageFunctionEfface))) ||
               (NULL == (_function_gen_nonce = OSSymbol::withCString(kAppleEffaceableStorageFunctionGenerateNonce)))) {
        debug(ERR, "couldn't allocate function symbols");
    } else {

        // add command gate to work loop
        getWorkLoop()->addEventSource(_command_gate);

        // commandgate actions
#define _ACTION_CAST(_x)    OSMemberFunctionCast(IOCommandGate::Action, this, _x)
        _get_action         = _ACTION_CAST(&AppleEffaceableStorage::getLockerGated);
        _set_action         = _ACTION_CAST(&AppleEffaceableStorage::setLockerGated);
        _space_action       = _ACTION_CAST(&AppleEffaceableStorage::spaceForLockerGated);
        _efface_action      = _ACTION_CAST(&AppleEffaceableStorage::effaceLockerGated);
        _gen_nonce_action   = _ACTION_CAST(&AppleEffaceableStorage::generateNonceGated);
        _format_action      = _ACTION_CAST(&AppleEffaceableStorage::formatStorageGated);
        _get_bytes_action   = _ACTION_CAST(&AppleEffaceableStorage::getBytesGated);
        _wipe_action        = _ACTION_CAST(&AppleEffaceableStorage::wipeStorageGated);
#undef _ACTION_CAST

        // initalize portable core
        setupSystemContract();
        setupDeviceContract();
    
        registerService();
        ok = true;
    }

    if (!ok) {
        if (NULL != _command_gate) {
            _command_gate->release();
            _command_gate = NULL;
        }
        if (NULL != _work_loop) {
            _work_loop->release();
            _work_loop = NULL;
        }
    }

    return ok;
}

void
AppleEffaceableStorage::registerService(IOOptionBits options)
{
    const OSSymbol * user_client;

    // hook up the user client
    user_client = OSSymbol::withCStringNoCopy("AppleEffaceableStorageUserClient");
    setProperty(gIOUserClientClassKey, (OSObject *)user_client);
    user_client->release();

    super::registerService(options);
}

IOWorkLoop *
AppleEffaceableStorage::getWorkLoop() const
{
    return _work_loop;
}

// =============================================================================

IOByteCount
AppleEffaceableStorage::getCapacity(void)
{
    return (IOByteCount)storage()->getCapacity(storage());
}

bool
AppleEffaceableStorage::isFormatted(void)
{
    return storage()->isFormatted(storage());
}

IOReturn
AppleEffaceableStorage::formatStorage(void)
{
    return _command_gate->runAction(_format_action);
}

IOReturn
AppleEffaceableStorage::getBytes(void * client_buf, IOByteCount offset, IOByteCount count)
{
    if (PE_i_can_has_debugger(NULL)) {
        return _command_gate->runAction(_get_bytes_action, (void *) client_buf, (void *) offset, (void *) count);
    } else {
        return kIOReturnUnsupported;
    }
}

IOReturn
AppleEffaceableStorage::wipeStorage(void)
{
    if (PE_i_can_has_debugger(NULL)) {
        return _command_gate->runAction(_wipe_action);
    } else {
        return kIOReturnUnsupported;
    }
}

IOReturn
AppleEffaceableStorage::getLocker(UInt32 type_id, void *data, IOByteCount *data_size, bool untrusted)
{
    return _command_gate->runAction(_get_action, (void *)type_id, data, (void *)data_size, (void *)untrusted);
}

IOReturn
AppleEffaceableStorage::setLocker(UInt32 type_id, const void *data, IOByteCount data_size, bool untrusted)
{
    return _command_gate->runAction(_set_action, (void *)type_id, (void *)data, (void *)data_size, (void *)untrusted);
}

IOReturn
AppleEffaceableStorage::spaceForLocker(UInt32 type_id, IOByteCount *data_space)
{
    return _command_gate->runAction(_space_action, (void *)type_id, (void *)data_space);
}

IOReturn
AppleEffaceableStorage::effaceLocker(UInt32 type_id, bool untrusted)
{
    return _command_gate->runAction(_efface_action, (void *)type_id, (void *)untrusted);
}

IOReturn
AppleEffaceableStorage::generateNonce(void * hash)
{
    return _command_gate->runAction(_gen_nonce_action, hash);
}

IOReturn
AppleEffaceableStorage::callPlatformFunction(
    const OSSymbol          *functionSymbol,
    bool                    waitForFunction,
    void                    *param1,
    void                    *param2,
    void                    *param3,
    void                    *param4)
{
    if (functionSymbol == _function_get)
        return getLocker((UInt32)(uintptr_t)param1, param2, (IOByteCount *)param3, (bool)param4);

    if (functionSymbol == _function_set)
        return setLocker((UInt32)(uintptr_t)param1, param2, (IOByteCount)param3, (bool)param4);

    if (functionSymbol == _function_space)
        return effaceLocker((UInt32)(uintptr_t)param1, (IOByteCount *)param2);

    if (functionSymbol == _function_efface)
        return effaceLocker((UInt32)(uintptr_t)param1, (bool)param2);

    if (functionSymbol == _function_gen_nonce)
        return generateNonce(param1);

    return(IOService::callPlatformFunction(functionSymbol, waitForFunction, param1, param2, param3, param4));
}

// =============================================================================

IOReturn
AppleEffaceableStorage::formatStorageGated(void)
{
    return ioReturn(storage()->formatStorage(storage()));
}

IOReturn
AppleEffaceableStorage::getBytesGated(void * client_buf, IOByteCount offset, IOByteCount count)
{
    return ioReturn(storage()->getBytes(storage(), client_buf, offset, count));
}

IOReturn
AppleEffaceableStorage::wipeStorageGated(void * tmp_buf)
{
    return ioReturn(storage()->wipeStorage(storage(), tmp_buf));
}

// =============================================================================

IOReturn
AppleEffaceableStorage::getLockerGated(UInt32 type_id, void *data, IOByteCount *data_size, bool untrusted)
{
    return ioReturn(storage()->getLocker(storage(), type_id, data, (uint32_t *)data_size, untrusted));
}

IOReturn
AppleEffaceableStorage::setLockerGated(UInt32 type_id, const void *data, IOByteCount data_size, bool untrusted)
{
    return ioReturn(storage()->setLocker(storage(), type_id, data, (uint32_t)data_size, untrusted));
}

IOReturn
AppleEffaceableStorage::spaceForLockerGated(UInt32 type_id, IOByteCount *data_space)
{
    return ioReturn(storage()->spaceForLocker(storage(), type_id, (uint32_t *)data_space));
}

IOReturn
AppleEffaceableStorage::effaceLockerGated(UInt32 type_id, bool untrusted)
{
    return ioReturn(storage()->effaceLocker(storage(), type_id, untrusted));
}

IOReturn
AppleEffaceableStorage::generateNonceGated(void * hash)
{
    return ioReturn(storage()->generateNonce(storage(), hash));
}

// =============================================================================

effaceable_storage_t *
AppleEffaceableStorage::storage(void)
{
    return &_storage;
}

effaceable_device_t *
AppleEffaceableStorage::device(void)
{
    return &_device;
}

effaceable_system_t *
AppleEffaceableStorage::system(void)
{
    return &_system;
}

EffaceableReturn
AppleEffaceableStorage::efReturn(IOReturn ret)
{
    EffaceableReturn cast = kEffaceableReturnInvalid;
    switch (ret) {
    case kIOReturnSuccess          : cast = kEffaceableReturnSuccess         ; break;
    case kIOReturnError            : cast = kEffaceableReturnError           ; break;
    case kIOReturnNotFound         : cast = kEffaceableReturnNotFound        ; break;
    case kIOReturnUnsupported      : cast = kEffaceableReturnUnsupported     ; break;
    case kIOReturnNotPrivileged    : cast = kEffaceableReturnNotPrivileged   ; break;
    case kIOReturnNotPermitted     : cast = kEffaceableReturnNotPermitted    ; break;
    case kIOReturnBadArgument      : cast = kEffaceableReturnBadArgument     ; break;
    case kIOReturnBadMedia         : cast = kEffaceableReturnBadMedia        ; break;
    case kIOReturnIOError          : cast = kEffaceableReturnIOError         ; break;
    case kIOReturnNoMemory         : cast = kEffaceableReturnNoMemory        ; break;
    case kIOReturnNoSpace          : cast = kEffaceableReturnNoSpace         ; break;
    case kIOReturnInternalError    : cast = kEffaceableReturnInternalError   ; break;
    case kIOReturnUnformattedMedia : cast = kEffaceableReturnUnformattedMedia; break;
    }
    return cast;
}

IOReturn
AppleEffaceableStorage::ioReturn(EffaceableReturn ret)
{
    IOReturn cast = kIOReturnInvalid;
    switch (ret) {
    case kEffaceableReturnSuccess          : cast = kIOReturnSuccess         ; break;
    case kEffaceableReturnError            : cast = kIOReturnError           ; break;
    case kEffaceableReturnNotFound         : cast = kIOReturnNotFound        ; break;
    case kEffaceableReturnUnsupported      : cast = kIOReturnUnsupported     ; break;
    case kEffaceableReturnNotPrivileged    : cast = kIOReturnNotPrivileged   ; break;
    case kEffaceableReturnNotPermitted     : cast = kIOReturnNotPermitted    ; break;
    case kEffaceableReturnBadArgument      : cast = kIOReturnBadArgument     ; break;
    case kEffaceableReturnBadMedia         : cast = kIOReturnBadMedia        ; break;
    case kEffaceableReturnIOError          : cast = kIOReturnIOError         ; break;
    case kEffaceableReturnNoMemory         : cast = kIOReturnNoMemory        ; break;
    case kEffaceableReturnNoSpace          : cast = kIOReturnNoSpace         ; break;
    case kEffaceableReturnInternalError    : cast = kIOReturnInternalError   ; break;
    case kEffaceableReturnUnformattedMedia : cast = kIOReturnUnformattedMedia; break;
    }
    return cast;
}

// =============================================================================

int
AppleEffaceableStorage::logf(void * ignored, const char * fmt, ...)
{
    int err;

    va_list ap;
    va_start(ap, fmt);
    err = vlogfSys(fmt, ap);
    va_end(ap);

    return err;
}

// =============================================================================

void
AppleEffaceableStorage::handleBootArgs(void)
{
    uint32_t arg_flag = 0;
    bool enable_wipe = false;
    bool enable_full_scan = false;

    // boot-arg support to enable/disable of media wipe
    if (PE_parse_boot_argn("effaceable-enable-wipe", &arg_flag, sizeof (arg_flag)) && arg_flag) {
        enable_wipe = (0 != arg_flag);
        // XXX plumb down through to core implementation
        debug(INIT, "wipe %sabled via boot-arg", enable_wipe ? "en" : "dis");
    }

    // boot-arg support to enable/disable full scan
    if (PE_parse_boot_argn("effaceable-enable-full-scan", &arg_flag, sizeof (arg_flag)) && arg_flag) {
        enable_full_scan = (0 != arg_flag);
        // XXX plumb down through to core implementation
        debug(INIT, "full scan %sabled via boot-arg", enable_full_scan ? "en" : "dis");
    }
}

// =============================================================================

AppleEffaceableStorage * context(effaceable_system_t * system)
{
    AppleEffaceableStorage * context;

    // XXX assert instead?
    if (NULL == (context = (AppleEffaceableStorage *)system->opaque))
        panic("null context");

    return context;
}

void * allocMemHook(effaceable_system_t * system, uint32_t size)
{
    return context(system)->allocMemSys(size);
}

void freeMemHook(effaceable_system_t * system, void * buf, uint32_t size)
{
    context(system)->freeMemSys(buf, size);
}

void * setMemHook(effaceable_system_t * system, void * buf, uint8_t val, uint32_t size)
{
    return context(system)->setMemSys(buf, val, size);
}

void * moveMemHook(effaceable_system_t * system, void * dst, const void * src, uint32_t size)
{
    return context(system)->moveMemSys(dst, src, size);
}

int cmpMemHook(effaceable_system_t * system, const void * lhs, const void * rhs, uint32_t size)
{
    return context(system)->cmpMemSys(lhs, rhs, size);
}

void readRandomHook(effaceable_system_t * system, void * buf, uint32_t size)
{
    context(system)->readRandomSys(buf, size);
}

void calcSHA1Hook(effaceable_system_t * system, const void * buf, uint32_t size, void * hash)
{
    return context(system)->calcSHA1Sys(buf, size, hash);
}

uint32_t crc32Hook(effaceable_system_t * system, uint32_t crc, const void * buf, uint32_t size, bool ignored)
{
    return context(system)->crc32Sys(crc, buf, size);
}

bool setPropertyHook(effaceable_system_t * system, const char * key, uint32_t value)
{
    return context(system)->setPropertySys(key, value);
}

void panicHook(effaceable_system_t * system, const char * msg)
{
    context(system)->panicSys(msg);
}

int vlogfHook(effaceable_system_t * system, const char * fmt, va_list ap)
{
    return context(system)->vlogfSys(fmt, ap);
}

// =============================================================================

void
AppleEffaceableStorage::setupSystemContract(void)
{
    // XXX convert to appropriate type of AssertMacros
    if (sizeof(uint32_t) != sizeof(IOByteCount))
        panic("(sizeof(uint32_t) != sizeof(IOByteCount))");

    system()->opaque      = this;

    system()->allocMem    = allocMemHook;
    system()->freeMem     = freeMemHook;
    system()->setMem      = setMemHook;
    system()->moveMem     = moveMemHook;
    system()->cmpMem      = cmpMemHook;
    system()->readRandom  = readRandomHook;
    system()->calcSHA1    = calcSHA1Hook;
    system()->crc32       = crc32Hook;
    system()->setProperty = setPropertyHook;
    system()->panicSys    = panicHook;
    system()->vlogf       = vlogfHook;
}

void *
AppleEffaceableStorage::allocMemSys(uint32_t size)
{
    void * buf = IOMalloc(size);

    if (0 == buf) {
        panic("no memory");
    }

    return buf;
}

void
AppleEffaceableStorage::freeMemSys(void * buf, uint32_t size)
{
    IOFree(buf, size);
}

void *
AppleEffaceableStorage::setMemSys(void * buf, uint8_t val, uint32_t size)
{
    return memset(buf, val, size);
}

void *
AppleEffaceableStorage::moveMemSys(void * dst, const void * src, uint32_t size)
{
    return memmove(dst, src, size);
}

int
AppleEffaceableStorage::cmpMemSys(const void * lhs, const void * rhs, uint32_t size)
{
    return memcmp(lhs, rhs, size);
}

void
AppleEffaceableStorage::readRandomSys(void * buf, uint32_t size)
{
    read_random(buf, size);
}

void
AppleEffaceableStorage::calcSHA1Sys(const void * buf, uint32_t size, void * hash)
{
    SHA1_CTX ctx;

    SHA1Init(&ctx);
    SHA1Update(&ctx, buf, size);
    SHA1Final(hash, &ctx);
}

uint32_t
AppleEffaceableStorage::crc32Sys(uint32_t crc, const void * buf, uint32_t size)
{
    return crc32(crc, buf, size);
}

bool
AppleEffaceableStorage::setPropertySys(const char * key, uint32_t value)
{
    return setProperty(key, value, 32);
}

void
AppleEffaceableStorage::panicSys(const char * msg)
{
    panic(msg);
}

int
AppleEffaceableStorage::vlogfSys(const char * fmt, va_list ap)
{
    IOLogv(fmt, ap);

    // XXX Given that IOLogv doesn't return character printed count,
    // as least common denominator, it's probably best to reduce vlogf
    // contract to void return value.
    return 0;
}

// =============================================================================
