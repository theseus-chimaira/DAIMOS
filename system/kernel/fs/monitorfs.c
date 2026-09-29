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

#define MONITORFS_PROCESS_FILE_COUNT 6U

static const struct vfs_name mfsproc_file_names[MONITORFS_PROCESS_FILE_COUNT] = {
        { 4U, { VFS_SIX6('P','P','I','D',' ',' '), 0UL, 0UL, 0UL } },
        { 5U, { VFS_SIX6('S','T','A','T','E',' '), 0UL, 0UL, 0UL } },
        { 5U, { VFS_SIX6('W','O','R','D','S',' '), 0UL, 0UL, 0UL } },
        { 4U, { VFS_SIX6('N','A','M','E',' ',' '), 0UL, 0UL, 0UL } },
        { 7U, { VFS_SIX6('C','M','D','L','I','N'),
            VFS_SIX6('E',' ',' ',' ',' ',' '), 0UL, 0UL } },
        { 11U, { VFS_SIX6('E','N','V','I','R','O'),
            VFS_SIX6('N','M','E','N','T',' '), 0UL, 0UL } }
};

static int
mfsproc_name_equal(const struct vfs_name *a, const struct vfs_name *b)
{
        return a->chars == b->chars &&
            vfs_name_words_equal(a->words, b->words, VFS_NAME_WORDS);
}

static int mfsdom_readchar(vnode_t node, kword_t off, unsigned int *chp);

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
                *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER,
                    MONITORFS_PROCESS_KIND_DIR, slot);
                return 0;
        }
        if (kind != MONITORFS_PROCESS_KIND_DIR)
                return -1;
        slot = MONITORFS_PROCESS_ID(dir);
        if (!mfsproc_slot_active(slot))
                return -1;
        for (kind = 0U; kind < MONITORFS_PROCESS_FILE_COUNT; ++kind)
                if (mfsproc_name_equal(name, &mfsproc_file_names[kind]))
                        break;
        if (kind >= MONITORFS_PROCESS_FILE_COUNT)
                return -1;
        *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER,
            MONITORFS_PROCESS_KIND_FILE,
            MONITORFS_PROCESS_INDEX(slot, kind));
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
        if (kind != MONITORFS_PROCESS_KIND_DIR ||
            !mfsproc_slot_active(MONITORFS_PROCESS_ID(dir)))
                return -1;
        if (off >= MONITORFS_PROCESS_FILE_COUNT)
                return 0;
        ent->name = mfsproc_file_names[off];
        ent->type = VFS_TYPE_REG;
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
        slot = MONITORFS_PROCESS_ID(node);
        if (kind == MONITORFS_PROCESS_KIND_ROOT) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
        } else if (kind == MONITORFS_PROCESS_KIND_DIR && mfsproc_slot_active(slot)) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
        } else if (kind == MONITORFS_PROCESS_KIND_FILE &&
            MONITORFS_PROCESS_LEAF(node) < MONITORFS_PROCESS_FILE_COUNT &&
            mfsproc_slot_active(slot)) {
                st->type = VFS_TYPE_REG;
                st->mode = 0444U;
        } else {
                return -1;
        }
        st->size_chars = 0UL;
        st->size_words = 0UL;
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
        unsigned int leaf;
        unsigned int slot;
        unsigned int state;

        if (chp == 0 || VFS_PROVIDER(node) != MONITORFS_PROCESS_PROVIDER)
                return -1;
        if (MONITORFS_IS_DOMAIN(node))
                return mfsdom_readchar(node, off, chp);
        kind = VFS_LOCAL_KIND(node);
        slot = MONITORFS_PROCESS_ID(node);
        leaf = MONITORFS_PROCESS_LEAF(node);
        if (kind != MONITORFS_PROCESS_KIND_FILE ||
            leaf >= MONITORFS_PROCESS_FILE_COUNT || !mfsproc_slot_active(slot))
                return -1;
        p = &proc_table[slot];
        if (leaf == MONITORFS_PROCESS_LEAF_NAME)
                return proc_image_text_readchar(slot, PROC_IMAGE_VIEW_NAME,
                    off, chp);
        if (leaf == MONITORFS_PROCESS_LEAF_CMDLINE)
                return proc_image_text_readchar(slot, PROC_IMAGE_VIEW_CMDLINE,
                    off, chp);
        if (leaf == MONITORFS_PROC_LEAF_ENV)
                return proc_image_text_readchar(slot,
                    PROC_IMAGE_VIEW_ENVIRONMENT, off, chp);
        if (leaf == MONITORFS_PROCESS_LEAF_STATE) {
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
        if (leaf == MONITORFS_PROCESS_LEAF_PPID)
                value = (kword_t)PROC_PARENT_SLOT(p);
        else if (leaf == MONITORFS_PROCESS_LEAF_WORDS)
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

#define MONITORFS_DOMAIN_STATUS_WORDS 6U

static const struct vfs_name mfsdom_file_names[] = {
        { 9U, { VFS_SIX6('P','R','O','C','E','S'),
            VFS_SIX6('S','E','S',' ',' ',' '), 0UL, 0UL } },
        { 5U, { VFS_SIX6('W','O','R','D','S',' '), 0UL, 0UL, 0UL } },
        { 7U, { VFS_SIX6('S','W','A','P','P','E'),
            VFS_SIX6('D',' ',' ',' ',' ',' '), 0UL, 0UL } },
        { 9U, { VFS_SIX6('S','W','A','P','W','O'),
            VFS_SIX6('R','D','S',' ',' ',' '), 0UL, 0UL } },
        { 7U, { VFS_SIX6('S','T','O','P','P','E'),
            VFS_SIX6('D',' ',' ',' ',' ',' '), 0UL, 0UL } },
        { 4U, { VFS_SIX6('P','I','D','S',' ',' '), 0UL, 0UL, 0UL } }
};

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
mfsdom_status(unsigned int did, kword_t status[MONITORFS_DOMAIN_STATUS_WORDS])
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
                        status[4] += (proc_swap_records[slot].state &
                            PROC_HALF_MASK) * 0200UL;
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
                *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER,
                    MONITORFS_PROCESS_KIND_DIR,
                    MONITORFS_DOMAIN_TAG | did);
                return 0;
        }
        if (kind != MONITORFS_PROCESS_KIND_DIR)
                return -1;
        did = MONITORFS_PROCESS_ID(dir);
        if (!mfsdom_exists(did))
                return -1;
        for (kind = 0U; kind < sizeof(mfsdom_file_names) /
            sizeof(mfsdom_file_names[0]); ++kind)
                if (mfsproc_name_equal(name, &mfsdom_file_names[kind]))
                        break;
        if (kind >= sizeof(mfsdom_file_names) /
            sizeof(mfsdom_file_names[0]))
                return -1;
        *nodep = VFS_NODE(MONITORFS_PROCESS_PROVIDER,
            MONITORFS_PROCESS_KIND_FILE,
            MONITORFS_DOMAIN_TAG | MONITORFS_PROCESS_INDEX(did, kind));
        return 0;
}

int
mfsdom_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
{
        unsigned int did;
        unsigned int seen;

        if (ent == 0 || !MONITORFS_IS_DOMAIN(dir))
                return -1;
        if (VFS_LOCAL_KIND(dir) == MONITORFS_PROCESS_KIND_DIR) {
                if (!mfsdom_exists(MONITORFS_PROCESS_ID(dir)))
                        return -1;
                if (off >= sizeof(mfsdom_file_names) /
                    sizeof(mfsdom_file_names[0]))
                        return 0;
                ent->name = mfsdom_file_names[off];
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
        } else if (kind == MONITORFS_PROCESS_KIND_DIR &&
            mfsdom_exists(MONITORFS_PROCESS_ID(node))) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
                st->size_words = 0UL;
        } else if (kind == MONITORFS_PROCESS_KIND_FILE &&
            MONITORFS_PROCESS_LEAF(node) <= MONITORFS_DOMAIN_LEAF_PIDS &&
            mfsdom_exists(MONITORFS_PROCESS_ID(node))) {
                st->type = VFS_TYPE_REG;
                st->mode = 0444U;
                st->size_words = 0UL;
        } else {
                return -1;
        }
        st->size_chars = 0UL;
        return 0;
}

static int
mfsdom_readchar(vnode_t node, kword_t off, unsigned int *chp)
{
        kword_t status[MONITORFS_DOMAIN_STATUS_WORDS];
        unsigned int did;
        unsigned int leaf;
        unsigned int line;
        unsigned int pos;
        unsigned int slot;
        unsigned int seen;
        unsigned int value;

        if (chp == 0 || !MONITORFS_IS_DOMAIN(node) ||
            VFS_LOCAL_KIND(node) != MONITORFS_PROCESS_KIND_FILE)
                return -1;
        did = MONITORFS_DOMAIN_ID(node);
        leaf = MONITORFS_PROCESS_LEAF(node);
        if (!mfsdom_exists(did) || leaf > MONITORFS_DOMAIN_LEAF_PIDS)
                return -1;
        if (leaf != MONITORFS_DOMAIN_LEAF_PIDS) {
                mfsdom_status(did, status);
                value = (unsigned int)status[leaf + 1U];
                return kfmt_u18_decimal_readchar((kword_t)value, off, chp);
        }

        line = (unsigned int)(off / 5UL);
        pos = (unsigned int)(off % 5UL);
        seen = 0U;
        for (slot = 0U; slot < proc_high_slot; ++slot) {
                if (!mfsdom_slot_active(slot) ||
                    mfsdom_proc_domain(&proc_table[slot]) != did)
                        continue;
                if (seen++ != line)
                        continue;
                if (pos == 3U)
                        *chp = 015U;
                else if (pos == 4U)
                        *chp = 012U;
                else
                        *chp = (unsigned int)'0' +
                            ((slot >> ((2U - pos) * 3U)) & 07U);
                return 1;
        }
        return 0;
}
