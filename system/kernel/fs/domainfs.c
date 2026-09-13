#include "procfs.h"
#include "proc.h"
#include "proc_swap.h"
#include "vm.h"

#define DOMAINFS_STATUS_WORDS 6U

static unsigned int
domainfs_proc_domain(const struct proc *p)
{
        if (PROC_STATE(p) == PROC_ZOMB)
                return PROC_ZOMB_DOMAIN(p);
        if (!PROC_HAS_UAREA(p))
                return 0U;
        return PROC_DOMAIN(p);
}

static int
domainfs_slot_active(unsigned int slot)
{
        return proc_table != 0 && slot < proc_high_slot &&
            !PROC_IS_FREE(&proc_table[slot]);
}

static int
domainfs_exists(unsigned int did)
{
        unsigned int slot;

        if (proc_table == 0 || did >= PROC_MAX_SLOTS)
                return 0;
        for (slot = 0U; slot < proc_high_slot; ++slot) {
                if (!domainfs_slot_active(slot))
                        continue;
                if (domainfs_proc_domain(&proc_table[slot]) == did)
                        return 1;
        }
        return 0;
}

static int
domainfs_parse_id(const struct vfs_name *name, unsigned int *didp)
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
domainfs_name_id(unsigned int did, struct vfs_name *name)
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
domainfs_status(unsigned int did, kword_t status[DOMAINFS_STATUS_WORDS])
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
                if (!domainfs_slot_active(slot))
                        continue;
                p = &proc_table[slot];
                if (domainfs_proc_domain(p) != did)
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
domainfs_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
{
        unsigned int kind;
        unsigned int did;

        if (name == 0 || nodep == 0 || VFS_PROVIDER(dir) != PROCFS_PROVIDER ||
            !PROCFS_IS_DOMAIN(dir))
                return -1;
        kind = VFS_LOCAL_KIND(dir);
        if (kind == PROCFS_KIND_ROOT) {
                if (domainfs_parse_id(name, &did) != 0 || !domainfs_exists(did))
                        return -1;
                *nodep = VFS_NODE(PROCFS_PROVIDER, PROCFS_KIND_PROC,
                    PROCFS_DOMAIN_TAG | did);
                return 0;
        }
        if (kind != PROCFS_KIND_PROC)
                return -1;
        did = PROCFS_DOMAIN_ID(dir);
        if (!domainfs_exists(did) ||
            !vfs_name_is6(name, VFS_SIX6('S','T','A','T','U','S'), 6U))
                return -1;
        *nodep = VFS_NODE(PROCFS_PROVIDER, PROCFS_KIND_STATUS,
            PROCFS_DOMAIN_TAG | did);
        return 0;
}

int
domainfs_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
{
        unsigned int did;
        unsigned int seen;

        if (ent == 0 || !PROCFS_IS_DOMAIN(dir))
                return -1;
        if (VFS_LOCAL_KIND(dir) == PROCFS_KIND_PROC) {
                if (!domainfs_exists(PROCFS_DOMAIN_ID(dir)))
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
        if (VFS_LOCAL_KIND(dir) != PROCFS_KIND_ROOT || proc_table == 0)
                return -1;
        seen = 0U;
        for (did = 0U; did < PROC_MAX_SLOTS; ++did) {
                if (!domainfs_exists(did))
                        continue;
                if (seen++ != off)
                        continue;
                domainfs_name_id(did, &ent->name);
                ent->type = VFS_TYPE_DIR;
                return 1;
        }
        return 0;
}

int
domainfs_stat(vnode_t node, struct vfs_stat *st)
{
        unsigned int kind;

        if (st == 0 || !PROCFS_IS_DOMAIN(node))
                return -1;
        kind = VFS_LOCAL_KIND(node);
        if (kind == PROCFS_KIND_ROOT) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
                st->size_words = 0UL;
        } else if (kind == PROCFS_KIND_PROC &&
            domainfs_exists(PROCFS_DOMAIN_ID(node))) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0555U;
                st->size_words = 0UL;
        } else if (kind == PROCFS_KIND_STATUS &&
            domainfs_exists(PROCFS_DOMAIN_ID(node))) {
                st->type = VFS_TYPE_REG;
                st->mode = 0444U;
                st->size_words = DOMAINFS_STATUS_WORDS;
        } else {
                return -1;
        }
        st->size_chars = 0UL;
        return 0;
}

int
domainfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        kword_t status[DOMAINFS_STATUS_WORDS];
        unsigned int did;
        unsigned int count;
        unsigned int i;

        if (buf == 0 || !PROCFS_IS_DOMAIN(node) ||
            VFS_LOCAL_KIND(node) != PROCFS_KIND_STATUS)
                return -1;
        did = PROCFS_DOMAIN_ID(node);
        if (!domainfs_exists(did))
                return -1;
        if (off >= DOMAINFS_STATUS_WORDS)
                return 0;
        count = DOMAINFS_STATUS_WORDS - off;
        if (count > nwords)
                count = nwords;
        domainfs_status(did, status);
        for (i = 0U; i < count; ++i)
                buf[i] = status[off + i];
        return (int)count;
}

int
domainfs_getcwd_did(unsigned int did, kword_t *buf, unsigned int nwords)
{
        struct vfs_name name;
        kword_t tail;
        unsigned int i;

        if (buf == 0 || nwords < 3U || !domainfs_exists(did))
                return -1;
        domainfs_name_id(did, &name);
        buf[0] = 8UL + (kword_t)name.chars;
        buf[1] = VFS_SIX6('/','D','O','M','A','I');
        tail = VFS_SIXCHAR('N') << 30U;
        tail |= VFS_SIXCHAR('/') << 24U;
        for (i = 0U; i < name.chars; ++i)
                tail |= (kword_t)vfs_name_char(&name, i) << (18U - i * 6U);
        buf[2] = tail;
        return 0;
}
