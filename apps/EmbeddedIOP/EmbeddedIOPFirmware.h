/*
 * Copyright (C) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*                                                                                      EATLINE
 * This file is consumed to auto-generate the firmware specfication.                    EATLINE
 *                                                                                      EATLINE
 * A line which contains the word EATLINE will be removed.                              EATLINE
 *                                                                                      EATLINE
 * Substitution is performed on comment lines beginning with SUBST.  Fields             EATLINE
 * on these lines are separated by ':' characters.                                      EATLINE
 * The second field on the line determines the form of substitution, and the fifth      EATLINE
 * field supplies termination.                                                          EATLINE
 *                                                                                      EATLINE
 * ENV          Declares and initialiases a scalar variable with value taken from the   EATLINE
 *              compile-time environment.  The variable preamble is read                EATLINE
 *              from the third field, and the environment variable name from            EATLINE
 *              the fourth.                                                             EATLINE
 *                                                                                      EATLINE
 * ENVSTR       Declares and initialiases a string variable with value taken from the   EATLINE
 *              compile-time environment.  The variable preamble is read                EATLINE
 *              from the third field, and the environment variable name from            EATLINE
 *              the fourth.                                                             EATLINE
 *                                                                                      EATLINE
 * VAR          Declares a variable with value taken from the object file.              EATLINE
 *              The variable declaration is read from the third field, and              EATLINE
 *              the symbol name to be looked up in the object file is read              EATLINE
 *              from the fourth.                                                        EATLINE
 *                                                                                      EATLINE
 * FILE         The binary file (named in the compile-time environment) is              EATLINE
 *              inserted in the form of lines of 16 comma-separated 8-bit               EATLINE
 *              hex constants, suitable for use as initialisers for an array            EATLINE
 *              of unsigned char.                                                       EATLINE
 *                                                                                      EATLINE */


/*SUBST:ENVSTR:static const char *fwFirmwareTarget __attribute__((used)) = :SUB_TARGET:;:*/
/*SUBST:ENVSTR:static const char *fwFirmwareBuild __attribute__((used)) = :BUILD:;:*/
/*SUBST:ENVSTR:static const char *fwFirmwareVersion __attribute__((used)) = :XBS_BUILD_TAG:;:*/
/*SUBST:VAR:static unsigned long fwConfigurationOffset __attribute__((used)) = :__iop_config:;:*/
/*SUBST:VAR:static unsigned long fwHeapBaseOffset __attribute__((used)) = :_heap_base:;:*/
/*SUBST:VAR:static unsigned long fwHeapSizeOffset __attribute__((used)) = :_heap_size:;:*/
/*SUBST:VAR:static unsigned long fwPageTableBase __attribute__((used)) = :_tt:;:*/
/*SUBST:VAR:static unsigned long fwPageTableSize __attribute__((used)) = :_tt_size:;:*/

/* IOP panic debug support */
/*SUBST:VAR:static unsigned long fwPanicStr __attribute__((used)) = :_gPanicStr:;:*/
/*SUBST:VAR:static unsigned long fwPanicFunc __attribute__((used)) = :_gPanicFunc:;:*/
/*SUBST:VAR:static unsigned long fwPanicLog __attribute__((used)) = :_gIOPPanicLog:;:*/
/*SUBST:VAR:static unsigned long fwPanicBytes __attribute__((used)) = :_gIOPPanicBytes:;:*/
static unsigned long fwPanicBytesMax __attribute__((used)) = 
/*SUBST:ENV::IOP_PANIC_LOG_SIZE:;:*/

/*EATLINE XXX this may no longer need the section attribute... */
static unsigned char fwFirmwareImage[] __attribute__ ((aligned(4096))) __attribute__ ((section("__DATA,fwFirmwareImage"))) = {
/*SUBST:FILE:*/
};

/* heap requirements from the IOP configuration */
static unsigned long fwHeapRequired __attribute__((used)) =
/*SUBST:ENV::IOP_HEAP_REQUIRED:;:*/

/* message channel size */
static unsigned long fwMessageChannelSize __attribute__((used)) =
/*SUBST:ENV::IOP_MESSAGE_CHANNEL_SIZE:;:*/
    
#endif /* _DO_NOT_DEFINE *//* EATLINE */

