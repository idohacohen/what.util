#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
print_primes(int (*inp)[])
{
    int buff;
    int prime;
    close((*inp)[1]);
    read((*inp)[0], &prime, 4);
    printf("prime %d\n", prime);
    if (read((*inp)[0], &buff, 4) == 0)
    {
        close((*inp)[0]);
        close((*inp)[1]);
        exit(0);
    }
    int outp[2];
    pipe(outp);
    if (fork() == 0)
    {
        print_primes(&outp);
    }
    else
    {
    do
    {
        if (buff % prime != 0)
        {
            write(outp[1], &buff, 4);
        }
    } while (read((*inp)[0], &buff, 4) != 0);
    close(outp[1]);
    close((*inp)[0]);
    }
    wait((int *) 0);
    exit(0);
}

int
main(int argc, char const *argv[])
{
    int initp[2];
    pipe(initp);
    for (int i = 2; i < 36; i++)
    {
        write(initp[1], &i, 4);
    }
    close(initp[1]);
    print_primes(&initp);
    exit(0);
}
