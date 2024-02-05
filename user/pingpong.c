#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
 * @brief send a message to the pong process then wait for a response
 *
 * @param p the pipe
 *
 * @return void
 */
void
ping(int *p)
{
    char buff[1];
    write(p[1], "b", 1);
    close(p[1]);
    wait((int *) 0);
    read(p[0], buff, 1);
    close(p[0]);
    if (buff[0] == 'b')
    {
        printf("%d: received pong\n", getpid());
    }
    else
    {
        fprintf(2, "pong error");
        exit(1);
    }   
}

/*
 * @brief receive a message from the ping process and send a response
 *
 * @param p the pipe
 *
 * @return void
 */
void
pong(int *p)
{
    char buff[1];
    read(p[0], buff, 1);
    close(p[0]);
    if (buff[0] == 'b')
    {
        printf("%d: received ping\n", getpid());
        write(p[1], "b", 1);
        close(p[1]);
    }
    else
    {
        fprintf(2, "ping error");
        exit(1);
    }
}

int 
main(int argc, char const* argv[])
{
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid > 0)
    {
        ping(p);
        exit(0);
    }
    else if (pid == 0)
    {
        pong(p);
        exit(0);
    }
    else
    {
        fprintf(2, "fork error");
        exit(1);
    }
}