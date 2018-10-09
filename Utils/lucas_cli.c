#include <sys/types.h>
#include "lucas_cli.h"
#include "gateway.h"
#include "stdio.h"


#define wifi_station_log(M, ...) custom_log("wifi", M, ##__VA_ARGS__)

#ifdef CLI_ENABLE

#define RET_CHAR '\n'
#define END_CHAR '\r'
#define PROMPT "\r\n# "
#define EXIT_MSG "exit"
#define NUM_BUFFERS 1
#define MAX_COMMANDS 50
#define INBUF_SIZE 80
#define OUTBUF_SIZE 1024

static void task_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);

struct cli_st
{
    int initialized;

    unsigned int bp; /* buffer pointer */
    char inbuf[INBUF_SIZE];
    char outbuf[OUTBUF_SIZE];
    const struct cli_command *commands[MAX_COMMANDS];
    unsigned int num_commands;
    int echo_disabled;
};

static struct cli_st *pCli = NULL;
static uint8_t *cli_rx_data;

static int cli_putstr(const char *msg);

/* Find the command 'name' in the cli commands table.
* If len is 0 then full match will be performed else upto len bytes.
* Returns: a pointer to the corresponding cli_command struct or NULL.
*/
static const struct cli_command *lookup_command(char *name, int len)
{
    int i = 0;
    int n = 0;

    while (i < MAX_COMMANDS && n < pCli->num_commands)
    {
        if (pCli->commands[i]->name == NULL)
        {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0)
        {
            if (!strncmp(pCli->commands[i]->name, name, len))
                return pCli->commands[i];
        }
        else
        {
            if (!strcmp(pCli->commands[i]->name, name))
                return pCli->commands[i];
        }

        i++;
        n++;
    }

    return NULL;
}

/* Parse input line and locate arguments (if any), keeping count of the number
* of arguments and their locations.  Look up and call the corresponding cli
* function if one is found and pass it the argv array.
*
* Returns: 0 on success: the input line contained at least a function name and
*          that function exists and was called.
*          1 on lookup failure: there is no corresponding function for the
*          input line.
*          2 on invalid syntax: the arguments list couldn't be parsed
*/
static int handle_input(char *inbuf)
{
    struct
    {
        unsigned inArg : 1;
        unsigned inQuote : 1;
        unsigned done : 1;
    } stat;
    static char *argv[16];
    int argc = 0;
    int i = 0;
    const struct cli_command *command = NULL;
    const char *p;

    memset((void *)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));

    do
    {
        switch (inbuf[i])
        {
        case '\0':
            if (stat.inQuote)
                return 2;
            stat.done = 1;
            break;

        case '"':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
            {
                memcpy(&inbuf[i - 1], &inbuf[i],
                       strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
                break;
            if (stat.inQuote && !stat.inArg)
                return 2;

            if (!stat.inQuote && !stat.inArg)
            {
                stat.inArg = 1;
                stat.inQuote = 1;
                argc++;
                argv[argc - 1] = &inbuf[i + 1];
            }
            else if (stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                stat.inQuote = 0;
                inbuf[i] = '\0';
            }
            break;

        case ' ':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
            {
                memcpy(&inbuf[i - 1], &inbuf[i],
                       strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                inbuf[i] = '\0';
            }
            break;

        default:
            if (!stat.inArg)
            {
                stat.inArg = 1;
                argc++;
                argv[argc - 1] = &inbuf[i];
            }
            break;
        }
    } while (!stat.done && ++i < INBUF_SIZE);

    if (stat.inQuote)
        return 2;

    if (argc < 1)
        return 0;

    if (!pCli->echo_disabled)
        cli_printf("\r\n");

    /*
  * Some comamands can allow extensions like foo.a, foo.b and hence
  * compare commands before first dot.
  */
    i = ((p = strchr(argv[0], '.')) == NULL) ? 0 : (p - argv[0]);
    command = lookup_command(argv[0], i);
    if (command == NULL)
        return 1;

    memset(pCli->outbuf, 0, OUTBUF_SIZE);
    cli_putstr("\r\n");
    command->function(pCli->outbuf, OUTBUF_SIZE, argc, argv);
    cli_putstr(pCli->outbuf);
    return 0;
}

/* Perform basic tab-completion on the input buffer by string-matching the
* current input line against the cli functions table.  The current input line
* is assumed to be NULL-terminated. */
static void tab_complete(char *inbuf, unsigned int *bp)
{
    int i, n, m;
    const char *fm = NULL;

    cli_printf("\r\n");

    /* show matching commands */
    for (i = 0, n = 0, m = 0; i < MAX_COMMANDS && n < pCli->num_commands;
         i++)
    {
        if (pCli->commands[i]->name != NULL)
        {
            if (!strncmp(inbuf, pCli->commands[i]->name, *bp))
            {
                m++;
                if (m == 1)
                    fm = pCli->commands[i]->name;
                else if (m == 2)
                    cli_printf("%s %s ", fm,
                               pCli->commands[i]->name);
                else
                    cli_printf("%s ",
                               pCli->commands[i]->name);
            }
            n++;
        }
    }

    /* there's only one match, so complete the line */
    if (m == 1 && fm)
    {
        n = strlen(fm) - *bp;
        if (*bp + n < INBUF_SIZE)
        {
            memcpy(inbuf + *bp, fm + *bp, n);
            *bp += n;
            inbuf[(*bp)++] = ' ';
            inbuf[*bp] = '\0';
        }
    }

    /* just redraw input line */
    cli_printf("%s%s", PROMPT, inbuf);
}

/* Get an input line.
*
* Returns: 1 if there is input, 0 if the line should be ignored. */
static int get_input(char *inbuf, unsigned int *bp)
{

    if (inbuf == NULL)
    {
        return 0;
    }
    while (cli_getchar(&inbuf[*bp]) == 1)
    {

        if (inbuf[*bp] == RET_CHAR)
            continue;
        if (inbuf[*bp] == END_CHAR)
        { /* end of input line */
               
            inbuf[*bp] = '\0';
            *bp = 0;
            return 1;
        }

        if ((inbuf[*bp] == 0x08) || /* backspace */
            (inbuf[*bp] == 0x7f))
        { /* DEL */
            if (*bp > 0)
            {
                (*bp)--;
                if (!pCli->echo_disabled)
                    cli_printf("%c %c", 0x08, 0x08);
            }
            continue;
        }

        if (inbuf[*bp] == '\t')
        {
            inbuf[*bp] = '\0';
            tab_complete(inbuf, bp);
            continue;
        }

        if (!pCli->echo_disabled)
            cli_printf("%c", inbuf[*bp]);

        (*bp)++;
        if (*bp >= INBUF_SIZE)
        {
            cli_printf("Error: input buffer overflow\r\n");
            cli_printf(PROMPT);
            *bp = 0;
            return 0;
        }
    }

    return 0;
}


/* Print out a bad command string, including a hex
* representation of non-printable characters.
* Non-printable characters show as "\0xXX".
*/
static void print_bad_command(char *cmd_string)
{
    if (cmd_string != NULL)
    {
        char *c = cmd_string;
        cli_printf("command '");
        while (*c != '\0')
        {
            if (isprint(*c))
            {
                cli_printf("%c", *c);
            }
            else
            {
                cli_printf("\\0x%x", *c);
            }
            ++c;
        }
        cli_printf("' not found\r\n");
    }
}

/* Main CLI processing thread
*
* Waits to receive a command buffer pointer from an input collector, and
* then processes.  Note that it must cleanup the buffer when done with it.
*
* Input collectors handle their own lexical analysis and must pass complete
* command lines to CLI.
*/
static void cli_main(uint32_t data)
{
    while (1)
    {
        int ret;
        char *msg = NULL;

        fgets(pCli->inbuf, INBUF_SIZE, stdin);
        pCli->inbuf[strlen(pCli->inbuf)-1] = '\0';

        if (!pCli->echo_disabled)
            cli_printf("%s", pCli->inbuf);

        msg = pCli->inbuf;

        if (msg != NULL)
        {
            if (strcmp(msg, EXIT_MSG) == 0)
                break;
            ret = handle_input(msg);
            if (ret == 1)
                print_bad_command(msg);
            else if (ret == 2)
                cli_printf("syntax error\r\n");
            cli_printf(PROMPT);
        }
    }

    cli_printf("CLI exited\r\n");
    free(pCli);
    pCli = NULL;
    pthread_exit(NULL);
}

static void task_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void tftp_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void partShow_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void uptime_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    //cmd_printf("UP time %ldms\r\n", mico_rtos_get_time());
}

extern void tftp_ota(void);
void tftp_ota_thread(int arg)
{
}

static void ota_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void help_command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);

/*
*  Command buffer API
*/

static void get_version(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void reboot_lucas(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void echo_cmd_handler(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if (argc == 1) {
        cmd_printf("Usage: echo on/off. Echo is currently %s\r\n",
        pCli->echo_disabled ? "Disabled" : "Enabled");
        return;
    }

    if (!strcasecmp(argv[1], "on")) {
        cmd_printf("Enable echo\r\n");
        pCli->echo_disabled = 0;
    } else if (!strcasecmp(argv[1], "off")) {
        cmd_printf("Disable echo\r\n");
        pCli->echo_disabled = 1;
    }
}

static void cli_exit_handler(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    pthread_exit(NULL);
}

static const struct cli_command built_ins[] = {
    {"help", NULL, help_command},
    {"version", NULL, get_version},
    {"echo", NULL, echo_cmd_handler},
    {"exit", "CLI exit", cli_exit_handler}};

/* Built-in "help" command: prints all registered commands and their help
* text string, if any. */
static void help_command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int i, n;
    uint32_t build_in_count = sizeof(built_ins) / sizeof(struct cli_command);

    build_in_count++;

    cmd_printf("====Build-in Commands====\r\n");
    for (i = 0, n = 0; i < MAX_COMMANDS && n < pCli->num_commands; i++)
    {
        if (pCli->commands[i]->name)
        {
            cmd_printf("%s: %s\r\n", pCli->commands[i]->name,
                       pCli->commands[i]->help ? pCli->commands[i]->help : "");
            n++;
            if (n == build_in_count)
            {
                cmd_printf("\r\n====User Commands====\r\n");
            }
        }
    }
}

int cli_register_command(const struct cli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    if (pCli->num_commands < MAX_COMMANDS)
    {
        /* Check if the command has already been registered.
    * Return 0, if it has been registered.
    */
        for (i = 0; i < pCli->num_commands; i++)
        {
            if (pCli->commands[i] == command)
                return 0;
        }
        pCli->commands[pCli->num_commands++] = command;
        return 0;
    }

    return 1;
}

int cli_unregister_command(const struct cli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    for (i = 0; i < pCli->num_commands; i++)
    {
        if (pCli->commands[i] == command)
        {
            pCli->num_commands--;
            int remaining_cmds = pCli->num_commands - i;
            if (remaining_cmds > 0)
            {
                memmove(&pCli->commands[i], &pCli->commands[i + 1],
                        (remaining_cmds *
                         sizeof(struct cli_command *)));
            }
            pCli->commands[pCli->num_commands] = NULL;
            return 0;
        }
    }

    return 1;
}

int cli_register_commands(const struct cli_command *commands, int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (cli_register_command(commands++))
            return 1;
    return 0;
}

int cli_unregister_commands(const struct cli_command *commands,
                            int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (cli_unregister_command(commands++))
            return 1;

    return 0;
}

int cli_init(void)
{
    int err;
    pthread_t thread;

    pCli = (struct cli_st *)malloc(sizeof(struct cli_st));
    if (pCli == NULL)
        return -1;

    cli_rx_data = (uint8_t *)malloc(INBUF_SIZE);
    if (cli_rx_data == NULL)
    {
        free(pCli);
        pCli = NULL;
        return -1;
    }
    memset((void *)pCli, 0, sizeof(struct cli_st));

    /* add our built-in commands */
    if (cli_register_commands(&built_ins[0], sizeof(built_ins) / sizeof(struct cli_command)))
    {
        free(pCli);
        pCli = NULL;
        return -1;
    }

    err = pthread_create(&thread, NULL, cli_main, NULL);
    if (0 != err)
    {
        cli_printf("Error: Failed to create cli thread: %d\r\n", err);
        free(pCli);
        pCli = NULL;
        return -1;
    }
    else
        cli_printf("cli_main  thread create  success!\n");

    pCli->initialized = 1;

    return 0;
}

/* ========= CLI input&output APIs ============ */

int cli_printf(const char *msg, ...)
{
    va_list ap;
    char *pos, message[256];
    int sz;
    int nMessageLen = 0;

    memset(message, 0, 256);
    pos = message;

    sz = 0;
    va_start(ap, msg);
    nMessageLen = vsnprintf(pos, 256 - sz, msg, ap);
    va_end(ap);

    if (nMessageLen <= 0)
        return 0;

    cli_putstr((const char *)message);
    return 0;
}

int cli_putstr(const char *msg)
{
    if (msg[0] != 0)
        for (int i = 0; i < strlen(msg); i++)
            putchar(msg[i]);

    return 0;
}

int cli_getchar(char *inbuf)
{
    *inbuf = getchar();
    return 1;
}
#endif