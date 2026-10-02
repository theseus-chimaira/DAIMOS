#ifndef DAIMOS_USER_UTILITY_H
#define DAIMOS_USER_UTILITY_H

#include "u.h"

int utility_dispatch(int argc, kword_t **argv, kword_t **envp,
    struct u_io *io);

#endif
