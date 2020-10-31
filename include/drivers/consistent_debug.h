/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_CONSISTENT_DEBUG__H
#define __DRIVERS_CONSISTENT_DEBUG__H


#include <consistent_debug/consistent_debug_registry.h>
#include <consistent_debug/consistent_debug_helper_fns.h>

// Functions

// Restores consistent debug state on resume from sleep.
int consistent_debug_resume(void);

// Set up consistent debug region on cold boot.
int consistent_debug_init(void);


/**
 * Returns the location of the Consistent Debug Registry
 * 
 * @author jdong (1/25/2013)
 * 
 * @return A pointer to the debug registry if it exists. NULL if 
 *         not yet initialized.
 */
dbg_registry_t* consistent_debug_get_registry(void);

/**
 * Attempt to register a header with the supplied information. 
 * 
 * @author jdong (12/10/2012)
 * 
 * @param hdr: A filled-out header. The debug registry will be 
 *           filled in with this information.
 * 
 * @return dbg_record_header_t*: NULL if unsuccessful. 
 *         Otherwise, a pointer to the entry in the debug
 *         registry. It's already filled out, so in the general
 *         case one would just check if it's non-NULL.
 */
dbg_record_header_t* consistent_debug_register_header(dbg_record_header_t hdr);


/**
 * Attempt to deregister a consistent debug header. This might 
 * have to be done when handing off from a bootloader to an OS, 
 * or when powering down / resetting a coprocessor. It ensures 
 * that consumers of a debug region are given a chance to clean 
 * up before it is no longer valid. 
 *  
 * Note the return value. One should initially check that a call 
 * returns "0" indicating an entry is valid and has been marked 
 * as being freed. Then, one should poll by calling again with 
 * the same argument until the return value is 1. 
 * 
 * @author jdong (12/14/2012)
 * 
 * @param hdr A pointer to a debug header returned by 
 *            consistent_debug_register_header()
 * 
 * @return -1: Error has occurred. Most likely the value passed 
 *         is not an in-use debug registry header.
 *          0: Record is marked as being freed -- waiting for
 *          the other end to acknowledge it. Check again later.
 *          1: Record has been freed and is now unused.
 */
int consistent_debug_unregister_header(dbg_record_header_t *hdr);




/**
 * Update the AP's Progress Report with the given state and 
 * state argument. 
 * 
 */
void consistent_debug_update_ap_cpr(cp_state_t state, int arg);

#endif
