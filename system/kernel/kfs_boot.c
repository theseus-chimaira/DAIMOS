#include "kinit.h"
#include "kboot.h"
#include "mm.h"
#include "initfs.h"
#include "exec.h"
#include "proc.h"
#include "file.h"
#include "dtfs.h"
#include "dsk270.h"
#include "module.h"
#include "syscall.h"
#include "fs_mres.h"
#include "diskset_mres.h"


#define HALF_MASK       0777777UL
#define TYPE_SHIFT      15U
#define TYPE_MASK       07UL
#define MODE_SHIFT      3U
#define MODE_MASK       07777UL
#define FLAGS_MASK      07UL

#define IHF_MAGIC       0U
#define IHF_VERSION     1U
#define IHF_NENT        2U
#define IHF_ENT_WORDS   3U
#define IHF_STR_WORDS   4U
#define IHF_DATA_WORDS  5U
#define IHF_FLAGS       6U
#define IHF_CKSUM       7U

#define IEF_NAME_OFF    0U
#define IEF_TYPE        1U
#define IEF_MODE        2U
#define IEF_DATA_OFF    3U
#define IEF_SIZE_WORDS  4U
#define IEF_SIZE_9BYTES 5U
#define IEF_AUX         6U
#define IEF_FLAGS       7U

extern kword_t __initfs_begin;
extern kword_t __initfs_begin_end;

static struct memfs_node boot_nodes[KBOOT_NODE_COUNT];

/* KINIT-only handles for moving the RAMFS mount out of the bootstrap
 * namespace after D6FS becomes the live root. */
static vnode_t boot_memfs_root = VFS_NODE_NONE;
static vnode_t boot_ramfs_root = VFS_NODE_NONE;
static unsigned int boot_ramfs_slot;

static void
boot_clear_node(struct memfs_node *np)
{
        unsigned int i;

        for (i = 0U; i < VFS_NAME_WORDS; ++i)
                np->name.words[i] = 0;
        np->name.chars = 0U;
        np->meta = 0;
        np->size_chars = 0;
        np->data = 0;
}

static void
boot_set_node(struct memfs_node *np, unsigned int parent,
    unsigned int type, unsigned int mode, unsigned int flags,
    unsigned int data_word, unsigned int data_words, kword_t size_chars)
{
        np->meta = ((kword_t)(parent & HALF_MASK) << 18U) |
            ((kword_t)(type & TYPE_MASK) << TYPE_SHIFT) |
            ((kword_t)(mode & MODE_MASK) << MODE_SHIFT) |
            (kword_t)(flags & FLAGS_MASK);
        np->data = ((kword_t)(data_word & HALF_MASK) << 18U) |
            (kword_t)(data_words & HALF_MASK);
        np->size_chars = size_chars;
}

static kword_t
boot_ent(const kword_t *image, unsigned int ent, unsigned int field)
{
        return image[INITFS_HDR_WORDS + ent * INITFS_ENT_WORDS + field];
}

static unsigned int
boot_nonet(const kword_t *strings, unsigned int off)
{
        kword_t word;
        unsigned int shift;

        word = strings[off / 4U];
        shift = (3U - (off & 3U)) * 9U;
        return (unsigned int)((word >> shift) & 0777UL);
}

static int
boot_name_char(unsigned int c)
{
        if (c >= (unsigned int)'A' && c <= (unsigned int)'Z')
                return 1;
        if (c >= (unsigned int)'0' && c <= (unsigned int)'9')
                return 1;
        return c == (unsigned int)'.' || c == (unsigned int)'_' ||
            c == (unsigned int)'-' || c == (unsigned int)'$' ||
            c == (unsigned int)'%';
}

static int
boot_name(const kword_t *strings, unsigned int string_nonets,
    unsigned int off, struct vfs_name *name)
{
        unsigned int i;
        unsigned int c;
        unsigned int wi;
        unsigned int shift;

        if (off >= string_nonets)
                return -1;
        name->chars = 0U;
        for (i = 0U; i < VFS_NAME_WORDS; ++i)
                name->words[i] = 0;
        for (i = 0U; i < VFS_NAME_MAX_CHARS; ++i) {
                if (off >= string_nonets)
                        return -1;
                c = boot_nonet(strings, off++);
                if (c == 0U) {
                        name->chars = i;
                        return i == 0U ? -1 : 0;
                }
                if (!boot_name_char(c))
                        return -1;
                wi = i / 6U;
                shift = 30U - (i % 6U) * 6U;
                name->words[wi] |= ((kword_t)(c - 040U) & 077UL) << shift;
        }
        return -1;
}

static int
boot_add_dir(unsigned int slot, unsigned int parent, kword_t word,
    unsigned int chars, unsigned int mode, int writable)
{
        struct memfs_node *np;

        if (slot >= KBOOT_NODE_COUNT || parent >= KBOOT_NODE_COUNT)
                return -1;
        np = &boot_nodes[slot];
        boot_clear_node(np);
        np->name.chars = chars;
        np->name.words[0] = word;
        boot_set_node(np, parent, VFS_TYPE_DIR, mode,
            MEMFS_F_USED | (writable ? MEMFS_F_WRITABLE : 0U),
            0U, 0U, 0);
        return 0;
}

int
kfs_boot_prepare(kword_t future_free_words)
{
        const kword_t *image;
        const kword_t *strings;
        const kword_t *data;
        unsigned int image_words;
        unsigned int nent;
        unsigned int str_words;
        unsigned int data_words;
        unsigned int entries_end;
        unsigned int strings_end;
        unsigned int i;
        unsigned int parent;
        unsigned int type;
        unsigned int data_off;
        unsigned int words;
        unsigned int next;
        unsigned int mount_slot;
        unsigned int ramfs_slot;
        unsigned int temp_slot;
        unsigned int memfs_service;
        kword_t size_chars;
        struct vfs_name name;
        struct memfs_node *np;

        image = &__initfs_begin;
        image_words = (unsigned int)(&__initfs_begin_end - &__initfs_begin);
        if (image_words < INITFS_HDR_WORDS ||
            image[IHF_MAGIC] != INITFS_MAGIC ||
            image[IHF_VERSION] != INITFS_VERSION ||
            image[IHF_ENT_WORDS] != INITFS_ENT_WORDS ||
            image[IHF_FLAGS] != 0 || image[IHF_CKSUM] != 0)
                return -1;
        nent = (unsigned int)image[IHF_NENT];
        memfs_service = module_service_get(MODULE_SERVICE_MEMFS);
        if (memfs_service == 0U) {
                if (nent != 0U)
                        return -1;
                goto bind_services;
        }
        str_words = (unsigned int)image[IHF_STR_WORDS];
        data_words = (unsigned int)image[IHF_DATA_WORDS];
        if (nent + 5U > KBOOT_NODE_COUNT)
                return -1;
        entries_end = INITFS_HDR_WORDS + nent * INITFS_ENT_WORDS;
        if (entries_end > image_words || str_words > image_words - entries_end)
                return -1;
        strings_end = entries_end + str_words;
        if (data_words > image_words - strings_end)
                return -1;
        strings = image + entries_end;
        data = image + strings_end;

        for (i = 0U; i < KBOOT_NODE_COUNT; ++i)
                boot_clear_node(&boot_nodes[i]);
        boot_set_node(&boot_nodes[0], 0U, VFS_TYPE_DIR, 0555U,
            MEMFS_F_USED, 0U, 0U, 0);

        for (i = 0U; i < nent; ++i) {
                type = (unsigned int)boot_ent(image, i, IEF_TYPE);
                parent = (unsigned int)boot_ent(image, i, IEF_AUX);
                if ((type != INITFS_REG && type != INITFS_DIR) ||
                    boot_ent(image, i, IEF_FLAGS) != 0 || parent > i ||
                    boot_name(strings, str_words * 4U,
                    (unsigned int)boot_ent(image, i, IEF_NAME_OFF), &name) != 0)
                        return -1;
                data_off = (unsigned int)boot_ent(image, i, IEF_DATA_OFF);
                words = (unsigned int)boot_ent(image, i, IEF_SIZE_WORDS);
                size_chars = boot_ent(image, i, IEF_SIZE_9BYTES);
                if (type == INITFS_DIR) {
                        if (data_off != 0U || words != 0U || size_chars != 0)
                                return -1;
                } else if (data_off > data_words || words > data_words - data_off ||
                    size_chars > (kword_t)words * 4UL) {
                        return -1;
                }
                np = &boot_nodes[i + 1U];
                boot_clear_node(np);
                np->name = name;
                boot_set_node(np, parent, type == INITFS_DIR ?
                    VFS_TYPE_DIR : VFS_TYPE_REG,
                    (unsigned int)boot_ent(image, i, IEF_MODE),
                    MEMFS_F_USED | MEMFS_F_IMAGE,
                    type == INITFS_DIR ? 0U : data_off, words, size_chars);
        }

        next = nent + 1U;
        mount_slot = next;
        if (boot_add_dir(next, 0U, VFS_SIX6('M','O','U','N','T',' '),
            5U, 0555U, 0) != 0)
                return -1;
        ++next;
        ramfs_slot = next;
        if (boot_add_dir(next, next - 1U,
            VFS_SIX6('R','A','M','F','S','0'), 6U, 0777U, 1) != 0)
                return -1;
        ++next;
        temp_slot = next;
        if (boot_add_dir(next, 0U, VFS_SIX6('T','E','M','P',' ',' '),
            4U, 0555U, 0) != 0)
                return -1;
        ++next;
        if (boot_add_dir(next, mount_slot, VFS_SIX6('D','T','0',' ',' ',' '),
            3U, 0777U, 0) != 0)
                return -1;
        {
                kword_t ramfs_base;
                kword_t ramfs_words;
                kword_t spare_words;

                ramfs_words = mm_largest_free();
                if (ramfs_words > KBOOT_RAMFS0_MAX_WORDS)
                        ramfs_words = KBOOT_RAMFS0_MAX_WORDS;
                spare_words = mm_total_free();
                if (future_free_words > MM_HALF_MASK - spare_words)
                        spare_words = MM_HALF_MASK;
                else
                        spare_words += future_free_words;
                if (spare_words <= KBOOT_FIRST_USER_RESERVE_WORDS)
                        return -1;
                spare_words -= KBOOT_FIRST_USER_RESERVE_WORDS;
                if (ramfs_words > spare_words)
                        ramfs_words = spare_words;
                if (ramfs_words < KBOOT_RAMFS0_MIN_WORDS ||
                    mm_alloc(ramfs_words, MM_TYPE_KERNEL_DYNAMIC, 1U,
                    MM_ALLOC_LOW, &ramfs_base) != MM_OK)
                        return -1;
                for (i = 0U; i < KBOOT_NODE_COUNT; ++i) {
                        kword_t *dst;
                        const kword_t *srcw;
                        unsigned int j;

                        dst = (kword_t *)(unsigned long)(ramfs_base + i * 8U);
                        srcw = (const kword_t *)&boot_nodes[i];
                        for (j = 0U; j < 8U; ++j)
                                dst[j] = srcw[j];
                }
                {
                        struct memfs config;
                        struct fs_mres_request req;
                        vnode_t root;
                        vnode_t temp;
                        vnode_t ramfs;

                        config.nodes = (struct memfs_node *)(unsigned long)
                            ramfs_base;
                        config.node_count = KBOOT_NODE_COUNT;
                        config.pool = (kword_t *)(unsigned long)
                            (ramfs_base + KBOOT_NODE_WORDS);
                        config.pool_words = (unsigned int)(ramfs_words -
                            KBOOT_NODE_WORDS);
                        config.used_words = 0U;
                        config.image_data = data;
                        req.op = FS_MRES_OP_MEMFS_INIT;
                        req.a = (kword_t)(unsigned long)&config;
                        if ((int)kinit_call18_1(memfs_service,
                            (kword_t)(unsigned long)&req) != 0 ||
                            vfs_mount(VFS_NODE_NONE, MEMFS_PROVIDER,
                            MEMFS_KIND_NODE, 0U, VFS_MOUNT_RW, &root) != 0)
                                return -1;
                        temp = VFS_NODE(MEMFS_PROVIDER,
                            VFS_MOUNT_KIND(VFS_MOUNT_ID(root),
                            MEMFS_KIND_NODE), temp_slot);
                        if (vfs_mount(temp, MEMFS_PROVIDER, MEMFS_KIND_NODE,
                            ramfs_slot, VFS_MOUNT_RW, &ramfs) != 0)
                                return -1;
                        boot_memfs_root = root;
                        boot_ramfs_root = ramfs;
                        boot_ramfs_slot = ramfs_slot;
                }
        }

bind_services:
        return 0;
}

static void
boot_name6(struct vfs_name *name, kword_t word, unsigned int chars)
{
        unsigned int i;

        name->chars = chars;
        name->words[0] = word;
        for (i = 1U; i < VFS_NAME_WORDS; ++i)
                name->words[i] = 0;
}

int
kfs_boot_rebind_root(void)
{
        struct vfs_name name;
        struct vfs_stat st;
        vnode_t mount_dir;
        vnode_t ramfs_target;
        vnode_t temp_target;
        vnode_t ramfs_mount;
        vnode_t temp_mount;

        if (boot_memfs_root == VFS_NODE_NONE)
                return 0;
        boot_name6(&name, VFS_SIX6('M','O','U','N','T',' '), 5U);
        if (vfs_lookup(vfs_namespace_root, &name, &mount_dir) != 0)
                return -1;
        boot_name6(&name, VFS_SIX6('R','A','M','F','S','0'), 6U);
        if (vfs_lookup(mount_dir, &name, &ramfs_target) != 0)
                return -1;
        boot_name6(&name, VFS_SIX6('T','E','M','P',' ',' '), 4U);
        if (vfs_lookup(vfs_namespace_root, &name, &temp_target) != 0)
                return -1;
        if (vfs_stat(ramfs_target, &st) != 0 || st.type != VFS_TYPE_DIR ||
            vfs_stat(temp_target, &st) != 0 || st.type != VFS_TYPE_DIR)
                return -1;

        if (vfs_unmount(boot_ramfs_root) != 0 ||
            vfs_unmount(boot_memfs_root) != 0)
                return -1;
        boot_ramfs_root = VFS_NODE_NONE;
        boot_memfs_root = VFS_NODE_NONE;

        if (vfs_mount(ramfs_target, MEMFS_PROVIDER, MEMFS_KIND_NODE,
            boot_ramfs_slot, VFS_MOUNT_RW, &ramfs_mount) != 0)
                return -1;
        if (vfs_mount(temp_target, MEMFS_PROVIDER, MEMFS_KIND_NODE,
            boot_ramfs_slot, VFS_MOUNT_RW, &temp_mount) != 0) {
                (void)vfs_unmount(ramfs_mount);
                return -1;
        }
        return 0;
}

