#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(2, "usage: sleep [ticksn um]\n");
        exit(1);
    }

    int ticks = atoi(argv[1]);
    int ret = sleep(ticks);

    exit(ret);
}