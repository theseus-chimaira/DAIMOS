#include "kinit.h"
#include "kboot_v1.h"
#include "initfs_v1.h"
#include "exec_v1.h"
#include "proc_v1.h"
#include "file_v1.h"
#include "devicefs_v1.h"
#include "dtfs_v1.h"
#include "dsk270.h"
#include "module.h"
#include "syscall_v1.h"
#include "fs_mres.h"


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

extern kword_t __initfs_v1_begin;
extern kword_t __initfs_v1_begin_end;

static struct memfs_v1_node boot_nodes_v2[KBOOT_V1_NODE_COUNT];

static void
boot_clear_node(struct memfs_v1_node *np)
{
        unsigned int i;

        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                np->name.words[i] = 0;
        np->name.chars = 0U;
        np->meta = 0;
        np->size_chars = 0;
        np->data = 0;
}

static void
boot_set_node(struct memfs_v1_node *np, unsigned int parent,
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
        return image[INITFS_V1_HDR_WORDS + ent * INITFS_V1_ENT_WORDS + field];
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
    unsigned int off, struct vfs_v1_name *name)
{
        unsigned int i;
        unsigned int c;
        unsigned int wi;
        unsigned int shift;

        if (off >= string_nonets)
                return -1;
        name->chars = 0U;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                name->words[i] = 0;
        for (i = 0U; i < VFS_V1_NAME_MAX_CHARS; ++i) {
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
        struct memfs_v1_node *np;

        if (slot >= KBOOT_V1_NODE_COUNT || parent >= KBOOT_V1_NODE_COUNT)
                return -1;
        np = &boot_nodes_v2[slot];
        boot_clear_node(np);
        np->name.chars = chars;
        np->name.words[0] = word;
        boot_set_node(np, parent, VFS_V1_TYPE_DIR, mode,
            MEMFS_V1_F_USED | (writable ? MEMFS_V1_F_WRITABLE : 0U),
            0U, 0U, 0);
        return 0;
}

int
kfs_boot_v2_prepare(void)
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
        kword_t size_chars;
        struct vfs_v1_name name;
        struct memfs_v1_node *np;

        image = &__initfs_v1_begin;
        image_words = (unsigned int)(&__initfs_v1_begin_end - &__initfs_v1_begin);
        if (image_words < INITFS_V1_HDR_WORDS ||
            image[IHF_MAGIC] != INITFS_V1_MAGIC ||
            image[IHF_VERSION] != INITFS_V1_VERSION ||
            image[IHF_ENT_WORDS] != INITFS_V1_ENT_WORDS ||
            image[IHF_FLAGS] != 0 || image[IHF_CKSUM] != 0)
                return -1;
        nent = (unsigned int)image[IHF_NENT];
        fs_memfs_service_addr = module_service_get(MODULE_SERVICE_MEMFS);
        fs_dtfs_service_addr = module_service_get(MODULE_SERVICE_DTFS);
        fs_d6fs_service_addr = module_service_get(MODULE_SERVICE_D6FS);
        if (fs_memfs_service_addr == 0U) {
                if (nent != 0U)
                        return -1;
                goto bind_services;
        }
        str_words = (unsigned int)image[IHF_STR_WORDS];
        data_words = (unsigned int)image[IHF_DATA_WORDS];
        if (nent + 4U > KBOOT_V1_NODE_COUNT)
                return -1;
        entries_end = INITFS_V1_HDR_WORDS + nent * INITFS_V1_ENT_WORDS;
        if (entries_end > image_words || str_words > image_words - entries_end)
                return -1;
        strings_end = entries_end + str_words;
        if (data_words > image_words - strings_end)
                return -1;
        strings = image + entries_end;
        data = image + strings_end;

        for (i = 0U; i < KBOOT_V1_NODE_COUNT; ++i)
                boot_clear_node(&boot_nodes_v2[i]);
        boot_set_node(&boot_nodes_v2[0], 0U, VFS_V1_TYPE_DIR, 0555U,
            MEMFS_V1_F_USED, 0U, 0U, 0);

        for (i = 0U; i < nent; ++i) {
                type = (unsigned int)boot_ent(image, i, IEF_TYPE);
                parent = (unsigned int)boot_ent(image, i, IEF_AUX);
                if ((type != INITFS_V1_REG && type != INITFS_V1_DIR) ||
                    boot_ent(image, i, IEF_FLAGS) != 0 || parent > i ||
                    boot_name(strings, str_words * 4U,
                    (unsigned int)boot_ent(image, i, IEF_NAME_OFF), &name) != 0)
                        return -1;
                data_off = (unsigned int)boot_ent(image, i, IEF_DATA_OFF);
                words = (unsigned int)boot_ent(image, i, IEF_SIZE_WORDS);
                size_chars = boot_ent(image, i, IEF_SIZE_9BYTES);
                if (type == INITFS_V1_DIR) {
                        if (data_off != 0U || words != 0U || size_chars != 0)
                                return -1;
                } else if (data_off > data_words || words > data_words - data_off ||
                    size_chars > (kword_t)words * 4UL) {
                        return -1;
                }
                np = &boot_nodes_v2[i + 1U];
                boot_clear_node(np);
                np->name = name;
                boot_set_node(np, parent, type == INITFS_V1_DIR ?
                    VFS_V1_TYPE_DIR : VFS_V1_TYPE_REG,
                    (unsigned int)boot_ent(image, i, IEF_MODE),
                    MEMFS_V1_F_USED | MEMFS_V1_F_IMAGE,
                    type == INITFS_V1_DIR ? 0U : data_off, words, size_chars);
        }

        next = nent + 1U;
        if (boot_add_dir(next, 0U, VFS_V1_SIX6('M','O','U','N','T',' '),
            5U, 0555U, 0) != 0)
                return -1;
        ++next;
        if (boot_add_dir(next, next - 1U,
            VFS_V1_SIX6('R','A','M','F','S','0'), 6U, 0777U, 1) != 0)
                return -1;
        file_v1_alias_node = VFS_V1_NODE(MEMFS_V1_PROVIDER,
            MEMFS_V1_KIND_NODE, next);
        ++next;
        if (boot_add_dir(next, 0U, VFS_V1_SIX6('D','T','0',' ',' ',' '),
            3U, 0777U, 0) != 0)
                return -1;
        for (i = 0U; i < KBOOT_V1_NODE_COUNT; ++i) {
                kword_t *dst;
                const kword_t *srcw;
                unsigned int j;

                dst = (kword_t *)(unsigned long)(KBOOT_V1_RAMFS0_BASE +
                    i * 8U);
                srcw = (const kword_t *)&boot_nodes_v2[i];
                for (j = 0U; j < 8U; ++j)
                        dst[j] = srcw[j];
        }
        {
                kword_t *state;
                unsigned int j;

                state = (kword_t *)(unsigned long)
                    (KBOOT_V1_RAMFS0_BASE + KBOOT_V1_NODE_WORDS);
                for (j = 0U; j < KBOOT_V1_RUNTIME_STATE_WORDS; ++j)
                        state[j] = 0;
        }
        {
                struct memfs_v1 config;
                struct fs_mres_request req;
                vnode_v1_t root;

                config.nodes = (struct memfs_v1_node *)(unsigned long)
                    KBOOT_V1_RAMFS0_BASE;
                config.node_count = KBOOT_V1_NODE_COUNT;
                config.pool = (kword_t *)(unsigned long)
                    (KBOOT_V1_RAMFS0_BASE + KBOOT_V1_NODE_WORDS +
                    KBOOT_V1_RUNTIME_STATE_WORDS);
                config.pool_words = KBOOT_V1_RAMFS0_WORDS -
                    KBOOT_V1_NODE_WORDS - KBOOT_V1_RUNTIME_STATE_WORDS;
                config.used_words = 0U;
                config.writable = 1;
                config.image_data = data;
                req.op = FS_MRES_OP_MEMFS_INIT;
                req.a = (kword_t)(unsigned long)&config;
                if (fs_mres_call(fs_memfs_service_addr, &req) != 0 ||
                    vfs_v1_mount(VFS_V1_NODE_NONE, MEMFS_V1_PROVIDER,
                    MEMFS_V1_KIND_NODE, 0U, VFS_V1_MOUNT_RW, &root) != 0)
                        return -1;
        }

bind_services:
        devicefs_v1_present = kcore_cty_putchar_v1 != 0 &&
            kcore_cty_getchar_v1 != 0 ?
            DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0) : 0;
        if (module_service_get(MODULE_SERVICE_DTC_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DTC_WRITE_BLOCK) != 0U)
                devicefs_v1_present |=
                    DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_DTC0);

        dsk270_read_addr_v1 =
            module_service_get(MODULE_SERVICE_DSK_READ_SECTOR);
        dsk270_write_addr_v1 =
            module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR);
        return 0;
}

