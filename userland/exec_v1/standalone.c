#include "cmd_v1.h"

int
main(int argc, kword_t **argv)
{
        struct u_v1_io io;
        io.in_fd = 0;
        io.out_fd = 1;
        io.err_fd = 2;
        return cmd_v1_dispatch(argc, argv, &io);
}
