/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <platform.h>
#include <platform/memmap.h>
#include <drivers/power.h>
#include <lib/env.h>

#if WITH_MENU
#if WITH_SIMPLE_MENU

#define kTokenBufSize   128
#define kMaxTokens      8

utime_t gMenuLastActivityTime;
static char *token_buf;

struct task_event gMenuTaskReadyEvent = EVENT_STATIC_INIT(gMenuTaskReadyEvent, false, 0);

/*
 * Parse tokens from (buf).
 */
static int
tokenize(const char *buf, char **tokens, int token_count)
{
        const char      *in;
        char            *out, *out_limit;
        int             token;
        enum {
                WHITESPACE,
                TOKEN,
                DONE
        }               state;

        in = buf;
        out = token_buf;
        out_limit = out + kTokenBufSize - 1;
        token = 0;
        state = WHITESPACE;
        
        for (;;) {
                /* state machine */
                switch(state) {
                case WHITESPACE:
                        /* if we hit the end of the input, bail now */
                        if ('\0' == *in) {
                                state = DONE;
                                break;
                        }

                        /* found a token starting? */
                        if (!isspace(*in)) {
                                tokens[token] = out;
                                state = TOKEN;
                                break;
                        }
                        /* discard and move on */
                        in++;
                        break;
                                
                case TOKEN:
                        /* output cursor has reached or exceeded the last safe character */
                        if (out >= out_limit) {
                                /* terminate at the last safe character */
                                *out_limit = '\0';
                                /* update the token count */
                                token++;
                                /* and we are done */
                                state = DONE;
                                break;
                        }

                        /* input cursor has reached the end of the input or whitespace */
                        if (('\0' == *in) || isspace(*in)) {
                                *out++ = '\0';
                                token++;

                                /* if the input is exhausted or we have run out of token space */
                                if (('\0' == *in) || (token >= token_count)) {
                                        state = DONE;
                                } else {
                                        /* return to discarding whitespace */
                                        state = WHITESPACE;
                                }
                                break;
                        }

                        /* copy another character */
                        *out++ = *in++;
                        break;
 
                case DONE:
                        return(token);
                }
        }
}


bool
process_command_line(char *line, int *ret_val)
{
        char                    *tokens[kMaxTokens];
        struct cmd_arg          args[kMaxTokens]; 
	const struct cmd_menu_item *menu_entry;
	void                    **menu_cursor;
	int                     argnum;
        int                     token_count;
        int                     ret;

        /* don't allow ourselves to be called before the menu task has started */
        if (NULL == token_buf)
                return(true);
        
	/* Update activity; we might have come from outside the get_line() loop */
	gMenuLastActivityTime = system_time();

        /* tokenize, will never return > kMaxTokens */
        token_count = tokenize(line, tokens, kMaxTokens);

        if (token_count > 0) {
		LINKER_SET_FOREACH(menu_cursor, menu) {
			menu_entry = (const struct cmd_menu_item *)*menu_cursor;
			if (!strcmp(menu_entry->cmd, tokens[0])) {

				/* parse the arglist */
				for (argnum = 0; argnum < token_count; argnum++) {
					/* are we done? */
					if (argnum >= token_count)
						break;

					args[argnum].n = strtol(tokens[argnum], NULL, 0);
					args[argnum].u = strtoul(tokens[argnum], NULL, 0);
					args[argnum].h = strtol(tokens[argnum], NULL, 16);
					if (!strcmp(tokens[argnum], "true"))
						args[argnum].b = true;
					else if (!strcmp(tokens[argnum], "false"))
						args[argnum].b = false;
					else
						args[argnum].b = args[argnum].n ? true : false;
					args[argnum].str = tokens[argnum];
				}
					
				/* call the routine */
				ret = menu_entry->func(argnum, args);
				if (ret_val)
					*ret_val = ret;
#if WITH_ENV
				/* stick the result in an environment variable */
				env_set_uint("?", ret, 0);
#endif
                                return(false);
			}
		}
        }
        return(true);
}


int menu_task(void *arg)
{

	/* allocate the token buffer */
	token_buf = malloc(kTokenBufSize);

	/* signal others waiting for us */
	event_signal(&gMenuTaskReadyEvent);

        /* spin throwing away console input */
	for (;;) {

                /* get an input character */
                (void)getchar();

		/* Update activity time on any new input. */
		gMenuLastActivityTime = system_time();
        }
        
	return 0;
}

#endif /* WITH_SIMPLE_MENU */
#endif /* WITH_MENU */
