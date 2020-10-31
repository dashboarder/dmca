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

#ifndef _SDIODRV_CONFIG_H
#define _SDIODRV_CONFIG_H


#include <sys/types.h>
#include <sdiocommon/sdhc_registers.h>
#include <sdiocommon/sdio_types.h>

/** @brief Resets the SDIO Host Controller
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] resetOptions
 *	Flags identifying what to reset
 * @return
 *	kSDIOReturnSuccess     - on success
 *  kSDIOReturnBadArgument - unsupported flag set
 *	kSDIOReturnInReset     - SDHC didn't come out of reset in a resonable time period
 */
SDIOReturn_t sdiodrv_resetSDHC(SDHCRegisters_t *sdhc, SDHCResetFlags_t resetOptions);

/** @brief Sets the SD Clock rate.
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in,out] targetSDClkRateHz
 *	Desired SD Clock rate, in Hz
 *	On exit set to the actual clock rate set in Hz, 0 on error.
 * @param[in] inputClkRateHz
 *	SDHC Block's input clock rate, in Hz. Used to derive SD Clock.
 * @return
 *	kSDIOReturnSuccess               - on success
 *  kSDIOReturnBadArgument           - Illegal target or base rate
 *	kSDIOReturnInternalClockUnstable - Block's internal clock didn't stabilize in a reasonable time period
 */
SDIOReturn_t sdiodrv_setClockRate(SDHCRegisters_t *sdhc, IOSDIOClockRate *targetSDClkRateHz, UInt32 inputClkRateHz);

/** @brief Enable / disable the SD Clock.
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] clockMode
 *	Whether to turn on or turn off the clock.
 * @return
 *	kSDIOReturnSuccess               - on success
 *  kSDIOReturnBadArgument           - Unsupported clock mode
 *	kSDIOReturnInternalClockUnstable - Block's internal clock didn't stabilize in a reasonable time period
 */
SDIOReturn_t sdiodrv_setClockMode(SDHCRegisters_t *sdhc, enum IOSDIOClockMode clockMode);

/** @brief Set the width of the data bus (# of data lines).
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] busWidth
 *	Whether to transfer data in 1 or 4 bit mode.
 * @return
 *	kSDIOReturnSuccess     - on success
 *  kSDIOReturnBadArgument - Unsupported bus width
 */
SDIOReturn_t sdiodrv_setBusWidth(SDHCRegisters_t *sdhc, enum IOSDIOBusWidth busWidth);

/** @brief Set the bus speed used by the Host Controller.
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] speedMode
 *	Whether to transfer data in low or high speed mode.
 * @return
 *	kSDIOReturnSuccess     - on success
 *  kSDIOReturnBadArgument - Unsupported speed mode
 */
SDIOReturn_t sdiodrv_setBusSpeedMode(SDHCRegisters_t *sdhc, enum IOSDIOBusSpeedMode speedMode);


#endif /* _SDIODRV_CONFIG_H */

