/* Sparse packed-process descriptor table, capacity selected by caller. */
#ifndef DAIMOS_PROC_PACKED_SPARSE_V34_H
#define DAIMOS_PROC_PACKED_SPARSE_V34_H
#include "proc_packed_lifetime_v33.h"

/* Three words per occupied entry: slot ID, encoded span, writable layout.
 * Zero slot means free. Entry addresses remain stable until explicit release.
 */
struct proc_packed_sparse_entry_v34 {
        kword_t slot;
        struct proc_packed_v1 descriptor;
};
struct proc_packed_sparse_v34 {
        struct proc_packed_sparse_entry_v34 *entries;
        unsigned int capacity;
};
int proc_packed_sparse_init_v34(struct proc_packed_sparse_v34 *table,
    struct proc_packed_sparse_entry_v34 *entries, unsigned int capacity);
struct proc_packed_v1 *proc_packed_sparse_get_v34(
    const struct proc_packed_sparse_v34 *table, unsigned int slot);
int proc_packed_sparse_publish_v34(struct proc_packed_sparse_v34 *table,
    unsigned int slot, const struct proc_packed_v1 *prepared);
int proc_packed_sparse_discard_v34(struct proc_packed_sparse_v34 *table,
    unsigned int slot);
#endif
