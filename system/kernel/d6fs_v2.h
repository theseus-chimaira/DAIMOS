#ifndef DAIMON_D6FS_V2_H
#define DAIMON_D6FS_V2_H

#include "kcore.h"

#define D6FS_V2_BLOCK_WORDS          0200U
#define D6FS_V2_MAX_MEMBERS          8U
#define D6FS_V2_EXTENTS              7U
#define D6FS_V2_FCB_WORDS            020U
#define D6FS_V2_SUPER_WORDS          020U
#define D6FS_V2_DIRENT_WORDS         6U
#define D6FS_V2_LOGICAL_BLOCK_BITS   24U
#define D6FS_V2_EXTENT_LOW_BITS      12U
#define D6FS_V2_EXTENT_HIGH_BITS     5U
#define D6FS_V2_EXTENT_LENGTH_BITS   17U

#define D6FS_V2_LOGICAL_BLOCK_MASK   077777777UL
#define D6FS_V2_EXTENT_LOW_MASK      07777UL
#define D6FS_V2_EXTENT_HIGH_MASK     037UL
#define D6FS_V2_EXTENT_MAX_BLOCKS    0200000UL
#define D6FS_V2_FCB_MASK             0777777UL
#define D6FS_V2_HASH_MASK            077777777UL

#define D6FS_V2_TYPE_FREE            0U
#define D6FS_V2_TYPE_REG             1U
#define D6FS_V2_TYPE_DIR             2U
#define D6FS_V2_TYPE_SYMLINK         3U

#define D6FS_V2_FLAG_APPEND          0001U
#define D6FS_V2_FLAG_NOUNLINK        0002U
#define D6FS_V2_FLAG_NODUMP          0004U
/* ARCHIVE is persistent and manual/tool-managed; kernel mutations preserve it. */
#define D6FS_V2_FLAG_ARCHIVE         0010U
#define D6FS_V2_FLAG_IMMUTABLE       0020U
#define D6FS_V2_FLAG_MASK            0777U

#define D6FS_V2_STATE_CLEAN          0U
#define D6FS_V2_STATE_DIRTY          1U

/* SIXBIT /D6FSV2/. */
#define D6FS_V2_MAGIC                044066263662UL
#define D6FS_V2_FORMAT_VERSION       2U

/* FCB word numbers, octal as in the format document. */
#define D6FS_V2_FCB_META             000U
#define D6FS_V2_FCB_OWNER            001U
#define D6FS_V2_FCB_SIZE             002U
#define D6FS_V2_FCB_MTIME            003U
#define D6FS_V2_FCB_PARENT           004U
#define D6FS_V2_FCB_LENHIGH          005U
#define D6FS_V2_FCB_EXTENT0          006U
#define D6FS_V2_FCB_RESERVED0        015U

/* Superblock word numbers. */
#define D6FS_V2_SB_MAGIC_VERSION     000U
#define D6FS_V2_SB_SEQUENCE          001U
#define D6FS_V2_SB_STATE             002U
#define D6FS_V2_SB_FS_UUID0          003U
#define D6FS_V2_SB_FS_UUID1          004U
#define D6FS_V2_SB_DISKSET_UUID0     005U
#define D6FS_V2_SB_DISKSET_UUID1     006U
#define D6FS_V2_SB_TOTAL_BLOCKS      007U
#define D6FS_V2_SB_ROOT_FCB          010U
#define D6FS_V2_SB_FCB_START         011U
#define D6FS_V2_SB_FCB_COUNT         012U
#define D6FS_V2_SB_FREEMAP_START     013U
#define D6FS_V2_SB_FREEMAP_BLOCKS    014U
#define D6FS_V2_SB_SUMMARY_START     015U
#define D6FS_V2_SB_SUMMARY_BLOCKS    016U
#define D6FS_V2_SB_RESERVED          017U

struct d6fs_v2_diskset {
        unsigned int members;
        kword_t blocks[D6FS_V2_MAX_MEMBERS];
};

struct d6fs_v2_phys {
        unsigned int member;
        kword_t block;
};

struct d6fs_v2_fcb_info {
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


#define D6FS_V2_DIRENT_HASH_SHIFT      12U
#define D6FS_V2_DIRENT_TYPE_SHIFT      9U
#define D6FS_V2_DIRENT_TYPE_MASK       07U
#define D6FS_V2_DIRENT_FLAGS_MASK      0777U

typedef int (*d6fs_v2_read_block_fn)(void *opaque, kword_t logical,
    kword_t block[D6FS_V2_BLOCK_WORDS]);
typedef int (*d6fs_v2_write_block_fn)(void *opaque, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS]);

struct d6fs_v2_dirent_info {
        kword_t name[4];
        kword_t hash;
        unsigned int type;
        unsigned int flags;
        unsigned int child_fcb;
};

struct d6fs_v2_super_info {
        kword_t sequence;
        unsigned int state;
        kword_t fs_uuid[2];
        kword_t diskset_uuid[2];
        kword_t total_blocks;
        unsigned int root_fcb;
        kword_t fcb_start;
        unsigned int fcb_count;
        kword_t freemap_start;
        kword_t freemap_blocks;
        kword_t summary_start;
        kword_t summary_blocks;
};

struct d6fs_v2_reader {
        d6fs_v2_read_block_fn read_block;
        d6fs_v2_write_block_fn write_block;
        void *opaque;
        struct d6fs_v2_super_info super;
        kword_t cache_block;
        unsigned int cache_valid;
        kword_t physical_reads;
        kword_t physical_writes;
        kword_t cache_hits;
        kword_t cache_misses;
        kword_t cache[D6FS_V2_BLOCK_WORDS];
};

/*
 * Encode/decode one inline extent.  The run word stores START24|LENLOW12;
 * the caller stores LENHIGH5 in the shared FCB length-high word.
 */
int d6fs_v2_extent_encode(kword_t start, kword_t blocks,
    kword_t *runp, unsigned int *highp);
int d6fs_v2_extent_decode(kword_t run, unsigned int high,
    kword_t *startp, kword_t *blocksp);
unsigned int d6fs_v2_extent_high_get(kword_t word, unsigned int extent);
int d6fs_v2_extent_high_set(kword_t *wordp, unsigned int extent,
    unsigned int high);

/* Fixed 16-word FCB codec and structural validation. */
int d6fs_v2_fcb_encode(kword_t fcb[D6FS_V2_FCB_WORDS],
    const struct d6fs_v2_fcb_info *info);
int d6fs_v2_fcb_decode(const kword_t fcb[D6FS_V2_FCB_WORDS],
    struct d6fs_v2_fcb_info *info);
int d6fs_v2_fcb_valid(const kword_t fcb[D6FS_V2_FCB_WORDS],
    kword_t fs_blocks, unsigned int fcb_count);

/* Fixed 16-word dual-superblock codec and structural validation. */
int d6fs_v2_super_encode(kword_t sb[D6FS_V2_SUPER_WORDS],
    const struct d6fs_v2_super_info *info);
int d6fs_v2_super_decode(const kword_t sb[D6FS_V2_SUPER_WORDS],
    struct d6fs_v2_super_info *info);
int d6fs_v2_super_valid(const kword_t sb[D6FS_V2_SUPER_WORDS],
    kword_t diskset_blocks);
int d6fs_v2_super_select(const kword_t a[D6FS_V2_SUPER_WORDS],
    const kword_t b[D6FS_V2_SUPER_WORDS], kword_t diskset_blocks,
    struct d6fs_v2_super_info *info, unsigned int *copyp);

/*
 * Map the linear D6FS address space over unequal striped members.  blocks[]
 * contains the usable blocks on each member after physical swap tails have
 * already been removed by the diskset layer.  Members are numbered in their
 * configured stripe order.  Missing members are not representable here: a
 * diskset is either complete or unusable.
 */
int d6fs_v2_diskset_valid(const struct d6fs_v2_diskset *set);
kword_t d6fs_v2_diskset_blocks(const struct d6fs_v2_diskset *set);
int d6fs_v2_map_block(const struct d6fs_v2_diskset *set, kword_t logical,
    struct d6fs_v2_phys *phys);

/* Directory entry codec and read-only media helpers. */
int d6fs_v2_dirent_decode(const kword_t ent[D6FS_V2_DIRENT_WORDS],
    struct d6fs_v2_dirent_info *info);
int d6fs_v2_dirent_valid(const kword_t ent[D6FS_V2_DIRENT_WORDS],
    unsigned int fcb_count);
int d6fs_v2_reader_init(struct d6fs_v2_reader *reader,
    d6fs_v2_read_block_fn read_block, void *opaque,
    const struct d6fs_v2_super_info *super);
int d6fs_v2_reader_get_block(struct d6fs_v2_reader *reader, kword_t logical,
    const kword_t **blockp);
int d6fs_v2_reader_fcb(struct d6fs_v2_reader *reader, unsigned int fcb_index,
    kword_t fcb[D6FS_V2_FCB_WORDS]);
int d6fs_v2_file_block(const kword_t fcb[D6FS_V2_FCB_WORDS],
    kword_t file_block, kword_t *logical_block);
int d6fs_v2_reader_read_words(struct d6fs_v2_reader *reader,
    const kword_t fcb[D6FS_V2_FCB_WORDS], kword_t off, kword_t *buf,
    unsigned int nwords);

#endif


#define D6FS_V2_BITS_PER_WORD          36U
#define D6FS_V2_BITS_PER_MAP_BLOCK     (D6FS_V2_BLOCK_WORDS * 36U)

kword_t d6fs_v2_name_hash24(const kword_t words[4], unsigned int chars);
int d6fs_v2_dirent_encode(kword_t ent[D6FS_V2_DIRENT_WORDS],
    const struct d6fs_v2_dirent_info *info);
int d6fs_v2_reader_set_writer(struct d6fs_v2_reader *reader,
    d6fs_v2_write_block_fn write_block);
int d6fs_v2_reader_write_block(struct d6fs_v2_reader *reader,
    kword_t logical, const kword_t block[D6FS_V2_BLOCK_WORDS]);
int d6fs_v2_reader_zero_block(struct d6fs_v2_reader *reader, kword_t logical);
int d6fs_v2_reader_write_words(struct d6fs_v2_reader *reader,
    const kword_t fcb[D6FS_V2_FCB_WORDS], kword_t off,
    const kword_t *buf, unsigned int nwords);
int d6fs_v2_reader_put_fcb(struct d6fs_v2_reader *reader,
    unsigned int fcb_index, const kword_t fcb[D6FS_V2_FCB_WORDS]);
int d6fs_v2_freemap_test(struct d6fs_v2_reader *reader, kword_t logical,
    unsigned int *allocatedp);
int d6fs_v2_freemap_set(struct d6fs_v2_reader *reader, kword_t logical,
    unsigned int allocated);
int d6fs_v2_alloc_run(struct d6fs_v2_reader *reader, kword_t cursor,
    kword_t max_blocks, kword_t *startp, kword_t *blocksp);
int d6fs_v2_free_run(struct d6fs_v2_reader *reader, kword_t start,
    kword_t blocks);
