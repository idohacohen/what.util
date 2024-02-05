#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

/*
 * @brief find the name of a file from a path
 *
 * @param path the path to the file
 *
 * @return the name of the file
*/
char*
fmtname(char *path)
{
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

/*
 * @brief find the file with the given name in the given path
 *
 * @param path the path to the file
 * @param filename the name of the file
 *
 * @return void
*/
void
find(char *path, char *filename)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type)
  {
  case T_DEVICE:
  case T_FILE:
    if (strcmp(fmtname(path), filename) == 0)
    {
        printf("%s\n", path);
    }
    break;

  case T_DIR:
    while(read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (strcmp(de.name, ".") == 0 ||
            strcmp(de.name, "..") == 0 || 
            de.inum == 0)
        {
            continue;
        }
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
        {
            fprintf(2, "find: path to %s too long", de.name);
            continue;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;        
        find(buf, filename);
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(2, "Usage: find dir filename\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
