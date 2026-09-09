#include "procfs.h"
#include "kfmt.h"
#include "proc.h"

static int
procfs_slot_active(unsigned int slot)
{
        return proc_table != 0 && slot < proc_slots &&
            PROC_STATE(&proc_table[slot]) != PROC_FREE;
}

static void
procfs_name_set(struct vfs_name *name, kword_t word, unsigned int chars)
{
        name->chars = chars;
        name->words[0] = word;
        name->words[1] = 0UL;
        name->words[2] = 0UL;
        name->words[3] = 0UL;
}

static void
procfs_dirent_set(struct vfs_dirent *ent, kword_t word,
    unsigned int chars, unsigned int type)
{
        procfs_name_set(&ent->name, word, chars);
        ent->type = type;
}

static int
procfs_parse_slot(const struct vfs_name *name, unsigned int *slotp)
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
procfs_format_slot(unsigned int slot, struct vfs_name *name)
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
        procfs_name_set(name, word, chars);
}

static int
procfs_file_kind(unsigned int kind)
{
        return kind >= PROCFS_KIND_PPID && kind <= PROCFS_KIND_COMM;
}

int
procfs_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
{
        unsigned int kind;
        unsigned int slot;

        if (name == 0 || nodep == 0 || VFS_PROVIDER(dir) != PROCFS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(dir);
        if (kind == PROCFS_KIND_ROOT) {
                if (procfs_parse_slot(name, &slot) != 0 ||
                    !procfs_slot_active(slot))
                        return -1;
                *nodep = VFS_NODE(PROCFS_PROVIDER, PROCFS_KIND_PROC, slot);
                return 0;
        }
        if (kind != PROCFS_KIND_PROC)
                return -1;
        slot = VFS_INDEX(dir);
        if (!procfs_slot_active(slot))
                return -1;
        if (vfs_name_is6(name, VFS_SIX6('P','P','I','D',' ',' '), 4U))
                kind = PROCFS_KIND_PPID;
        else if (vfs_name_is6(name, VFS_SIX6('S','T','A','T','E',' '), 5U))
                kind = PROCFS_KIND_STATE;
        else if (vfs_name_is6(name, VFS_SIX6('W','O','R','D','S',' '), 5U))
                kind = PROCFS_KIND_WORDS;
        else if (vfs_name_is6(name, VFS_SIX6('C','O','M','M',' ',' '), 4U))
                kind = PROCFS_KIND_COMM;
        else
                return -1;
        *nodep = VFS_NODE(PROCFS_PROVIDER, kind, slot);
        return 0;
}

int
procfs_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
{
        unsigned int kind;
        unsigned int slot;
        unsigned int seen;

        if (ent == 0 || VFS_PROVIDER(dir) != PROCFS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(dir);
        if (kind == PROCFS_KIND_ROOT) {
                if (proc_table == 0)
                        return 0;
                seen = 0U;
                for (slot = 0U; slot < proc_high_slot; ++slot) {
                        if (!procfs_slot_active(slot))
                                continue;
                        if (seen++ != off)
                                continue;
                        procfs_format_slot(slot, &ent->name);
                        ent->type = VFS_TYPE_DIR;
                        return 1;
                }
                return 0;
        }
        if (kind != PROCFS_KIND_PROC || !procfs_slot_active(VFS_INDEX(dir)))
                return -1;
        if (off >= 4U)
                return 0;
        if (off == 0U)
                procfs_dirent_set(ent, VFS_SIX6('P','P','I','D',' ',' '),
                    4U, VFS_TYPE_REG);
        else if (off == 1U)
                procfs_dirent_set(ent, VFS_SIX6('S','T','A','T','E',' '),
                    5U, VFS_TYPE_REG);
        else if (off == 2U)
                procfs_dirent_set(ent, VFS_SIX6('W','O','R','D','S',' '),
                    5U, VFS_TYPE_REG);
        else
                procfs_dirent_set(ent, VFS_SIX6('C','O','M','M',' ',' '),
                    4U, VFS_TYPE_REG);
        return 1;
}

int
procfs_stat(vnode_t node, struct vfs_stat *st)
{
        unsigned int kind;
        unsigned int slot;

        if (st == 0 || VFS_PROVIDER(node) != PROCFS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(node);
        slot = VFS_INDEX(node);
        if (kind == PROCFS_KIND_ROOT) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
        } else if (kind == PROCFS_KIND_PROC && procfs_slot_active(slot)) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
        } else if (procfs_file_kind(kind) && procfs_slot_active(slot)) {
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
procfs_readchar(vnode_t node, kword_t off, unsigned int *chp)
{
        struct proc *p;
        kword_t value;
        kword_t name;
        unsigned int chars;
        unsigned int kind;
        unsigned int slot;
        unsigned int state;

        if (chp == 0 || VFS_PROVIDER(node) != PROCFS_PROVIDER)
                return -1;
        kind = VFS_LOCAL_KIND(node);
        slot = VFS_INDEX(node);
        if (!procfs_file_kind(kind) || !procfs_slot_active(slot))
                return -1;
        p = &proc_table[slot];
        if (kind == PROCFS_KIND_COMM)
                return vfs_sixbit_readchar(proc_comm(p), 6U, off, chp);
        if (kind == PROCFS_KIND_STATE) {
                state = PROC_STATE(p);
                chars = 4U;
                if (slot != 0U && PROC_MEM_BASE(p) == 0UL) {
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
                } else if (state == PROC_ZOMB) {
                        name = VFS_SIX6('Z','O','M','B',' ',' ');
                } else if (state == PROC_STOP) {
                        name = VFS_SIX6('S','T','O','P',' ',' ');
                } else {
                        name = VFS_SIX6('F','R','E','E',' ',' ');
                }
                return vfs_sixbit_readchar(name, chars, off, chp);
        }
        value = kind == PROCFS_KIND_PPID ?
            (kword_t)PROC_PARENT_SLOT(p) : PROC_MEM_WORDS(p);
        return kfmt_u36_decimal_readchar(value, off, chp);
}

/* Assembly file_getcwd uses this for synthetic /PROC/<slot> directories. */
int
procfs_getcwd_slot(unsigned int slot, kword_t *buf, unsigned int nwords)
{
        struct vfs_name name;

        if (buf == 0 || nwords < 3U || !procfs_slot_active(slot))
                return -1;
        procfs_format_slot(slot, &name);
        buf[0] = 6UL + (kword_t)name.chars;
        buf[1] = VFS_SIX6('/','P','R','O','C','/');
        buf[2] = name.words[0];
        return 0;
}
