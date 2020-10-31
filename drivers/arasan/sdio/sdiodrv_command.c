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

#include "sdiodrv_command.h"

#include <sys/task.h>

#include <debug.h>
#include <AssertMacros.h>

#include "sdiocommon/sdhc_debug.h"
#include "sdiocommon/sdio_cmdprop.h"
#include "sdiodrv_config.h"



#define CMD_INHIBITED_CHECKS	(20)
#define CMD_INHIBITED_WAIT		(5)
#define DATA_INHIBITED_CHECKS	(20)
#define DATA_INHIBITED_WAIT		(5)
#define COMMAND_COMPLETE_CHECKS (1000)


static SDIOReturn_t
sdiodrv_prepareCommand(const SDHCRegisters_t *sdhc)
{
	if(sdhc_isCommandInhibitedOnCmd(sdhc)) {
		dprintf(DEBUG_CRITICAL, "SDHC @ %p: Command inhibited!\n", sdhc);
		
		for(unsigned int i=0; sdhc_isCommandInhibitedOnCmd(sdhc) && i < CMD_INHIBITED_CHECKS; i++) {
			task_sleep(CMD_INHIBITED_WAIT);
		}
		
		if(sdhc_isCommandInhibitedOnCmd(sdhc)) {
			return kSDIOReturnCmdLineBusy;
		}
	}

	// TODO: Handle busy and abort
/*	if(issueWithBusy && !issueAbortCommand) {
		if(sdhc_isCommandInhibitedOnData(sdhc)) {
			dprintf(DEBUG_CRITICAL, "SDHC @ %p: Data inhibited!\n", sdhc);
			
			for(unsigned int i=0; sdhc_isCommandInhibitedOnData(sdhc) && i < DATA_INHIBITED_CHECKS; i++) {
				task_sleep(DATA_INHIBITED_WAIT);
			}
			
			if(sdhc_isCommandInhibitedOnData(sdhc)) {
				return kSDIOReturnDataLineBusy;
			}
		}
	}
*/	
	return kSDIOReturnSuccess;
}



static SDIOReturn_t
sdiodrv_completeCommand(SDHCRegisters_t *sdhc, struct SDIOCommandResponse *response)
{
	SDIOReturn_t retval = sdhc_getCommandStatus(sdhc);

	if(kSDIOReturnSuccess != retval) {
		dprintf(DEBUG_CRITICAL, "%s: SDIO Cmd Failed (0x%X): cmd = 0x%X, arg = 0x%X, resp0 = 0x%X, normalInt = 0x%X, errorInt = 0x%X, state = 0x%X\n",  __func__,
			retval, sdhc->command, sdhc->argument, sdhc->response[0],
			sdhc->normalInterruptStatus, sdhc->errorInterruptStatus, sdhc->presentState);
		
		sdiodrv_resetSDHC(sdhc, kSDHCResetCmdLine);
	}

	sdhc_clearCommandStatus(sdhc);
	
	sdhc_copyCommandResponse(sdhc, response);
	
	return retval;
}

SDIOReturn_t
sdiodrv_sendSDIOCommand(SDHCRegisters_t *sdhc,
					   const struct SDIOCommand *command,
					   struct SDIOCommandResponse *response)
{
	check(command);
	
	return sdiodrv_sendCommand(sdhc, command->index, command->argument, response);
}

SDIOReturn_t
sdiodrv_sendCommand(SDHCRegisters_t *sdhc, UInt16 commandIndex, UInt32 commandArg, struct SDIOCommandResponse *response)
{
	check(sdhc);
	check(response);
	
	SDIOReturn_t retval = sdiodrv_prepareCommand(sdhc);
	if(kSDIOReturnSuccess != retval) {
		return retval;
	}

	sdhc->argument = commandArg;
	
	UInt16 command = sdio_generateCommand(commandIndex);

//	dprintf(DEBUG_INFO, "%s: cmd = 0x%04X, arg = 0x%08X\n", __func__,
//		command, sdhc->argument);
//	sdhc_dumpRegisterFile(sdhc);

	sdhc->command = command;

	for(unsigned int i=0; !sdhc_isCommandComplete(sdhc); i++) {
		if(i >= COMMAND_COMPLETE_CHECKS) {
			dprintf(DEBUG_CRITICAL, "%s: No command complete after %u checks: cmd = 0x%X, arg = 0x%X, normalInt = 0x%X, errorInt = 0x%X, state = 0x%X\n",
				__func__, COMMAND_COMPLETE_CHECKS,
				sdhc->command, sdhc->argument,
				sdhc->normalInterruptStatus, sdhc->errorInterruptStatus, sdhc->presentState);
			
			sdiodrv_resetSDHC(sdhc, kSDHCResetCmdLine);

			return kSDIOReturnNoCmdComplete;
		}
		
		task_yield();
	}

	retval = sdiodrv_completeCommand(sdhc, response);
	
	return retval;
}



