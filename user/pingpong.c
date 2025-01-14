#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char** argv) {
    if(argc != 1) {
        fprintf(2, "usage: pingpong\n");
        exit(1);
    }
    
    int pid = 0;
    int pipe_1[2], pipe_2[2];
    char buf[] = {"nonsense"};

    pipe(pipe_1);
    pipe(pipe_2);

    int ret = fork();
    if(ret == 0) {
        pid = getpid();
        close(pipe_1[1]);
        close(pipe_2[0]);

        read(pipe_1[0], buf, strlen(buf));
        printf("%d: received ping\n", pid);
        write(pipe_2[1], buf, strlen(buf));
    }
    else {
        pid = getpid();
        close(pipe_1[0]);
        close(pipe_2[1]);

        write(pipe_1[1], buf, strlen(buf));
        read(pipe_2[0], buf, strlen(buf));
        printf("%d: received pong\n", pid);
    }

    exit(0);
}