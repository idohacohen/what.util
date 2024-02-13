#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXARGSLEN 512
#define MAXCMDS 10


/*
 * @brief free_all - free all the memory allocated for the lines
 *
 * @param plst - the list of lines
 * 
 * @param plst_len - the length of the list
*/
void free_all(char *plst[], int plst_len)
{
    for (int i = 0; i < plst_len; i++)
    {
        free(plst[i]);
    }
    free(plst);
}


/*
 * @brief parse_lines - parse the lines from the input to to separate commands
 *
 * @param lines - the list of lines
 * 
 * @return the number of lines
*/
int parse_lines(char *lines[])
{
    char buf[MAXARGSLEN];
    int line = 0;
    int i = 0;
    while (read(0, (void *)(buf + i), 1) != 0)
    {
        if (line > MAXCMDS)
        {
            fprintf(2, "too many extra args");
            free_all(lines, line);
            exit(1);
        }
        if (i > MAXARGSLEN)
        {
            fprintf(2, "arg line too long");
            free_all(lines, line);
            exit(1);
        }
        i++;
        if (buf[i - 1] == '\n')
        {
            buf[i - 1] = 0;
            lines[line] = malloc(sizeof(char) * (i - 1));
            strcpy(lines[line], buf);
            i = 0;
            line++;
        }
    }
    return line;
}

/*
 * @brief parse_args - parse the arguments from a string
 *
 * @param s - the input string
 * 
 * @param argv - the list of arguments passed to xargs
 * 
 * @param nargv - the list of new arguments to be passed to the command
*/
void parse_args(char *s, char *argv[], char *nargv[])
{
    int i = 0;
    while (argv[i] != 0)
    {
        nargv[i] = argv[i];
        i++;
    }
    if (*s == 0)
    {
        return;
    } 
    char *buf = s;
    nargv[i] = buf;
    i++;
    while ((buf = strchr(s, ' ')) != 0 && i < MAXARG)
    {
        *buf = 0;
        nargv[i] = buf + 1;
        i++;
    }
    nargv[i] = 0;
}

/*
 * @brief xargs - execute the command with the arguments and those from the input
 *
 * @param argv - the list of arguments passed to xargs
*/
void xargs(char *argv[])
{
    char *lines_buf[MAXCMDS];
    char **lines;
    int len = parse_lines(lines_buf);
    lines = malloc(sizeof(char*) * len);
    for (int i = 0; i < len; i++)
    {
        lines[i] = lines_buf[i];
    }
    char *nargv[MAXARG];
    for (int i = 0; i < len; i++)
    {
        if (fork() == 0)
        {
            for (int i = 0; i < MAXARG; i++)
            {
                nargv[i] = 0;
            }
            parse_args(lines[i], argv, nargv);
            exec(argv[0], nargv);
        }
        else
        {
            wait(0);
        }
    }
    free_all(lines, len);
    exit(0);
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "Usege: more-arguments | xargs command arguments\n");
        exit(1);
    }
    xargs(argv + 1);
    return 0;
}
