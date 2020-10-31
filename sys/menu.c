/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
#if !WITH_SIMPLE_MENU

// 0x1b 0x5b 0x41 uparrow
// 0x1b 0x5b 0x42 downarrow
// 0x1b 0x5b 0x43 rightarrow
// 0x1b 0x5b 0x44 leftarrow

#define HISTORY_SIZE 16
static char (*history)[256];
static uint32_t curr_history_pos = 0;

#define kTokenBufSize (1024)
#define kMaxMenuArgs 64

utime_t gMenuLastActivityTime;
static char *token_buf;
static char *console_prompt;

struct task_event gMenuTaskReadyEvent = EVENT_STATIC_INIT(gMenuTaskReadyEvent, false, 0);

static char *get_line(const char *prompt, char *buf, int buf_size)
{
	int i;
	int escape;
	uint32_t history_pos = curr_history_pos;
	uint32_t saved_history_pos = curr_history_pos;

do_prompt:
	puts(prompt);

	history[curr_history_pos][0] = 0;

	i = 0;
	escape = 0;
	for (;;) {
		char c = getchar();

//		printf("c = 0x%x (%c)\n", c, c);

		/* Update activity time on any new input. */
		gMenuLastActivityTime = system_time();

		switch (c) {
			case '\n':
				escape = 0;
				putchar('\n');
				if (i != 0) {
					buf[i] = 0;

					/* save the history, if it's different from the last command */
					if (strcmp(buf, history[(curr_history_pos - 1) % HISTORY_SIZE]) != 0) {
						strlcpy(history[curr_history_pos], buf, sizeof(history[curr_history_pos]));
						curr_history_pos = (curr_history_pos + 1) % HISTORY_SIZE;
					}
					return buf;
				} else {
					goto do_prompt;
				}
				break;
			case 0x8: // backspace
			case 0x7f: // del
				escape = 0;
				if (i > 0) {
					i--;
					putchar(0x8);
					putchar(0x20);
					putchar(0x8);
				}
				break;
			case 0x1b:
				escape = 1;
				break;
			case 0x5b:
				if (escape > 0)
					escape = 2;
				break;
			case 0x41: case 0x42:
				if (escape < 2) {
					goto regchar;
				} else {
					// up/down arrow
					char *new_history;

					if (c == 0x41) {
						int temp = history_pos;
						history_pos = (history_pos - 1) % HISTORY_SIZE;
						if (history_pos == saved_history_pos) {
//							printf("%d %d\n", history_pos, saved_history_pos);
							// we've wrapped around
							history_pos = temp;
							break;
						}
						if (strlen(history[history_pos]) == 0) {
//							printf("%p %d, len 0\n", history[history_pos], history_pos);
							// we've walked before the start of history
							history_pos = temp;
							break;
						}
					} else {
						if (history_pos == saved_history_pos)
							break; // can't go forward past top of the history
						history_pos = (history_pos + 1) % HISTORY_SIZE;
					}
					new_history = history[history_pos];

					/* wipe out the line */
					while(i > 0) {
						putchar(0x8);
						putchar(' ');
						putchar(0x8);
						i--;
					}

					/* output the new line */
					for (i=0; new_history[i] != 0; i++) {
						buf[i] = new_history[i];
						putchar(new_history[i]);
					}
				}
				escape = 0;
				break;
			case 0x43: case 0x44:
				if (escape < 2) {
					goto regchar;
				} else {
					// left & right arrow
					escape = 0;
				}
				break;
			default:
regchar:
				escape = 0;
				if (i < (buf_size - 1)) {
					buf[i++] = c;
					putchar(c);
				}
		}
	}
}

/* 
 * chop a string into multiple tokens, each pointed to by the tokens array.
 * if an unescaped ';' is hit, stop tokenizing and return the remainder of
 * the line in *line_remainder. If no ';' is hit, return null in *line_remainder.
 */
static int tokenize(const char *buf, char **tokens, int token_count, const char **line_remainder)
{
	bool done;
	bool done_after;
	int inpos = 0;
	int outpos = 0;
	int outpos_token_start = 0;
	int token = 0;
	char last_char = 0;
	enum {
		INITIAL,
		CONSUME_WHITESPACE,
		TOKEN_START,
		TOKEN,
		QUOTED_TOKEN,
		FINISH_TOKEN,

#if WITH_ENV
		ENV_TOKEN,
		FINISH_ENV_TOKEN,
#endif
	} state = INITIAL;

	*line_remainder = NULL;

	done = done_after = false;
	while (!done) {
		/* end of input buffer, finish off whatever token we're in the middle of and exit */
		if ((buf[inpos] == 0) || (outpos >= (kTokenBufSize - 1))) {
			done = true;
			if (state == TOKEN || state == QUOTED_TOKEN || state == FINISH_TOKEN)
				state = FINISH_TOKEN;
#if WITH_ENV
			else if (state == ENV_TOKEN)
				state = FINISH_ENV_TOKEN;
#endif
			else
				break;
		}
		if (done_after)
			done = true;

//		printf("tok: in %d (%c) out %d tok %d state %d\n", 
//				inpos, buf[inpos], outpos, token, state);
		switch (state) {
			case INITIAL:
			case CONSUME_WHITESPACE:
				/* eat whitespace until we see the start of a token */
				if (isspace(buf[inpos])) {
					last_char = buf[inpos];
					inpos++;
					continue;
				}

				state = TOKEN_START;
				break;
			case TOKEN_START:
				/* start of a token, examine the first character to see if we need to do anything special */
				outpos_token_start = outpos;
				if (buf[inpos] == '#') {
					/* comment, ignore the rest of the line */
					last_char = buf[inpos];
					done = true;
					break;
#if WITH_ENV
				} else if (buf[inpos] == '$') {
					/* environment variable */
					last_char = buf[inpos];
					inpos++;
					state = ENV_TOKEN;
					tokens[token] = &token_buf[outpos];
#endif
				} else if (buf[inpos] == ';') {
					/* if we hit a unescaped ';', it's the end of the line */
					*line_remainder = &buf[inpos+1];
					done = true;
					break;
				} else if (buf[inpos] == '"') {
					/* quoted token */
					last_char = buf[inpos];
					inpos++;
					state = QUOTED_TOKEN;
					tokens[token] = &token_buf[outpos];
				} else {
					state = TOKEN;
					tokens[token] = &token_buf[outpos];
				}
				break;
			case TOKEN:
				/* consume characters until we hit whitespace */
				if (isspace(buf[inpos])) {
					/* whitespace, null terminate current token and switch to whitespace consume state */
					last_char = buf[inpos];
					state = FINISH_TOKEN;
					break;
				} else if (buf[inpos] == ';') {
					/* if we hit a unescaped ';', it's the end of the line */
					if (last_char != '\\') {
						*line_remainder = &buf[inpos+1];
						state = FINISH_TOKEN;
						done_after = true;
						break;
					}
				}

				last_char = buf[inpos];
				if (buf[inpos] != '\\')
					token_buf[outpos++] = buf[inpos];
				inpos++;
				break;
			case QUOTED_TOKEN:
				/* consume characters until we hit terminating quote */
				if (buf[inpos] == '"') {
					/* terminating quote, null terminate current token and switch to whitespace consume state */
					inpos++; // consume the quote
					state = FINISH_TOKEN;
					break;
				}

				last_char = buf[inpos];
				token_buf[outpos++] = buf[inpos++];
				break;
			case FINISH_TOKEN:
				/* finish whatever token we're dealing with */
				token_buf[outpos++] = 0;
				if (++token < token_count)
					state = CONSUME_WHITESPACE;
				else {
					*line_remainder = &buf[inpos];
					done = true;
				}
				break;
#if WITH_ENV
			case ENV_TOKEN:
				/* consume characters until we hit whitespace */
				if (isspace(buf[inpos])) {
					/* whitespace, we've now got the environment variable we want to read */
					last_char = buf[inpos];
					state = FINISH_ENV_TOKEN;
					break;
				}

				last_char = buf[inpos];
				token_buf[outpos++] = buf[inpos++];
				break;
			case FINISH_ENV_TOKEN: {
				/* the current token now holds the environment variable we want to read */
				const char *env;
			   
				token_buf[outpos++] = 0;

				/* read in the environment variable */
				env = env_get(tokens[token]);
				if (!env) {
					printf("\ncould not read environment var '%s'\n", tokens[token]);
					return -1;
				}

				/* copy the value of the environment variable on top of the current token */
				strlcpy(&token_buf[outpos_token_start], env, kTokenBufSize - outpos_token_start);
				outpos = outpos_token_start + strlen(token_buf + outpos_token_start) + 1;

				if (++token < token_count)
					state = CONSUME_WHITESPACE;
				else {
					*line_remainder = &buf[inpos];
					done = true;
				}
				break;
			}
#endif
			default:
				panic("tokenize: got into an invalid state\n");
		}
	}

	return token;
}

bool process_command_line(char *line, int* ret_val)
{
	char *tokens[kMaxMenuArgs];
 	struct cmd_arg args[kMaxMenuArgs];
	int token_count;
	int ret = 0;
	const struct cmd_menu_item *menu_entry = NULL;
	void **menu_cursor;
	int argnum;

        /* don't allow ourselves to be called before the menu task has started */
        if (NULL == token_buf)
                return(false);

	/* Update activity; we might have come from outside the get_line() loop */
	gMenuLastActivityTime = system_time();

	while(line) {
	    	memset(tokens, 0, sizeof(tokens));
		memset(args, 0, sizeof(args));
		token_count = tokenize(line, tokens, sizeof(tokens)/sizeof(char *), (const char **)&line);
		if (token_count <= 0)
			continue;

//		printf("buf %p, line %p\n", buf, line);
//		for(i=0; i < token_count; i++)
//			printf("\t%d: '%s'\n", i, tokens[i]);

		/* if the command starts with '?', only execute it if the last command returned a positive error */
		if (tokens[0][0] == '?') {
			tokens[0]++;
			if (ret < 0)
				continue;
		}

		menu_entry = NULL;
		LINKER_SET_FOREACH(menu_cursor, menu) {
			menu_entry = (const struct cmd_menu_item *)*menu_cursor;
			if (!strcmp(menu_entry->cmd, tokens[0])) {

				/* parse the arglist */
				for (argnum = 0; argnum < kMaxMenuArgs; argnum++) {
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
				break;
			} else {
				menu_entry = NULL;
			}
		}
	}

	return (menu_entry == NULL);	
}

int menu_task(void *arg)
{
	char *buf;

	/* allocate the history buffer */
	history = calloc(1, HISTORY_SIZE * sizeof(*history));	

	/* allocate the line buffer */
	buf = malloc(512);

	/* allocate the token buffer */
	token_buf = malloc(kTokenBufSize);

	/* signal others waiting for us */
	event_signal(&gMenuTaskReadyEvent);

	menu_prompt(NULL);
	
	for (;;) {
		/* prompt */
		puts("\x1b[m");
		get_line(console_prompt, buf, 512);

		if (process_command_line(buf, NULL)) {
			printf("\x1b[1m?SYNTAX ERROR\n");
		}
	}

	return 0;
}

char *menu_prompt(char *new_prompt)
{
	char *ret;

	ret = console_prompt;
	if (NULL != new_prompt) {
		console_prompt = new_prompt;
	} else {
		console_prompt = "] ";
	}
	return(ret);
}

static int do_help(int argc, struct cmd_arg *args)
{
	const struct cmd_menu_item *menu_entry;
	void **menu_cursor;

	printf("command list:\n");
	LINKER_SET_FOREACH(menu_cursor, menu) {
		menu_entry = (const struct cmd_menu_item *)*menu_cursor;
		if (menu_entry->help)
			printf("        %-16.16s %s\n", menu_entry->cmd, menu_entry->help);
	}

	return 0;
}

MENU_COMMAND_DEVELOPMENT(help, do_help, NULL, NULL);

#if 0
static int do_argtest(int argc, struct cmd_arg *args)
{
	int i;

	printf("argtest, %d args:\n", argc);
	for (i=0; i<argc; i++) {
		printf("%d: n %d u %u h %x str %p (%s)\n", i, args[i].n, args[i].u, args[i].h, args[i].str, args[i].str);
	}

	return 0;
}

MENU_COMMAND_DEBUG(argtest, do_argtest, NULL, NULL);
#endif

#endif /* !WITH_SIMPLE_MENU */
#endif /* WITH_MENU */
