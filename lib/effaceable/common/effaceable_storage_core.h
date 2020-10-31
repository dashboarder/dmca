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

#ifndef _EFFACEABLE_STORAGE_CORE_H_
#define _EFFACEABLE_STORAGE_CORE_H_

// =============================================================================

__BEGIN_DECLS

// -----------------------------------------------------------------------------

static void updateInitialProperties(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static uint32_t extGetCopiesPerUnit(effaceable_storage_t * storage);
static uint32_t extGetCopySize(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static uint32_t getCopiesPerUnit(effaceable_storage_t * storage);
static uint32_t getCopySize(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static EffaceableReturn getLocker(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted);
static EffaceableReturn getLockerExternal(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted);
static EffaceableReturn getLockerInternal(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted, bool notprivate);
static EffaceableReturn setLocker(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted);
static EffaceableReturn setLockerExternal(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted);
static EffaceableReturn setLockerInternal(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted, bool notprivate);
static EffaceableReturn spaceForLocker(effaceable_storage_t * storage, uint32_t type_id, uint32_t *data_space);
static EffaceableReturn effaceLocker(effaceable_storage_t * storage, uint32_t type_id, bool untrusted);
static EffaceableReturn effaceLockerExternal(effaceable_storage_t * storage, uint32_t type_id, bool untrusted);
static EffaceableReturn effaceLockerInternal(effaceable_storage_t * storage, uint32_t type_id, bool untrusted, bool notprivate);
static EffaceableReturn getBytes(effaceable_storage_t * storage, void * client_buf, uint32_t offset, uint32_t count);
static EffaceableReturn wipeStorage(effaceable_storage_t * storage, void * tmp_buf);
static uint32_t getCapacity(effaceable_storage_t * storage);
static bool isFormatted(effaceable_storage_t * storage);
static EffaceableReturn formatStorage(effaceable_storage_t * storage);
static EffaceableReturn generateNonce(effaceable_storage_t * storage, void * hash);
static EffaceableReturn consumeNonce(effaceable_storage_t * storage, uint64_t * nonce);

// -----------------------------------------------------------------------------

static EffaceableReturn eraseGroupLogged(effaceable_storage_t * storage, uint32_t group);
static EffaceableReturn writeUnitLogged(effaceable_storage_t * storage, const void * io_buf, uint32_t group, uint32_t unit);
static EffaceableReturn readUnitLogged(effaceable_storage_t * storage, void * io_buf, uint32_t group, uint32_t unit);

// -----------------------------------------------------------------------------

static void findCurrentClone(effaceable_storage_t * storage, void * tmp_buf, void * scan_buf, void * clone_buf);
static bool isGroupEffaced(effaceable_storage_t * storage, void * tmp_buf, uint32_t group);
static EffaceableReturn obscureCopy(effaceable_storage_t * storage, void * clear_buf, void * crypt_buf, uint32_t copy);
static EffaceableReturn revealCopy(effaceable_storage_t * storage, void * crypt_buf, void * clear_buf, uint32_t copy);
static EffaceableReturn commitClone(effaceable_storage_t * storage, void * clone_buf, uint32_t group);
static EffaceableReturn writeClone(effaceable_storage_t * storage, void * tmp_buf, void * clone_buf, uint32_t group, uint32_t generation);
static uint32_t checksumCopy(effaceable_storage_t * storage, void * clone_buf);
static bool isCopyValid(effaceable_storage_t * storage, const void * clone_buf);
static EffaceableReturn confirmClone(effaceable_storage_t * storage, void * tmp_buf, void * clone_buf, uint32_t group);

// -----------------------------------------------------------------------------

static uint32_t nextGroup(effaceable_storage_t * storage, uint32_t group);
static EffaceableReturn effaceGroup(effaceable_storage_t * storage, void * tmp_buf, uint32_t group);

// -----------------------------------------------------------------------------

static locker_t * findLockerForRequest(effaceable_storage_t * storage, uint32_t type_id);
static EffaceableReturn discardLockers(effaceable_storage_t * storage);
static void appendSentinel(effaceable_storage_t * storage);
static locker_t * findLockerForRequest(effaceable_storage_t * storage, uint32_t type_id);
static EffaceableReturn getLockersFromStorage(effaceable_storage_t * storage);
static EffaceableReturn putLockersInStorage(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static void cleanBuffers(effaceable_storage_t * storage);
static void foundCurrentCloneInGroup(effaceable_storage_t * storage, uint32_t group);
static void updateDynamicProperties(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

static void initContract(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static inline void * getCache(effaceable_storage_t * storage);
static inline void * setCache(effaceable_storage_t * storage, void * buf);

static inline void * getScratch(effaceable_storage_t * storage);
static inline void * setScratch(effaceable_storage_t * storage, void * buf);

static inline void * getSniff(effaceable_storage_t * storage);
static inline void * setSniff(effaceable_storage_t * storage, void * buf);

static inline void * getCrypt(effaceable_storage_t * storage);
static inline void * setCrypt(effaceable_storage_t * storage, void * buf);

static inline void * getNonceHash(effaceable_storage_t * storage);
static inline void * setNonceHash(effaceable_storage_t * storage, void * buf);

static inline uint32_t getGeneration(effaceable_storage_t * storage);
static inline uint32_t setGeneration(effaceable_storage_t * storage, uint32_t generation);

static inline uint32_t getGroup(effaceable_storage_t * storage);
static inline uint32_t setGroup(effaceable_storage_t * storage, uint32_t group);

static inline bool getIsFormatted(effaceable_storage_t * storage);
static inline bool setIsFormatted(effaceable_storage_t * storage, bool is_formatted);

static inline bool getFullScanEnabled(effaceable_storage_t * storage);
static inline bool setFullScanEnabled(effaceable_storage_t * storage, bool enable);

static inline bool getWipeEnabled(effaceable_storage_t * storage);
static inline bool setWipeEnabled(effaceable_storage_t * storage, bool enable);

static inline locker_list_t * getLockerList(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static inline uint32_t getGroupCount(effaceable_storage_t * storage);
static inline uint32_t getUnitsPerGroup(effaceable_storage_t * storage);
static inline uint32_t getUnitSize(effaceable_storage_t * storage);
static inline bool eraseGroup(effaceable_storage_t * storage, uint32_t group);
static inline bool writeUnit(effaceable_storage_t * storage, const void * buf, uint32_t group, uint32_t unit);
static inline bool readUnit(effaceable_storage_t * storage, void * buf, uint32_t group, uint32_t unit);

// -----------------------------------------------------------------------------

static inline void deviceCleanBuffers(effaceable_storage_t * storage);
static inline void deviceFoundCurrentCloneInGroup(effaceable_storage_t * storage, uint32_t group);
static inline void deviceUpdateDynamicProperties(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

static inline void * allocMem(effaceable_storage_t * storage, uint32_t size);
static inline void freeMem(effaceable_storage_t * storage, void * buf, uint32_t size);
static inline void * setMem(effaceable_storage_t * storage, void * buf, uint8_t val, uint32_t size);
static inline void * moveMem(effaceable_storage_t * storage, void * dst, const void * src, uint32_t size);
static inline int cmpMem(effaceable_storage_t * storage, const void * lhs, const void * rhs, uint32_t size);
static inline void readRandom(effaceable_storage_t * storage, void * buf, uint32_t size);
static inline void calcSHA1(effaceable_storage_t * storage, const void * buf, uint32_t size, void * hash);
static inline uint32_t crc32Sys(effaceable_storage_t * storage, uint32_t crc, const void * buf, uint32_t size, bool finish);
static inline bool setPropertySys(effaceable_storage_t * storage, const char * key, uint32_t value);
static inline void panicSys(effaceable_storage_t * storage, const char * msg);

static int logf(effaceable_storage_t * storage, const char * fmt, ...);

// -----------------------------------------------------------------------------

static void initContext(effaceable_storage_t * storage, effaceable_device_t * device, effaceable_system_t * system);

// -----------------------------------------------------------------------------

__END_DECLS

// =============================================================================

#endif /* _EFFACEABLE_STORAGE_CORE_H_ */
