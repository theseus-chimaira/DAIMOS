#ifndef DAIMON_DISKSET_MRES_H
#define DAIMON_DISKSET_MRES_H

#include "kcore.h"

#define DISKSET_MRES_OP_BLOCKS          2U
#define DISKSET_MRES_OP_READ_BLOCK      3U
#define DISKSET_MRES_OP_WRITE_BLOCK     4U
#define DISKSET_MRES_OP_WRITABLE        5U
#define DISKSET_MRES_OP_SWAP_BLOCKS     6U
#define DISKSET_MRES_OP_SWAP_READ       7U
#define DISKSET_MRES_OP_SWAP_WRITE      8U
#define DISKSET_MRES_OP_LOG_BLOCKS      9U
#define DISKSET_MRES_OP_LOG_READ       10U
#define DISKSET_MRES_OP_LOG_WRITE      11U

struct diskset_mres_request {
        kword_t op;
        kword_t a;
        kword_t b;
        kword_t c;
};

/* Fixed KCORE service pointer, bound once after MINIT discovery. */
extern unsigned int diskset_service_addr;

/* KINIT-only relocated runtime-state destinations exported by the MRES. */
extern unsigned int diskset_state_addr;
extern unsigned int diskset_total_addr;

#endif
