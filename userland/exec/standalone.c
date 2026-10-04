#include "cmd.h"

int
main(int argc, kword_t **argv)
{
        struct u_io io;
        int sink;
        int rc;

        io.in_fd = 0;
        io.out_fd = 1;
        io.err_fd = 2;
        /* Native text files and pipes carry S6REC records.  Keep terminal
         * output character-oriented, but adapt redirected CMD output through
         * the same bounded text sink used by the other standalone utilities. */
        sink = dsys_isatty(io.out_fd) < 0 &&
            u_text_sink_attach(io.out_fd) == 0;
        rc = CMD_PROGRAM_ENTRY(DAIMOS_CMD_TOKEN)(argc, argv, &io);
        if (sink && u_text_sink_detach() != 0)
                rc = 1;
        return rc;
}
