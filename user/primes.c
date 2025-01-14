#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
/*
use pipe and fork to implement primes sieve
1. first fork, father process output 2~35
2. child process take the first number of pipe, this number is prime (e.g. 2)
3. second fork, child process call the function recursively
4. father process choose numbers that cannot divided by "the first number" (e.g. 4,6,8,10,..,2n can be divided by 2, so delete them)
4. NOTE: every father process should wait child, making xv6 limitations for process number works
*/
void sieve(int pipe_left[]) {
    int prime_now = 0;
    int read_ret = read(pipe_left[0], &prime_now, sizeof(prime_now));
    if(read_ret == 0) { 
        /*
        note: asssume the number that remains are "32 33 34 35", these are not primes, 
                so the father process don't write things in pipe, and child process just 
                read null from pipe, the 'read_ret' will be zero(0), so we should stop now!
        */
        close(pipe_left[0]);
        exit(0);
    }
    printf("prime %d\n", prime_now);

    int pipe_right[2] = {};
    pipe(pipe_right);

    int ret = fork();
    if(ret == 0) {
        close(pipe_right[1]);
        sieve(pipe_right);
        close(pipe_right[0]);
    }
    else {
        close(pipe_right[0]);
        int num = 0;

        while(read(pipe_left[0], &num, sizeof(num)) != 0) {
            if(num % prime_now != 0) {
                write(pipe_right[1], &num, sizeof(num));
            }
        }
        close(pipe_right[1]);
        wait(0);
    }
}

int main(int argc, char** argv) {
    if(argc != 1) {
        fprintf(2, "usage: pingpong\n");
        exit(1);
    }
    
    int pipe_1[2] = {};
    pipe(pipe_1);

    int ret = fork();
    if(ret == 0) {
        close(pipe_1[1]);
        sieve(pipe_1);
        close(pipe_1[0]);
    }
    else {
        close(pipe_1[0]);

        for(int i = 2; i < 36; i ++) {
            write(pipe_1[1], &i, sizeof(i));
        }
        close(pipe_1[1]);
        wait(0); //is that legal?
    }
    
    exit(0);
}