#include "diskset_mres.h"
#include "diskset.h"


int
diskset_mres_dispatch(struct diskset_mres_request *r)
{
        if (r == 0)
                return -1;
        switch ((unsigned int)r->op) {
        case DISKSET_MRES_OP_INIT:
                return diskset_boot_init(
                    (const struct diskset *)(unsigned long)r->a);
        case DISKSET_MRES_OP_BLOCKS:
                return (int)diskset_blocks();
        case DISKSET_MRES_OP_READ_BLOCK:
                return diskset_read_block(r->a,
                    (kword_t *)(unsigned long)r->b);
        case DISKSET_MRES_OP_WRITE_BLOCK:
                return diskset_write_block(r->a,
                    (const kword_t *)(unsigned long)r->b);
        case DISKSET_MRES_OP_WRITABLE:
                return diskset_writable();
        case DISKSET_MRES_OP_SWAP_BLOCKS:
                return (int)diskset_swap_blocks();
        case DISKSET_MRES_OP_SWAP_READ:
                return diskset_swap_read(r->a, r->b,
                    (kword_t *)(unsigned long)r->c);
        case DISKSET_MRES_OP_SWAP_WRITE:
                return diskset_swap_write(r->a, r->b,
                    (const kword_t *)(unsigned long)r->c);
        case DISKSET_MRES_OP_LOG_BLOCKS:
                return (int)diskset_log_blocks();
        case DISKSET_MRES_OP_LOG_READ:
                return diskset_log_read(r->a,
                    (kword_t *)(unsigned long)r->b);
        case DISKSET_MRES_OP_LOG_WRITE:
                return diskset_log_write(r->a,
                    (const kword_t *)(unsigned long)r->b);
        default:
                return -1;
        }
}
