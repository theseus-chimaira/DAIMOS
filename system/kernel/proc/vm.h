#ifndef DAIMON_VM_H
#define DAIMON_VM_H

#include "proc.h"
#include "vfs.h"

/*
 * Process VM boundary.
 *
 * struct proc carries one compact backend state word.  Its LH is the logical
 * user-space size; its RH is owned entirely by the selected machine backend.
 * A zero word means that no user address space has been established; a zero
 * RH means there is no currently activatable/resident machine mapping.
 *
 * PDP-6 stores the contiguous physical relocation base in the private RH.
 * Paged machines may instead store a page-map/address-space handle there.
 * Generic process, exec, scheduler, MonitorFS process view, and physical-MM code must not
 * interpret the private half.
 */
#define VM_SPACE_WORDS(p) \
        ((kword_t)(((p)->vm_state >> PROC_HALF_SHIFT) & PROC_HALF_MASK))
#define VM_SPACE_RESET(p) ((p)->vm_state = 0UL)
#define VM_SPACE_ACTIVE(p) \
        (((p)->vm_state & PROC_HALF_MASK) != 0UL)

/*
 * Callers validate process pointers and logical ranges before entering the
 * backend.  Keeping those policy checks generic avoids duplicate resident
 * code in every machine implementation.
 */
int vm_space_create(struct proc *p, unsigned int owner, kword_t words);
int vm_space_load_file(struct proc *p, vnode_t node, kword_t file_offset,
    kword_t user_offset, unsigned int words);
int vm_space_destroy(struct proc *p, unsigned int owner);
int vm_space_can_swap(const struct proc *p);
int vm_space_startup(struct proc *p, const kword_t *records,
    kword_t counts, kword_t *startup);

/* Called by physical MM for an unpinned MM_TYPE_PROCESS extent. */
#define VM_EXTENT_ALIGN_WORDS 02000UL
int vm_extent_move(unsigned int owner, kword_t base, kword_t words,
    kword_t new_base);

/* Machine backend entries used directly by the PDP-10 assembly paths. */
void vm_enter_initial_user(struct proc *p, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);

#endif
