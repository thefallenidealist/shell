// Created 190928
// Simple shell which can be later used as UART shell on STM32
// Point is to have a way to execute internal commands, like gpio_set(),
// adc_read() and others to test its functionality without recompiling every
// minute.

#include <stdio.h>
#include <stdint.h>
#include "./src/3rd_party/strtok.c"	// copied from FreeBSD source
#include <stdlib.h>		// 191111 exit()
#include "./src/colors_ansi.h"

#ifdef DEBUG
#define dprintf(fmt, ...) \
	do { if (DEBUG) printf(ANSI_COLOR_YELLOW "DBG INFO %s:%d %s(): " \
	ANSI_COLOR_RESET fmt, __FILE__, __LINE__, __func__,\
	##__VA_ARGS__); } while (0)
#else
#define dprintf(...)
#endif

#define SHELL_NAME			"ush"
#define SHELL_PROMPT_SIGN	"%%"
#define DELIM				" "
#define MAX_ARGV			10		// max args in one line (depends on DELIM)
#define LINE_BUFFER_SIZE	255		// max chars in one line

// helpers:
static void print_argv(char **buffer);
static void remove_char(char *input, char to_remove);

static void print_prompt(void);
static char *read_line(void);
static char **split_line(char *line);
static uint8_t execute(char *argv[]);

uint8_t cmd_help(char *argv[]);
uint8_t cmd_echo(char *argv[]);
uint8_t cmd_exit(char *argv[]);

char *splitted[MAX_ARGV];	// commands after splliting on DELIM
char input[LINE_BUFFER_SIZE];		// UART RX will populate this

// types								 									{{{
// ----------------------------------------------------------------------------
typedef uint8_t (*cmd_fn)(char **);
typedef struct
{
	char		*name;
	cmd_fn		pfn;
} sh_cmd_t;

sh_cmd_t sh_cmd[] =
{
	{"help",		&cmd_help},
	{"echo",		&cmd_echo},
	{"exit",		&cmd_exit},
};
uint8_t NUMBER_OF_COMMANDS = sizeof(sh_cmd)/sizeof(sh_cmd[0]);

typedef enum
{
	ERET_SPLITLINE_ARGV_OVERRUN = -1,
} return_codes;
// ------------------------------------------------------------------------ }}}

// helpers								 									{{{
// ----------------------------------------------------------------------------
// print argv[] until null
static void print_argv(char **buffer)
{
#ifdef DEBUG
	dprintf("------------\n");
	uint8_t n = 0;

	while (buffer[n] != NULL)
	{
		printf("buffer[%d]: %s\n", n, buffer[n]);
		n++;
	}
	dprintf("------------\n");
#endif
}

static void remove_char(char *input, char to_remove)
{
	char *src, *dst;
	for (src = dst = input; *src != '\0'; src++)
	{
		*dst = *src;
		if (*dst != to_remove)
		{
			dst++;
		}
	}
	*dst = '\0';
}
// ------------------------------------------------------------------------ }}}

// shell main functions - read_line, split_line, execute					{{{
// ----------------------------------------------------------------------------
static void print_prompt(void)
{
	printf(SHELL_NAME SHELL_PROMPT_SIGN " ");
}

// ----------------------------------------------------------------------------
static char *read_line(void)
{
	// printf("input string: ");
	fgets(input, 100, stdin);
	// scanf("---> %s\n", input);
	// printf("input string: %s", input);
	remove_char(input, '\n');
	return input;
}

// ----------------------------------------------------------------------------
// input: string
// ouput: array of pointers to original string
static char **split_line(char *line)
{
	char *token;
	char *rest = line;
	uint8_t n = 0;

	while ((token = strtok_r(rest, DELIM, &rest)))
	{
		splitted[n++] = token;
		if (n == MAX_ARGV)
		{
			printf("Error, no more place for argv[%d], max is %d\n", n+1, MAX_ARGV);
			exit(ERET_SPLITLINE_ARGV_OVERRUN);	// TODO 191110: make it to return only from this function, don't kill whole shell
		}
	}

	print_argv(splitted);

	return splitted;
}

// ----------------------------------------------------------------------------
static uint8_t execute(char *argv[])
{
	if (argv[0] == NULL)
	{
		// An empty command was entered.
		printf("---> %s() END empty command\n", __func__);
		return 1;
	}

	if (*argv[0] == 0)
	{
		// ist just enter press, don't try to execute it
		return 1;
	}

	for (uint8_t i = 0; i < NUMBER_OF_COMMANDS; i++)
	{
		// execute if found
		if (strcmp(argv[0], sh_cmd[i].name) == 0)
		{
			// printf("---> %s() found function: %s\n", __func__, sh_cmd[i].name);
			uint8_t status = sh_cmd[i].pfn(argv);
			return status;
		}
	}

	printf(SHELL_NAME ": Command not found: \"%s\"\n", argv[0]);
	memset(input, 0, sizeof(input));
	memset(splitted, 0, sizeof(splitted));

	return 1;
}
// ------------------------------------------------------------------------ }}}

// shell commands															{{{
// ----------------------------------------------------------------------------
uint8_t cmd_help(char *argv[])
{
	printf(SHELL_NAME " help, available commands:\n");
	for (uint8_t i = 0; i < NUMBER_OF_COMMANDS; i++)
	{
		printf("  %s\n", sh_cmd[i].name);
	}

	return 1;
}

uint8_t cmd_echo(char *argv[])
{
	printf("%s\n", argv[1]);
	return 1;
}

uint8_t cmd_exit(char *argv[])
{
	return 0;
}
// ------------------------------------------------------------------------ }}}

int main(int argc, const char *argv[])
{
	char *line;
	char **argv_split;		// will be something like: "cmd arg1 arg2 arg3 null"
	uint8_t status;

	printf(SHELL_NAME " here, use help command\n");
	do
	{
		print_prompt();
		line = read_line();
		argv_split = split_line(line);
		status = execute(argv_split);
	} while (status);

	return 0;
}
