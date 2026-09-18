#include "exec.h"
#include "fs_mres.h"
#include "file.h"
#include "vm.h"
#include "proc_swap.h"
#include "syscall.h"

#define EXEC_HALF_MASK 0777777UL
#define EXEC_WORD_MASK 0777777777777UL
#define EXEC_DXR_MAGIC \
    ((VFS_SIX6('D','X','R',' ',' ',' ') >> 18) & EXEC_HALF_MASK)

int
exec_load_process(struct proc *p, unsigned int owner,
    const kword_t *path)
{
        vnode_t node;
        struct vfs_stat st;
        kword_t hdr[EXEC_DXR_EXT_HDR_WORDS];
        int process_words;
        int entry;
        int image_words;
        int text_words;
        int bss_words;
        int header_words;
        int dxr_flags;
        int reloc_words;

        /* DXR header fields are at most 18 bits.  Signed locals avoid KCC
         * sign-bit normalization around ordinary bounded comparisons. */
        if (p == 0 || path == 0 ||
            file_lookup_path(path, &node) != 0 ||
            vfs_stat(node, &st) != 0 || st.type != VFS_TYPE_REG ||
            file_check_access(node, 01U) != 0 ||
            st.size_words < EXEC_DXR_BASE_HDR_WORDS ||
            vfs_read_words(node, 0U, hdr, EXEC_DXR_BASE_HDR_WORDS) !=
            (int)EXEC_DXR_BASE_HDR_WORDS ||
            ((hdr[0] >> 18U) & EXEC_HALF_MASK) != EXEC_DXR_MAGIC)
                return -1;

        entry = (int)(hdr[0] & EXEC_HALF_MASK);
        image_words = (int)((hdr[1] >> 18U) & EXEC_HALF_MASK);
        bss_words = (int)(hdr[1] & EXEC_DXR_BSS_MASK);
        dxr_flags = (int)(hdr[1] &
            (EXEC_DXR_F_PURE | EXEC_DXR_F_IMPURE));
        if (image_words == 0 || image_words > (int)EXEC_DXR_MAX_IMAGE_WORDS ||
            bss_words > (int)EXEC_DXR_MAX_BSS_WORDS ||
            entry >= image_words ||
            dxr_flags == (EXEC_DXR_F_PURE | EXEC_DXR_F_IMPURE))
                return -1;
        reloc_words = (image_words + 35) / 36;
        process_words = (int)EXEC_DXR_BASE_HDR_WORDS + image_words +
            reloc_words;
        header_words = EXEC_DXR_BASE_HDR_WORDS;
        text_words = 0;
        if (st.size_words != (kword_t)process_words) {
                if (st.size_words != (kword_t)(process_words + 1) ||
                    vfs_read_words(node, 2U, &hdr[2], 1U) != 1 ||
                    (unsigned int)(hdr[2] & EXEC_HALF_MASK) !=
                    EXEC_DXR_TEXT_TAG)
                        return -1;
                text_words = (int)((hdr[2] >> 18U) & EXEC_HALF_MASK);
                if (text_words > image_words)
                        return -1;
                header_words = EXEC_DXR_EXT_HDR_WORDS;
        }
        process_words = (int)EXEC_USER_ORIGIN + image_words + bss_words +
            (int)EXEC_DXR_STACK_WORDS;
        if (process_words > (int)EXEC_HALF_MASK)
                return -1;
        if (vm_space_create(p, owner, (kword_t)process_words) != 0)
                return -1;
        if (vm_space_load_file(p, node, (kword_t)header_words,
            (kword_t)EXEC_USER_ORIGIN, (unsigned int)image_words) != 0)
                goto fail;

        p->meta = (p->meta &
            ((kword_t)PROC_PARENT_MASK << PROC_PARENT_SHIFT)) |
            (((kword_t)(entry + EXEC_USER_ORIGIN) & PROC_HALF_MASK) <<
            PROC_ENTRY_SHIFT);
        p->sched = PROC_SCHED_DEFAULT;
        PROC_SET_STATE(p, PROC_SIDL);
        if (proc_swap_attach(owner, node, (kword_t)text_words,
            (unsigned int)dxr_flags == EXEC_DXR_F_PURE &&
            header_words == EXEC_DXR_EXT_HDR_WORDS) != 0)
                goto fail;
        return 0;

fail:
        if (vm_space_destroy(p, owner) == 0)
                PROC_SET_META_LH(p, 0UL);
        return -1;
}


/* Return words occupied by one validated counted SIXBIT record. */
static unsigned int
exec_record_words(const kword_t *record, unsigned int max_chars, int nonempty)
{
        unsigned int chars;

        if (record == 0 || (record[0] & ~PROC_HALF_MASK) != 0UL)
                return 0U;
        chars = (unsigned int)record[0];
        if ((nonempty && chars == 0U) || chars > max_chars)
                return 0U;
        return 1U + (chars + 5U) / 6U;
}

/* Replace the current user image while preserving its process identity and
 * stable u-area.  EXEC V1 is an inline, bounded launch block using the same
 * counted SIXBIT startup-record format and child AC convention as RUN V2.
 * The replacement VM and its startup data are complete before the old VM is
 * touched, so every validation/load failure leaves the old image executable. */
int
exec_replace_current(const kword_t *block,
    unsigned int available_words, kword_t *entry_startup)
{
    const struct sys_exec_v1 *args =
        (const struct sys_exec_v1 *)block;
        struct proc staged;
        struct proc *current;
        const kword_t *path;
        const kword_t *records;
        const kword_t *end;
        const kword_t *scan;
        kword_t startup[4];
        kword_t old_swap;
        kword_t new_swap;
        kword_t counts;
        unsigned int slot;
        unsigned int words;
        unsigned int argc;
        unsigned int envc;
        unsigned int i;

        if (args == 0 || entry_startup == 0 || proc_table == 0 ||
            available_words < SYS_EXEC_V1_MIN_WORDS)
                return -1;
        if ((unsigned int)((args->version_words >> 18U) & PROC_HALF_MASK) !=
            SYS_EXEC_VERSION_1)
                return -1;
        words = (unsigned int)(args->version_words & PROC_HALF_MASK);
        if (words < SYS_EXEC_V1_MIN_WORDS || words > available_words)
                return -1;
        if ((args->argc & ~PROC_HALF_MASK) != 0UL ||
            (args->envc & ~PROC_HALF_MASK) != 0UL)
                return -1;
        argc = (unsigned int)args->argc;
        envc = (unsigned int)args->envc;
        if (argc > SYS_RUN_ARG_MAX || envc > SYS_RUN_ENV_MAX)
                return -1;

        end = (const kword_t *)args + words;
        path = &args->path[0];
        if (path >= end)
                return -1;
        i = exec_record_words(path, SYS_RUN_PATH_MAX_CHARS, 1);
        if (i == 0U || path + i > end)
                return -1;
        records = path + i;
        scan = records;
        for (i = 0U; i < argc + envc; ++i) {
                unsigned int record_words;

                if (scan >= end)
                        return -1;
                record_words = exec_record_words(scan,
                    SYS_RUN_ARG_MAX_CHARS, 0);
                if (record_words == 0U || scan + record_words > end)
                        return -1;
                scan += record_words;
        }
        if (scan != end)
                return -1;

        slot = (unsigned int)proc_current_slot;
        if (slot == 0U || slot >= proc_slots)
                return -1;
        current = &proc_table[slot];
        if (!PROC_HAS_UAREA(current) || !VM_SPACE_ACTIVE(current) ||
            proc_swap_records == 0)
                return -1;

        old_swap = proc_swap_records[slot].state;
        staged.meta = current->meta;
        if (exec_load_process(&staged, slot, path) != 0) {
                proc_swap_records[slot].state = old_swap;
                return -1;
        }
        counts = ((kword_t)argc << 18U) | (kword_t)envc;
        if (vm_space_startup(&staged, records, counts, startup) != 0) {
                (void)vm_space_destroy(&staged, slot);
                proc_swap_records[slot].state = old_swap;
                return -1;
        }
        new_swap = proc_swap_records[slot].state;
        proc_swap_records[slot].state = old_swap;

        /* The mapped launch block is no longer referenced.  Clear the hold
         * before freeing its containing VM; the failure path restores it. */
        PROC_CTL_WORD(current) &= ~PROC_USER_MAP_BIT;
        if (vm_space_destroy(current, slot) != 0) {
                PROC_CTL_WORD(current) |= PROC_USER_MAP_BIT;
                (void)vm_space_destroy(&staged, slot);
                proc_swap_records[slot].state = old_swap;
                return -1;
        }

        current->vm_state = staged.vm_state;
        proc_swap_records[slot].state = new_swap;
        PROC_SWAP_BACKING_WORD(current) = 0UL;
        entry_startup[0] = PROC_ENTRY(&staged);
        entry_startup[1] = startup[3];
        entry_startup[2] = startup[0];
        entry_startup[3] = startup[1];
        entry_startup[4] = startup[2];
        return 0;
}
