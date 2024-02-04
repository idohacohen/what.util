#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
main(int argc, char const* argv[])
{
    char buff[1];
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid > 0)
    {
        write(p[1], "b", 1);
        close(p[1]);
        wait((int *) 0);
        read(p[0], buff, 1);
        close(p[0]);
        if (buff[0] == 'b')
        {
            printf("%d: received pong\n", getpid());
            exit(0);
        }
        else
        {
            fprintf(2, "pong error");
            exit(1);
        }   
    }
    else if (pid == 0)
    {
        read(p[0], buff, 1);
        close(p[0]);
        if (buff[0] == 'b')
        {
            printf("%d: received ping\n", getpid());
            write(p[1], "b", 1);
            close(p[1]);
            exit(0);
        }
        else
        {
            fprintf(2, "ping error");
            exit(1);
        }
    }
    else
    {
        fprintf(2, "fork error");
        exit(1);
    }
}