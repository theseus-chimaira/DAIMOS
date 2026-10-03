#include "utility.h"

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct u_io io;
        int sink;
        int rc;

        io.in_fd = 0;
        io.out_fd = 1;
        io.err_fd = 2;
        sink = dsys_isatty(io.out_fd) < 0 && u_text_sink_attach(io.out_fd) == 0;
        rc = utility_dispatch(argc, argv, envp, &io);
        if (sink && u_text_sink_detach() != 0)
                rc = 1;
        return rc;
}
