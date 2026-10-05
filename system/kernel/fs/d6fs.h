#ifndef DAIMON_D6FS_H
#define DAIMON_D6FS_H

#include "kcore.h"
#include "fs_backing.h"
#include "storage.h"

#define D6FS_BLOCK_WORDS          BSTORE_BLOCK_WORDS
extern kword_t fs_block_workspace[D6FS_BLOCK_WORDS];
#define D6FS_CACHE_INVALID 0777777777777UL
#define D6FS_EXTENTS              7U
#define D6FS_FCB_WORDS            020U
#define D6FS_SUPER_WORDS          020U
#define D6FS_DIRENT_WORDS         6U
#define D6FS_LOGICAL_BLOCK_BITS   24U
#define D6FS_EXTENT_LOW_BITS      12U
#define D6FS_EXTENT_HIGH_BITS     5U
#define D6FS_EXTENT_LENGTH_BITS   17U

#define D6FS_LOGICAL_BLOCK_MASK   077777777UL
#define D6FS_EXTENT_LOW_MASK      07777UL
#define D6FS_EXTENT_HIGH_MASK     037UL
#define D6FS_EXTENT_MAX_BLOCKS    0200000UL
#define D6FS_FCB_MASK             0777777UL
#define D6FS_HASH_MASK            077777777UL

#define D6FS_TYPE_FREE            0U
#define D6FS_TYPE_REG             1U
#define D6FS_TYPE_DIR             2U
#define D6FS_TYPE_SYMLINK         3U
#define D6FS_TYPE_FIFO            4U

#define D6FS_FLAG_APPEND          0001U
#define D6FS_FLAG_NOUNLINK        0002U
#define D6FS_FLAG_NODUMP          0004U
/* ARCHIVE is persistent and manual/tool-managed; kernel mutations preserve it. */
#define D6FS_FLAG_ARCHIVE         0010U
#define D6FS_FLAG_IMMUTABLE       0020U
#define D6FS_FLAG_MASK            0777U

#define D6FS_STATE_CLEAN          0U
#define D6FS_STATE_DIRTY          1U

/* SIXBIT /D6FSV2/. */
#define D6FS_MAGIC                044066263662UL
#define D6FS_FORMAT_VERSION       2U

/* Runtime secondary-mount handoff.  A transient scanner performs full media
 * validation.  The layout mirrors struct d6fs_reader so one BLT can seed the
 * dynamic state, except word 1 is prepacked runtime opaque state with mount id
 * zero (summary block plus selected A/B copy), and word 016 is a version marker
 * which resident code replaces with trusted callbacks. */
#define D6FS_MOUNT_WORDS          021U
#define D6FS_MOUNT_MARKER         016U
#define D6FS_MOUNT_BACKING_BLOCKS 020U
#define D6FS_MOUNT_MAGIC          044066263602UL

/* FCB word numbers, octal as in the format document. */
#define D6FS_FCB_META             000U
#define D6FS_FCB_OWNER            001U
#define D6FS_FCB_SIZE             002U
#define D6FS_FCB_MTIME            003U
#define D6FS_FCB_PARENT           004U
#define D6FS_FCB_LENHIGH          005U
#define D6FS_FCB_EXTENT0          006U
#define D6FS_FCB_RESERVED0        015U

/* Superblock word numbers. */
#define D6FS_SB_MAGIC_VERSION     000U
#define D6FS_SB_SEQUENCE          001U
#define D6FS_SB_STATE             002U
#define D6FS_SB_FS_UUID0          003U
#define D6FS_SB_FS_UUID1          004U
#define D6FS_SB_SWAP_RESERVATION 005U
#define D6FS_SB_LOG_RESERVATION  006U
#define D6FS_SB_TOTAL_BLOCKS      007U
#define D6FS_SB_ROOT_FCB          010U
#define D6FS_SB_FCB_START         011U
#define D6FS_SB_FCB_COUNT         012U
#define D6FS_SB_FREEMAP_START     013U
#define D6FS_SB_FREEMAP_BLOCKS    014U
#define D6FS_SB_SUMMARY_START     015U
#define D6FS_SB_SUMMARY_BLOCKS    016U
#define D6FS_SB_RESERVATION_HIGH 017U

#define D6FS_RESERVATION_START_SHIFT 12U
#define D6FS_RESERVATION_LEN_LOW_MASK 07777UL
#define D6FS_RESERVATION_LEN_HIGH_MASK 07777UL
#define D6FS_RES_SWAP_HI_SHIFT 24U
#define D6FS_RES_LOG_HI_SHIFT 12U
#define D6FS_RESERVATION_RESERVED_MASK 07777UL

struct d6fs_fcb_info {
        unsigned int type;
        unsigned int flags;
        unsigned int mode;
        unsigned int tail;
        unsigned int extent_count;
        unsigned int uid;
        unsigned int gid;
        kword_t size_words;
        kword_t mtime;
        unsigned int parent_fcb;
};


#define D6FS_DIRENT_HASH_SHIFT      12U
#define D6FS_DIRENT_TYPE_SHIFT      9U
#define D6FS_DIRENT_TYPE_MASK       07U
#define D6FS_DIRENT_FLAGS_MASK      0777U

typedef int (*d6fs_read_block_fn)(void *opaque, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS]);
typedef int (*d6fs_write_block_fn)(void *opaque, kword_t logical,
    const kword_t block[D6FS_BLOCK_WORDS]);

struct d6fs_dirent_info {
        kword_t name[4];
        kword_t hash;
        unsigned int type;
        unsigned int flags;
        unsigned int child_fcb;
};

struct d6fs_super_info {
        kword_t sequence;
        unsigned int state;
        kword_t fs_uuid[2];
        kword_t total_blocks;
        unsigned int root_fcb;
        kword_t fcb_start;
        unsigned int fcb_count;
        kword_t freemap_start;
        kword_t freemap_blocks;
};

struct d6fs_reader {
        kword_t alloc_cursor;
        kword_t opaque;
        struct d6fs_super_info super;
        kword_t super_block[2];
        struct fs_backing backing;
};

/* super.state is boot-only.  Runtime block identity is owned by BCACHE;
 * resident D6FS deliberately keeps no second private cache tag. */

/*
 * Encode/decode one inline extent.  The run word stores START24|LENLOW12;
 * the caller stores LENHIGH5 in the shared FCB length-high word.
 */
int d6fs_extent_decode(kword_t run, unsigned int high,
    kword_t *startp, kword_t *blocksp);
unsigned int d6fs_extent_high_get(kword_t word, unsigned int extent);

/* Fixed 16-word FCB codec and structural validation. */
int d6fs_fcb_decode_valid(const kword_t fcb[D6FS_FCB_WORDS],
    kword_t fs_blocks, unsigned int fcb_count, struct d6fs_fcb_info *info);

/* Fixed 16-word dual-superblock decode and structural validation. */
int d6fs_super_decode(const kword_t sb[D6FS_SUPER_WORDS],
    struct d6fs_super_info *info);
int d6fs_super_select(const kword_t a[D6FS_SUPER_WORDS],
    const kword_t b[D6FS_SUPER_WORDS], kword_t blockset_blocks,
    struct d6fs_super_info *info, unsigned int *copyp);

/* Directory entry codec and read-only media helpers. */
int d6fs_dirent_decode_valid(const kword_t ent[D6FS_DIRENT_WORDS],
    unsigned int fcb_count, struct d6fs_dirent_info *info);
const kword_t *d6fs_reader_get_block(struct d6fs_reader *reader,
    kword_t logical);
int d6fs_reader_fcb(struct d6fs_reader *reader, unsigned int fcb_index,
    kword_t fcb[D6FS_FCB_WORDS], struct d6fs_fcb_info *info);
kword_t d6fs_file_block(const kword_t fcb[D6FS_FCB_RESERVED0],
    kword_t file_block);
int d6fs_reader_read_words(struct d6fs_reader *reader,
    const kword_t fcb[D6FS_FCB_WORDS], kword_t off, kword_t *buf,
    unsigned int nwords);

#endif


#define D6FS_BITS_PER_WORD          36U
#define D6FS_BITS_PER_MAP_BLOCK     (D6FS_BLOCK_WORDS * 36U)

kword_t d6fs_name_hash24(const kword_t words[4], unsigned int chars);
int d6fs_reader_write_block(struct d6fs_reader *reader,
    kword_t logical, const kword_t block[D6FS_BLOCK_WORDS]);
int d6fs_reader_zero_block(struct d6fs_reader *reader, kword_t logical);
int d6fs_reader_write_words(struct d6fs_reader *reader,
    const kword_t fcb[D6FS_FCB_WORDS], kword_t off,
    const kword_t *buf, unsigned int nwords);
int d6fs_reader_put_fcb(struct d6fs_reader *reader,
    unsigned int fcb_index, const kword_t fcb[D6FS_FCB_WORDS]);
int d6fs_freemap_state(struct d6fs_reader *reader, kword_t logical);
int d6fs_freemap_set(struct d6fs_reader *reader, kword_t logical,
    unsigned int allocated);
int d6fs_free_run(struct d6fs_reader *reader, kword_t start,
    kword_t blocks);
