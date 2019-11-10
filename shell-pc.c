#include <stdio.h>
#include <stdint.h>
#include "./src/3rd_party/strtok.c"	// copied from FreeBSD source

#define SHELL_NAME	"ush"

static void print_argv(char **buffer);
static char **split_line(char *line);
static void shell_loop(void);

#define DELIM " "
#define MAX_ARGV 10
#define LINE_BUFFER_SIZE 100
char *splitted[MAX_ARGV];	// commands after splliting on DELIM
char input[LINE_BUFFER_SIZE];		// UART RX will populate this
uint8_t cmd_help(char *argv[]);
uint8_t cmd_echo(char *argv[]);
uint8_t cmd_exit(char *argv[]);

typedef uint8_t (*cmd_fn)(char **);


// print argv[] until null
static void print_argv(char **buffer)
{
	printf("------------ %s()\n", __func__);
	uint8_t n = 0;

	while (buffer[n] != NULL)
	{
		printf("buffer[%d]: %s\n", n, buffer[n]);
		n++;
	}
	// printf("------------ %s()\n", __func__);
}

// input: string
// ouput: array of pointers to original string
static char **split_line(char *line)
{
	char *token;
	char *rest = line;
	uint8_t n = 0;

	// while ((token = strtok_r(rest, " ", &rest)))
	while ((token = strtok_r(rest, DELIM, &rest)))
	{
		splitted[n++] = token;
	}

	return splitted;
}

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




uint8_t cmd_help(char *argv[])
{
	// printf("%s() here, argv[0]: %s argv[1]: %s\n", __func__, argv[0], argv[1]);
	printf(SHELL_NAME " help, available commands:\n");
	for (uint8_t i = 0; i < NUMBER_OF_COMMANDS; i++)
	{
		printf("  %s\n", sh_cmd[i].name);
	}

	return 1;
}

uint8_t cmd_echo(char *argv[])
{
	// printf("%s() here, argv[0]: %s argv[1]: %s\n", __func__, argv[0], argv[1]);
	// printf("Echoing back 1st argument: %s\n", argv[1]);
	printf("%s\n", argv[1]);
	return 1;
}

uint8_t cmd_exit(char *argv[])
{
	return 0;
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

static uint8_t execute(char *argv[])
{
	if (argv[0] == NULL)
	{
		// An empty command was entered.
		printf("---> %s() END empty command\n", __func__);
		return 1;
	}

	// printf("1st char: %c %d\n", *argv[0], *argv[0]);

	// if (*argv[0] == '\n')
	if (*argv[0] == 0)
	{
		// ist just enter press, don't try to execute it
		return 1;
	}

	// remove_char(argv[0], '\n');

	// search for function
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

	return 1;
}


static char *read_line(void)
{
	// printf("input string: ");
	fgets(input, 100, stdin);
	// scanf("---> %s\n", input);
	// printf("input string: %s", input);
	remove_char(input, '\n');
	return input;
}


static void shell_loop(void)
{
	char *line;
	char **argv;	// will be something like: "cmd arg1 arg2 arg3 null"
	uint8_t status;

	do
	{
		printf("%% ");
		line = read_line();
		argv = split_line(line);
		// print_argv(argv);
		status = execute(argv);
	} while (status);
}

int main(int argc, const char *argv[])
{
	printf(SHELL_NAME " here, use help command\n");
	shell_loop();

	return 0;
}
