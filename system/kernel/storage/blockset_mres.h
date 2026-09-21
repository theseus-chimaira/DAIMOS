#ifndef DAIMON_BLOCKSET_MRES_H
#define DAIMON_BLOCKSET_MRES_H

#include "kcore.h"

#define BLOCKSET_MRES_OP_BLOCKS       2U
#define BLOCKSET_MRES_OP_READ_BLOCK   3U
#define BLOCKSET_MRES_OP_WRITE_BLOCK  4U
#define BLOCKSET_MRES_OP_WRITABLE     5U
#define BLOCKSET_MRES_OP_TAIL_BLOCKS  6U
#define BLOCKSET_MRES_OP_TAIL_READ    7U
#define BLOCKSET_MRES_OP_TAIL_WRITE   8U
#define BLOCKSET_MRES_OP_MAP          9U

struct blockset_mres_request {
        kword_t op;
        kword_t a;
        kword_t b;
        kword_t c;
};

/* KINIT-only relocated runtime-state destinations exported by the MRES. */
extern unsigned int blockset_state_addr;
extern unsigned int blockset_read_addr;
extern unsigned int blockset_write_addr;

#endif
