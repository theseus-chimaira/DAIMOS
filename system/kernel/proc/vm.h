/**
 * @file vm.h
 * @brief Machine-independent process-VM boundary used by EXEC, MM, and proc.
 *
 * struct proc exposes only logical user-space size; the RH backend state is
 * deliberately opaque to generic code. The PDP-6 backend stores a contiguous
 * relocation base there. A noncontiguous backend MUST NOT assume that an
 * 18-bit physical base or arbitrary page-map pointer fits the same encoding.
 * Keep a backend-owned handle/descriptor if a future CPU actually needs one.
 *
 * Portability boundary v1605: a KI10 page map is not an APR-like physical
 * base. In particular, a caller cannot turn a user pointer into a single
 * 18-bit physical pointer by adding the process VM word's right half.
 * All user reads/writes, image loading, and snapshotting need target-owned
 * bounded transfer operations before a page-mapped backend can be enabled.
 * The PDP-6 backend and syscall hot paths remain unchanged for now.
 */
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
 * VM destruction publishes the all-ones sentinel.  It must not be mistaken
 * for an active address simply because its RH is nonzero.
 *
 * PDP-6 stores the contiguous physical relocation base in the private RH.
 * Other machines may require a separately owned mapping descriptor; they
 * must define their own backend state and presence/activation semantics.
 * Generic process, exec, scheduler, MonitorFS process view, and physical-MM
 * code must not interpret the private half. Some existing PDP-6 assembly
 * call sites still do so; they are portability conversion sites, NOT proof
 * that the current VM_SPACE_ACTIVE macro is universal. In particular the
 * present RH-nonzero active check applies only to the PDP-6 representation.
 *
 * Any future paged implementation must preserve logical address/permission
 * validation while keeping user-memory transfer independent of a single
 * physically contiguous executive pointer. No extra dispatch is introduced
 * on PDP-6 solely to anticipate that backend.
 * Existing VM_SPACE_ACTIVE and other direct callers still depend on the
 * current nonzero-RH residency encoding; they must be audited and converted
 * to backend predicates during KI10 work. Do NOT introduce runtime callbacks
 * or a new process descriptor word only for hypothetical portability.
 */
#define VM_SPACE_WORDS(p) \
        ((kword_t)(((p)->vm_state >> PROC_HALF_SHIFT) & PROC_HALF_MASK))
#define VM_SPACE_RESET(p) ((p)->vm_state = 0UL)
#define VM_SPACE_ACTIVE(p) \
        ((p)->vm_state != ~(kword_t)0 && \
        ((p)->vm_state & PROC_HALF_MASK) != 0UL)

/*
 * Callers validate process pointers and logical ranges before entering the
 * backend.  Keeping those policy checks generic avoids duplicate resident
 * code in every machine implementation.
 */
/** Allocate a new backend address space for one process. */
int vm_space_create(struct proc *p, unsigned int owner, kword_t words);
int vm_space_load_file(struct proc *p, vnode_t node, kword_t file_offset,
    kword_t user_offset, unsigned int words);
/** Destroy the current backend address space and detach swap backing. */
int vm_space_destroy(struct proc *p, unsigned int owner);
int vm_space_can_swap(const struct proc *p);
int vm_space_startup(struct proc *p, const kword_t *records,
    kword_t counts, kword_t *startup);
int vm_space_inspect_word(const struct proc *p, kword_t offset,
    kword_t *wordp);
/** Query/set the current process break; zero queries without changing it. */
kword_t vm_space_brk_current(kword_t requested);

/* Called by physical MM for an unpinned MM_TYPE_PROCESS extent. */
#define VM_EXTENT_ALIGN_WORDS 02000UL
/** Move one inactive physical process extent during MM compaction. */
int vm_extent_move(unsigned int owner, kword_t base, kword_t words,
    kword_t new_base);

/* Machine backend entries used directly by the PDP-10 assembly paths. */
void vm_enter_initial_user(struct proc *p, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);

#endif
