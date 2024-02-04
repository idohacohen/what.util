#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
    char buff[1];
    int p[2];
    pipe(p);
    if (fork() == 0)
    {
        printf("asd\n");
        write(p[1], "h", 1);
        close(p[1]);
        exit(0);
    }
    else
    {
        close(p[1]);
        int x = read(p[0], buff, 1);
        int y = read(p[0], buff, 1);
        close(p[0]);
        printf("test %d\n %d", x, y);        
        exit(0);
    }
}
