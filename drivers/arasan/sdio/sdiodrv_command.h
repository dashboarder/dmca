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

#ifndef _SDIODRV_COMMAND_H
#define _SDIODRV_COMMAND_H


#include <sys/types.h>
#include <sdiocommon/sdhc_registers.h>


struct SDIOCommand;
struct SDIOCommandResponse;

/** @brief Sends an SDIO Command.
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] commandIndex
 *	The number of the command to perform.
 * @param[in] commandArg
 *	The command argument.
 * @param[out] response
 *	The response to the SDIO command.
 * @return
 *	kSDIOReturnSuccess         - on success
 *	kSDIOReturnCmdLineBusy     - Command line already in use
 *	kSDIOReturnNoCmdComplete   - Command complete status never set in SDHC
 *	kSDIOReturnCmdLineConflict - Line voltage doesn't match what was driven
 *	kSDIOReturnCmdTimeout      - SDHC timed out waiting for command response
 *	kSDIOReturnCmdCRCError     - SDHC detected a CRC error in command
 *	kSDIOReturnCmdEndBitError  - SDHC detected an end-bit error in command
 *	kSDIOReturnCmdIndexError   - Command index in response doesn't match request
 */
SDIOReturn_t sdiodrv_sendCommand(SDHCRegisters_t *sdhc,
								 UInt16 commandIndex,
								 UInt32 commandArg,
								 struct SDIOCommandResponse *response);

/** @brief  Sends an SDIO Command.
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] command
 *	The command parameters.
 * @param[out] response
 *	The response to the SDIO command.
 * @return
 *	kSDIOReturnSuccess         - on success
 *	kSDIOReturnCmdLineBusy     - Command line already in use
 *	kSDIOReturnNoCmdComplete   - Command complete status never set in SDHC
 *	kSDIOReturnCmdLineConflict - Line voltage doesn't match what was driven
 *	kSDIOReturnCmdTimeout      - SDHC timed out waiting for command response
 *	kSDIOReturnCmdCRCError     - SDHC detected a CRC error in command
 *	kSDIOReturnCmdEndBitError  - SDHC detected an end-bit error in command
 *	kSDIOReturnCmdIndexError   - Command index in response doesn't match request
 */
SDIOReturn_t sdiodrv_sendSDIOCommand(SDHCRegisters_t *sdhc,
									 const struct SDIOCommand *command,
									 struct SDIOCommandResponse *response);


#endif /* _SDIODRV_COMMAND_H */

