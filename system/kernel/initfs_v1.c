#include "initfs_v1.h"

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

static kword_t
initfs_v1_ent(const kword_t *image, unsigned int ent, unsigned int field)
{
        return image[INITFS_V1_HDR_WORDS + ent * INITFS_V1_ENT_WORDS + field];
}

static unsigned int
initfs_v1_nonet(const kword_t *strings, unsigned int off)
{
        kword_t word;
        unsigned int shift;

        word = strings[off / 4U];
        shift = (3U - (off & 3U)) * 9U;
        return (unsigned int)((word >> shift) & 0777UL);
}

static int
initfs_v1_name_char(unsigned int c)
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
initfs_v1_name(const kword_t *strings, unsigned int string_nonets,
    unsigned int off, struct vfs_v1_name *name)
{
        unsigned int i;
        unsigned int c;
        unsigned int wi;
        unsigned int shift;

        if (name == 0 || off >= string_nonets)
                return -1;
        name->chars = 0U;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                name->words[i] = 0;
        for (i = 0U; i < VFS_V1_NAME_MAX_CHARS; ++i) {
                if (off >= string_nonets)
                        return -1;
                c = initfs_v1_nonet(strings, off++);
                if (c == 0U) {
                        name->chars = i;
                        return i == 0U ? -1 : 0;
                }
                if (!initfs_v1_name_char(c))
                        return -1;
                wi = i / 6U;
                shift = 30U - (i % 6U) * 6U;
                name->words[wi] |= ((kword_t)(c - 040U) & 077UL) << shift;
        }
        return -1;
}

int
initfs_v1_mount(struct memfs_v1 *fs, struct memfs_v1_node *nodes,
    unsigned int node_count, const kword_t *image, unsigned int image_words)
{
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
        kword_t size_chars;
        struct vfs_v1_name name;
        const kword_t *strings;
        const kword_t *data;

        if (fs == 0 || nodes == 0 || image == 0 ||
            image_words < INITFS_V1_HDR_WORDS ||
            image[IHF_MAGIC] != INITFS_V1_MAGIC ||
            image[IHF_VERSION] != INITFS_V1_VERSION ||
            image[IHF_ENT_WORDS] != INITFS_V1_ENT_WORDS ||
            image[IHF_FLAGS] != 0 || image[IHF_CKSUM] != 0)
                return -1;
        nent = (unsigned int)image[IHF_NENT];
        str_words = (unsigned int)image[IHF_STR_WORDS];
        data_words = (unsigned int)image[IHF_DATA_WORDS];
        if (nent + 1U > node_count ||
            nent > ((~0U) - INITFS_V1_HDR_WORDS) / INITFS_V1_ENT_WORDS)
                return -1;
        entries_end = INITFS_V1_HDR_WORDS + nent * INITFS_V1_ENT_WORDS;
        if (entries_end > image_words || str_words > image_words - entries_end)
                return -1;
        strings_end = entries_end + str_words;
        if (data_words > image_words - strings_end)
                return -1;
        strings = image + entries_end;
        data = image + strings_end;
        if (memfs_v1_init(fs, nodes, node_count, 0, 0U, 0) != 0)
                return -1;
        fs->image_data = data;
        for (i = 0U; i < nent; ++i) {
                type = (unsigned int)initfs_v1_ent(image, i, IEF_TYPE);
                parent = (unsigned int)initfs_v1_ent(image, i, IEF_AUX);
                if ((type != INITFS_V1_REG && type != INITFS_V1_DIR) ||
                    initfs_v1_ent(image, i, IEF_FLAGS) != 0 || parent > i ||
                    initfs_v1_name(strings, str_words * 4U,
                    (unsigned int)initfs_v1_ent(image, i, IEF_NAME_OFF),
                    &name) != 0)
                        return -1;
                data_off = (unsigned int)initfs_v1_ent(image, i, IEF_DATA_OFF);
                words = (unsigned int)initfs_v1_ent(image, i, IEF_SIZE_WORDS);
                size_chars = initfs_v1_ent(image, i, IEF_SIZE_9BYTES);
                if (type == INITFS_V1_DIR) {
                        if (data_off != 0U || words != 0U || size_chars != 0)
                                return -1;
                } else if (data_off > data_words ||
                    words > data_words - data_off ||
                    size_chars > (kword_t)words * 4UL) {
                        return -1;
                }
                if (memfs_v1_import_node(fs, i + 1U, parent, &name,
                    type == INITFS_V1_DIR ? VFS_V1_TYPE_DIR : VFS_V1_TYPE_REG,
                    (unsigned int)initfs_v1_ent(image, i, IEF_MODE),
                    type == INITFS_V1_DIR ? 0U : data_off, words,
                    size_chars) != 0)
                        return -1;
        }
        return 0;
}
