#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc > 1){
        fprintf(2, "Usage: pstree\n");
        exit(1);
    }
    pstree();
    exit(0);
}
