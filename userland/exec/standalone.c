#include "cmd.h"

int
main(int argc, kword_t **argv)
{
        struct u_io io;
        io.in_fd = 0;
        io.out_fd = 1;
        io.err_fd = 2;
        return cmd_dispatch(argc, argv, &io);
}
