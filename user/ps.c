#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc > 1){
        fprintf(2, "Usage: ps\n");
        exit(1);
    }
    ps();
    exit(0);
}
