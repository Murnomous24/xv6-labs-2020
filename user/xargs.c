#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

/*
xargs : read content from stdin, and build & execute command
- 1. [command] | xargs [...]
- 2. child process excute the left command
- 3. child transfer the output to stdout
- 4. father process read content from stdin as arguments
- 5. father call function with arguments from step 4
*/

int main(int argc, char** argv) {
    char buf1[512] = {};
    char buf2[MAXARG][32] = {};
    char *exec_buf[MAXARG] = {};

    for(int i = 0; i < MAXARG; i ++) {
        exec_buf[i] = buf2[i];
    }

    for(int i = 1; i < argc; i ++) {
        strcpy(buf2[i - 1], argv[i]);
    }

    int n_read = 0;
    while((n_read = read(0, buf1, sizeof(buf1))) > 0) {
        int pos = argc - 1;
        char *c = buf2[pos];

        for(char *p = buf1; *p; p ++) {
            if(*p == ' ' || *p == '\n') {
                *c = '\0';
                pos ++;
                c = buf2[pos];
            }
            else {
                *c ++ = *p;
            }
        }
        *c = '\0';
        pos ++;
        exec_buf[pos] = 0;

        int ret = fork();
        if(ret == 0) {
             exec(exec_buf[0], exec_buf);
        }
        else {
            wait(0);
        }
    }

    exit(0);
}