#include "monitorfs.h"
#include "kfmt.h"
#include "proc.h"
#include "vm.h"

static int
mfsproc_slot_active(unsigned int slot)
{
        return proc_table != 0 && slot < proc_slots &&
            !PROC_IS_FREE(&proc_table[slot]);
}

static inline void
mfsproc_name_set(struct vfs_name *name, kword_t word, unsigned int chars)
{
        name->chars = chars;
        name->words[0] = word;
        name->words[1] = 0UL;
        name->words[2] = 0UL;
        name->words[3] = 0UL;
}

static inline void
mfsproc_dirent_set(struct vfs_dirent *ent, kword_t word,
    unsigned int chars, unsigned int type)
{
        mfsproc_name_set(&ent->name, word, chars);
        ent->type = type;
}

static int
mfsproc_parse_slot(const struct vfs_name *name, unsigned int *slotp)
{
        unsigned int i;
        unsigned int value;
        unsigned int ch;

        if (name == 0 || slotp == 0 || name->chars == 0U ||
            name->chars > 3U)
                return -1;
        value = 0U;
        for (i = 0U; i < name->chars; ++i) {
                ch = vfs_name_char(name, i);
                if (ch < (unsigned int)VFS_SIXCHAR('0') ||
                    ch > (unsigned int)VFS_SIXCHAR('9'))
                        return -1;
                value = value * 10U + ch - (unsigned int)VFS_SIXCHAR('0');
        }
        if (value >= PROC_MAX_SLOTS)
                return -1;
        *slotp = value;
        return 0;
}

static void
mfsproc_format_slot(unsigned int slot, struct vfs_name *name)
{
        unsigned int hundreds;
        unsigned int tens;
        unsigned int ones;
        kword_t word;
        unsigned int chars;

        hundreds = slot / 100U;
        tens = (slot / 10U) % 10U;
        ones = slot % 10U;
        word = 0UL;
        chars = 0U;
        if (hundreds != 0U) {
                word |= VFS_SIXCHAR('0' + hundreds) << 30U;
                word |= VFS_SIXCHAR('0' + tens) << 24U;
                word |= VFS_SIXCHAR('0' + ones) << 18U;
                chars = 3U;
        } else if (tens != 0U) {
                word |= VFS_SIXCHAR('0' + tens) << 30U;
                word |= VFS_SIXCHAR('0' + ones) << 24U;
                chars = 2U;
        } else {
                word |= VFS_SIXCHAR('0' + ones) << 30U;
                chars = 1U;
        }
        mfsproc_name_set(name, word, chars);
}

#define MONITORFS_PROCESS_FILE_COUNT 5U
#define MONITORFS_PROCESS_FILE_LEN_BITS 3U
#define MONITORFS_PROCESS_FILE_LEN_MASK 07UL
#define MONITORFS_PROCESS_FILE_LENGTHS \
        ((kword_t)4U | ((kword_t)5U << 3) | ((kword_t)5U << 6) | \
        ((kword_t)4U << 9) | ((kword_t)6U << 12))

static const kword_t mfsproc_file_names[MONITORFS_PROCESS_FILE_COUNT] = {
        VFS_SIX6('P','P','I','D',' ',' '),
        VFS_SIX6('S','T','A','T','E',' '),
        VFS_SIX6('W','O','R','D','S',' '),
        VFS_SIX6('C','O','M','M',' ',' '),
        VFS_SIX6('S','T','A','T','U','S')
};

static inline unsigned int
mfsproc_file_chars(unsigned int index)
{
        return (unsigned int)((MONITORFS_PROCESS_FILE_LENGTHS >>
            (index * MONITORFS_PROCESS_FILE_LEN_BITS)) & MONITORFS_PROCESS_FILE_LEN_MASK);
}

static inline int
mfsproc_file_kind(unsigned int kind)
{
        return kind >= MONITORFS_PROCESS_KIND_PPID && kind <= MONITORFS_PROCESS_KIND_STATUS;
}

int
mfsproc_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
{
        unsigned int kind;
        unsigned int slot;

        if (name == 0 || nodep == 0 || VFS_PROVIDER(dir) != MONITORFS_PROCESS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(dir);
        if (kind == MONITORFS_PROCESS_KIND_ROOT) {
                if (mfsproc_parse_slot(name, &slot) != 0 ||
                    !mfsproc_slot_active(slot))
                        return -1;
                *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER, MONITORFS_PROCESS_KIND_PROC, slot);
                return 0;
        }
        if (kind != MONITORFS_PROCESS_KIND_PROC)
                return -1;
        slot = VFS_INDEX(dir);
        if (!mfsproc_slot_active(slot))
                return -1;
        for (kind = MONITORFS_PROCESS_KIND_PPID; kind <= MONITORFS_PROCESS_KIND_STATUS; ++kind) {
                unsigned int index;

                index = kind - MONITORFS_PROCESS_KIND_PPID;
                if (vfs_name_is6(name, mfsproc_file_names[index],
                    mfsproc_file_chars(index)))
                        break;
        }
        if (kind > MONITORFS_PROCESS_KIND_STATUS)
                return -1;
        *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER, kind, slot);
        return 0;
}

int
mfsproc_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
{
        unsigned int kind;
        unsigned int slot;
        unsigned int seen;

        if (ent == 0 || VFS_PROVIDER(dir) != MONITORFS_PROCESS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(dir);
        if (kind == MONITORFS_PROCESS_KIND_ROOT) {
                if (proc_table == 0)
                        return 0;
                seen = 0U;
                for (slot = 0U; slot < proc_high_slot; ++slot) {
                        if (!mfsproc_slot_active(slot))
                                continue;
                        if (seen++ != off)
                                continue;
                        mfsproc_format_slot(slot, &ent->name);
                        ent->type = VFS_TYPE_DIR;
                        return 1;
                }
                return 0;
        }
        if (kind != MONITORFS_PROCESS_KIND_PROC || !mfsproc_slot_active(VFS_INDEX(dir)))
                return -1;
        if (off >= MONITORFS_PROCESS_FILE_COUNT)
                return 0;
        mfsproc_dirent_set(ent, mfsproc_file_names[off],
            mfsproc_file_chars(off), VFS_TYPE_REG);
        return 1;
}

int
mfsproc_stat(vnode_t node, struct vfs_stat *st)
{
        unsigned int kind;
        unsigned int slot;

        if (st == 0 || VFS_PROVIDER(node) != MONITORFS_PROCESS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(node);
        slot = VFS_INDEX(node);
        if (kind == MONITORFS_PROCESS_KIND_ROOT) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
        } else if (kind == MONITORFS_PROCESS_KIND_PROC && mfsproc_slot_active(slot)) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
        } else if (mfsproc_file_kind(kind) && mfsproc_slot_active(slot)) {
                st->type = VFS_TYPE_REG;
                st->mode = 0444U;
        } else {
                return -1;
        }
        st->size_chars = 0UL;
        st->size_words = 0UL;
        return 0;
}

/*
 * STATUS is deliberately one vnode kind: struct file stores descriptor
 * metadata in vnode bits that are otherwise zero only while local kinds stay
 * in 0..7.  IDs are fixed-width three-digit octal values so the PDP-6 can
 * emit them with shifts/masks instead of resident decimal division code:
 *
 *   PID PPID PGRP SID DID S\r\n
 */
static int
mfsproc_status_readchar(struct proc *p, unsigned int slot, kword_t off,
    unsigned int *chp)
{
        unsigned int field;
        unsigned int pos;
        unsigned int value;
        unsigned int state;

        if (off < 20UL) {
                field = (unsigned int)off >> 2;
                pos = (unsigned int)off & 3U;
                if (pos == 3U) {
                        *chp = (unsigned int)' ';
                        return 1;
                }
                if (field == 0U)
                        value = slot;
                else if (field == 1U)
                        value = PROC_PARENT_SLOT(p);
                else if (field == 2U)
                        value = PROC_PGRP(p);
                else {
                        value = (unsigned int)proc_scope_id(p);
                        if (field != 3U)
                                value >>= PROC_ZOMB_DOMAIN_SHIFT;
                        value &= (unsigned int)PROC_ZOMB_SESSION_MASK;
                }
                *chp = (unsigned int)'0' +
                    ((value >> ((2U - pos) * 3U)) & 07U);
                return 1;
        }
        if (off == 20UL) {
                state = PROC_STATE(p);
                if (state == PROC_ZOMB)
                        *chp = (unsigned int)'Z';
                else if (slot != 0U && !VM_SPACE_ACTIVE(p))
                        *chp = (unsigned int)'W';
                else if (state == PROC_SIDL)
                        *chp = (unsigned int)'I';
                else if (state == PROC_SRUN)
                        *chp = (unsigned int)'R';
                else if (state == PROC_SLEEP)
                        *chp = (unsigned int)'S';
                else if (state == PROC_STOP)
                        *chp = (unsigned int)'T';
                else
                        *chp = (unsigned int)'F';
                return 1;
        }
        if (off == 21UL) {
                *chp = 015U;
                return 1;
        }
        if (off == 22UL) {
                *chp = 012U;
                return 1;
        }
        return 0;
}

int
mfsproc_readchar(vnode_t node, kword_t off, unsigned int *chp)
{
        struct proc *p;
        kword_t value;
        kword_t name;
        unsigned int chars;
        unsigned int kind;
        unsigned int slot;
        unsigned int state;

        if (chp == 0 || VFS_PROVIDER(node) != MONITORFS_PROCESS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(node);
        slot = VFS_INDEX(node);
        if (!mfsproc_file_kind(kind) || !mfsproc_slot_active(slot))
                return -1;
        p = &proc_table[slot];
        if (kind == MONITORFS_PROCESS_KIND_COMM)
                return vfs_sixbit_readchar(proc_comm(p), 6U, off, chp);
        if (kind == MONITORFS_PROCESS_KIND_STATUS)
                return mfsproc_status_readchar(p, slot, off, chp);
        if (kind == MONITORFS_PROCESS_KIND_STATE) {
                state = PROC_STATE(p);
                chars = 4U;
                if (state == PROC_ZOMB) {
                        name = VFS_SIX6('Z','O','M','B',' ',' ');
                } else if (slot != 0U && !VM_SPACE_ACTIVE(p)) {
                        name = VFS_SIX6('S','W','A','P',' ',' ');
                } else if (state == PROC_SIDL) {
                        name = VFS_SIX6('I','D','L',' ',' ',' ');
                        chars = 3U;
                } else if (state == PROC_SRUN) {
                        name = VFS_SIX6('R','U','N',' ',' ',' ');
                        chars = 3U;
                } else if (state == PROC_SLEEP) {
                        name = VFS_SIX6('S','L','E','E','P',' ');
                        chars = 5U;
                } else if (state == PROC_STOP) {
                        name = VFS_SIX6('S','T','O','P',' ',' ');
                } else {
                        name = VFS_SIX6('F','R','E','E',' ',' ');
                }
                return vfs_sixbit_readchar(name, chars, off, chp);
        }
        if (kind == MONITORFS_PROCESS_KIND_PPID)
                value = (kword_t)PROC_PARENT_SLOT(p);
        else if (kind == MONITORFS_PROCESS_KIND_WORDS)
                value = VM_SPACE_WORDS(p);
        else
                return -1;
        return kfmt_u18_decimal_readchar(value, off, chp);
}

/* Assembly file_getcwd uses this for /MONITOR/PROC/<slot> directories. */


#include "monitorfs.h"
#include "proc.h"
#include "proc_swap.h"
#include "vm.h"

#define MonitorFS domain view_STATUS_WORDS 6U

static unsigned int
mfsdom_proc_domain(const struct proc *p)
{
        if (PROC_STATE(p) == PROC_ZOMB)
                return PROC_ZOMB_DOMAIN(p);
        if (!PROC_HAS_UAREA(p))
                return 0U;
        return PROC_DOMAIN(p);
}

static int
mfsdom_slot_active(unsigned int slot)
{
        return proc_table != 0 && slot < proc_high_slot &&
            !PROC_IS_FREE(&proc_table[slot]);
}

static int
mfsdom_exists(unsigned int did)
{
        unsigned int slot;

        if (proc_table == 0 || did >= PROC_MAX_SLOTS)
                return 0;
        for (slot = 0U; slot < proc_high_slot; ++slot) {
                if (!mfsdom_slot_active(slot))
                        continue;
                if (mfsdom_proc_domain(&proc_table[slot]) == did)
                        return 1;
        }
        return 0;
}

static int
mfsdom_parse_id(const struct vfs_name *name, unsigned int *didp)
{
        unsigned int i;
        unsigned int ch;
        unsigned int value;

        if (name == 0 || didp == 0 || name->chars == 0U || name->chars > 3U)
                return -1;
        value = 0U;
        for (i = 0U; i < name->chars; ++i) {
                ch = vfs_name_char(name, i);
                if (ch < (unsigned int)VFS_SIXCHAR('0') ||
                    ch > (unsigned int)VFS_SIXCHAR('9'))
                        return -1;
                value = value * 10U + ch - (unsigned int)VFS_SIXCHAR('0');
        }
        if (value >= PROC_MAX_SLOTS)
                return -1;
        *didp = value;
        return 0;
}

static void
mfsdom_name_id(unsigned int did, struct vfs_name *name)
{
        unsigned int h;
        unsigned int t;
        unsigned int o;
        kword_t word;

        h = did / 100U;
        t = (did / 10U) % 10U;
        o = did % 10U;
        word = 0UL;
        if (h != 0U) {
                word = VFS_SIXCHAR('0' + h) << 30U;
                word |= VFS_SIXCHAR('0' + t) << 24U;
                word |= VFS_SIXCHAR('0' + o) << 18U;
                name->chars = 3U;
        } else if (t != 0U) {
                word = VFS_SIXCHAR('0' + t) << 30U;
                word |= VFS_SIXCHAR('0' + o) << 24U;
                name->chars = 2U;
        } else {
                word = VFS_SIXCHAR('0' + o) << 30U;
                name->chars = 1U;
        }
        name->words[0] = word;
        name->words[1] = 0UL;
        name->words[2] = 0UL;
        name->words[3] = 0UL;
}

static void
mfsdom_status(unsigned int did, kword_t status[MonitorFS domain view_STATUS_WORDS])
{
        struct proc *p;
        unsigned int slot;

        status[0] = (kword_t)did;
        status[1] = 0UL;
        status[2] = 0UL;
        status[3] = 0UL;
        status[4] = 0UL;
        status[5] = 0UL;
        if (proc_table == 0)
                return;
        for (slot = 0U; slot < proc_high_slot; ++slot) {
                if (!mfsdom_slot_active(slot))
                        continue;
                p = &proc_table[slot];
                if (mfsdom_proc_domain(p) != did)
                        continue;
                ++status[1];
                if (PROC_STATE(p) == PROC_ZOMB)
                        continue;
                if (VM_SPACE_ACTIVE(p)) {
                        status[2] += VM_SPACE_WORDS(p);
                } else if (slot != 0U && proc_swap_records != 0 &&
                    proc_swap_records[slot].state != 0UL) {
                        ++status[3];
                        status[4] += proc_swap_records[slot].state &
                            PROC_HALF_MASK;
                }
                if (PROC_STATE(p) == PROC_STOP)
                        ++status[5];
        }
}

int
mfsdom_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
{
        unsigned int kind;
        unsigned int did;

        if (name == 0 || nodep == 0 || VFS_PROVIDER(dir) != MONITORFS_PROCESS_PROVIDER ||
            !MONITORFS_IS_DOMAIN(dir))
                return -1;
        kind = VFS_LOCAL_KIND(dir);
        if (kind == MONITORFS_PROCESS_KIND_ROOT) {
                if (mfsdom_parse_id(name, &did) != 0 || !mfsdom_exists(did))
                        return -1;
                *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER, MONITORFS_PROCESS_KIND_PROC,
                    MONITORFS_DOMAIN_TAG | did);
                return 0;
        }
        if (kind != MONITORFS_PROCESS_KIND_PROC)
                return -1;
        did = MONITORFS_PROCESS_ID(dir);
        if (!mfsdom_exists(did) ||
            !vfs_name_is6(name, VFS_SIX6('S','T','A','T','U','S'), 6U))
                return -1;
        *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER, MONITORFS_PROCESS_KIND_STATUS,
            MONITORFS_DOMAIN_TAG | did);
        return 0;
}

int
mfsdom_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
{
        unsigned int did;
        unsigned int seen;

        if (ent == 0 || !MONITORFS_IS_DOMAIN(dir))
                return -1;
        if (VFS_LOCAL_KIND(dir) == MONITORFS_PROCESS_KIND_PROC) {
                if (!mfsdom_exists(MONITORFS_PROCESS_ID(dir)))
                        return -1;
                if (off != 0U)
                        return 0;
                ent->name.chars = 6U;
                ent->name.words[0] = VFS_SIX6('S','T','A','T','U','S');
                ent->name.words[1] = 0UL;
                ent->name.words[2] = 0UL;
                ent->name.words[3] = 0UL;
                ent->type = VFS_TYPE_REG;
                return 1;
        }
        if (VFS_LOCAL_KIND(dir) != MONITORFS_PROCESS_KIND_ROOT || proc_table == 0)
                return -1;
        seen = 0U;
        for (did = 0U; did < PROC_MAX_SLOTS; ++did) {
                if (!mfsdom_exists(did))
                        continue;
                if (seen++ != off)
                        continue;
                mfsdom_name_id(did, &ent->name);
                ent->type = VFS_TYPE_DIR;
                return 1;
        }
        return 0;
}

int
mfsdom_stat(vnode_t node, struct vfs_stat *st)
{
        unsigned int kind;

        if (st == 0 || !MONITORFS_IS_DOMAIN(node))
                return -1;
        kind = VFS_LOCAL_KIND(node);
        if (kind == MONITORFS_PROCESS_KIND_ROOT) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
                st->size_words = 0UL;
        } else if (kind == MONITORFS_PROCESS_KIND_PROC &&
            mfsdom_exists(MONITORFS_PROCESS_ID(node))) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
                st->size_words = 0UL;
        } else if (kind == MONITORFS_PROCESS_KIND_STATUS &&
            mfsdom_exists(MONITORFS_PROCESS_ID(node))) {
                st->type = VFS_TYPE_REG;
                st->mode = 0444U;
                st->size_words = MonitorFS domain view_STATUS_WORDS;
        } else {
                return -1;
        }
        st->size_chars = 0UL;
        return 0;
}

int
mfsdom_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        kword_t status[MonitorFS domain view_STATUS_WORDS];
        unsigned int did;
        unsigned int count;
        unsigned int i;

        if (buf == 0 || !MONITORFS_IS_DOMAIN(node) ||
            VFS_LOCAL_KIND(node) != MONITORFS_PROCESS_KIND_STATUS)
                return -1;
        did = MONITORFS_PROCESS_ID(node);
        if (!mfsdom_exists(did))
                return -1;
        if (off >= MonitorFS domain view_STATUS_WORDS)
                return 0;
        count = MonitorFS domain view_STATUS_WORDS - off;
        if (count > nwords)
                count = nwords;
        mfsdom_status(did, status);
        for (i = 0U; i < count; ++i)
                buf[i] = status[off + i];
        return (int)count;
}
