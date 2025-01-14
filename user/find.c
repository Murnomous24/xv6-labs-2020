#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char buf[1024];
int match(char*, char*);

void
grep(char *pattern, int fd)
{
  int n, m;
  char *p, *q;

  m = 0;
  while((n = read(fd, buf+m, sizeof(buf)-m-1)) > 0){
    m += n;
    buf[m] = '\0';
    p = buf;
    while((q = strchr(p, '\n')) != 0){
      *q = 0;
      if(match(pattern, p)){
        *q = '\n';
        write(1, p, q+1 - p);
      }
      p = q+1;
    }
    if(m > 0){
      m -= p - buf;
      memmove(buf, p, m);
    }
  }
}

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

/*
^: search from start of string
&: search from end of string
.: match any character(not null)
*: match a character(once or more)
*/
int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text); //match from start
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0'); //or search one by one
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0') //regex is null
    return 1;
  if(re[1] == '*') //regex is '*'
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0') //regex is '...$\0' 
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text)) //regex is '.' and *text is not null or regex equals *text
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char *re) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }
    if((fstat(fd, &st) < 0)) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }

    switch(st.type) {
        case T_FILE:
            if(match(re, fmtname(path))) {
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                fprintf(2, "find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                if(de.inum == 0) continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0) {
                    fprintf(2, "find: cannot stat %s\n", buf);
                    continue;
                }

                if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                    continue;
                }
                //printf("find's full name: %s\n", buf);
                find(buf, re);
            }
    }
    close(fd);
}

int main(int argc, char *argv[])
{
  if(argc <= 1){
    fprintf(2, "usage: find path pattern\n");
    exit(1);
  }
  
  char *path = argv[1];
  char *re = argv[2];
  find(path, re);
  exit(0);
}
