#include "utility.h"

#ifndef DAIMOS_UTILITY_PROGRAM
#define DAIMOS_UTILITY_PROGRAM 0
#endif

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct u_io io;
#if DAIMOS_UTILITY_PROGRAM != UTILITY_PROGRAM_TRUE && \
    DAIMOS_UTILITY_PROGRAM != UTILITY_PROGRAM_FALSE
        int sink;
#endif
        int rc;

        io.in_fd = 0;
        io.out_fd = 1;
        io.err_fd = 2;
#if DAIMOS_UTILITY_PROGRAM != UTILITY_PROGRAM_TRUE && \
    DAIMOS_UTILITY_PROGRAM != UTILITY_PROGRAM_FALSE
        sink = dsys_isatty(io.out_fd) < 0 && u_text_sink_attach(io.out_fd) == 0;
#endif
        rc = UTILITY_PROGRAM_ENTRY(DAIMOS_UTILITY_TOKEN)(argc, argv, envp,
            &io);
#if DAIMOS_UTILITY_PROGRAM != UTILITY_PROGRAM_TRUE && \
    DAIMOS_UTILITY_PROGRAM != UTILITY_PROGRAM_FALSE
        if (sink && u_text_sink_detach() != 0)
                rc = 1;
#endif
        return rc;
}
