/* mkauxstore.c - format/inspect DAIMOS AUXSTORE media descriptors. */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_MASK              0777777777777ULL
#define HALF_MASK              0777777U
#define BLOCK_WORDS            0200U
#define WORD_BYTES             8U
#define BLOCK_BYTES            (BLOCK_WORDS * WORD_BYTES)

#define AUXSTORE_MAGIC         0416570636421ULL /* SIXBIT /AUXST1/ */
#define AUXSTORE_VERSION       1U
#define AUXSTORE_DESC_MAGIC    0U
#define AUXSTORE_DESC_VERSION  1U
#define AUXSTORE_DESC_BACK     2U
#define AUXSTORE_DESC_LOG      3U
#define AUXSTORE_DESC_CACHE    4U
#define AUXSTORE_DESC_D6FS     5U
#define AUXSTORE_DESC_D6SUPER  6U

static void
die(const char *msg)
{
        fprintf(stderr, "mkauxstore: %s\n", msg);
        exit(1);
}

static void
die_path(const char *path)
{
        fprintf(stderr, "mkauxstore: %s: %s\n", path, strerror(errno));
        exit(1);
}

static unsigned long
parse_number(const char *s)
{
        char *end;
        unsigned long v;

        errno = 0;
        v = strtoul(s, &end, 0);
        if (errno != 0 || end == s || *end != '\0' || v > HALF_MASK)
                die("invalid numeric argument");
        return v;
}

static uint64_t
pair(unsigned start, unsigned blocks)
{
        return (((uint64_t)start & HALF_MASK) << 18) |
            ((uint64_t)blocks & HALF_MASK);
}

static void
put64le(FILE *fp, uint64_t v)
{
        unsigned char raw[8];
        unsigned i;

        v &= WORD_MASK;
        for (i = 0U; i < 8U; ++i)
                raw[i] = (unsigned char)(v >> (i * 8U));
        if (fwrite(raw, 1U, sizeof(raw), fp) != sizeof(raw))
                die("cannot write image");
}

static uint64_t
get64le(FILE *fp)
{
        unsigned char raw[8];
        uint64_t v;
        unsigned i;

        if (fread(raw, 1U, sizeof(raw), fp) != sizeof(raw))
                die("cannot read image");
        v = 0U;
        for (i = 0U; i < 8U; ++i)
                v |= (uint64_t)raw[i] << (i * 8U);
        return v & WORD_MASK;
}

static void
write_block(FILE *fp, unsigned block, const uint64_t words[BLOCK_WORDS])
{
        unsigned i;

        if (fseek(fp, (long)block * (long)BLOCK_BYTES, SEEK_SET) != 0)
                die("cannot seek image");
        for (i = 0U; i < BLOCK_WORDS; ++i)
                put64le(fp, words[i]);
}

static unsigned
image_blocks(FILE *fp)
{
        long bytes;

        if (fseek(fp, 0L, SEEK_END) != 0)
                die("cannot seek image");
        bytes = ftell(fp);
        if (bytes <= 0 || ((unsigned long)bytes % BLOCK_BYTES) != 0U)
                die("image size is not a whole DAIMOS block count");
        if ((unsigned long)bytes / BLOCK_BYTES > HALF_MASK)
                die("image is too large for AUXSTORE descriptor ranges");
        return (unsigned)((unsigned long)bytes / BLOCK_BYTES);
}

static void
make_blank(const char *path, unsigned blocks)
{
        FILE *fp;

        if (blocks == 0U)
                die("blank image must contain at least one block");
        fp = fopen(path, "wb");
        if (fp == NULL)
                die_path(path);
        if (fseek(fp, (long)blocks * (long)BLOCK_BYTES - 1L, SEEK_SET) != 0 ||
            fputc(0, fp) == EOF || fclose(fp) != 0)
                die_path(path);
}

static void
zero_range(FILE *fp, unsigned start, unsigned blocks)
{
        uint64_t zero[BLOCK_WORDS];
        unsigned i;

        memset(zero, 0, sizeof(zero));
        for (i = 0U; i < blocks; ++i)
                write_block(fp, start + i, zero);
}

static void
format_auxstore(const char *path, unsigned back_blocks, unsigned log_blocks,
    unsigned cache_blocks, unsigned d6fs_base, unsigned d6fs_blocks,
    unsigned d6fs_super_a, unsigned d6fs_super_b)
{
        FILE *fp;
        uint64_t desc[BLOCK_WORDS];
        unsigned total, log_start, back_start, cache_start, aux_start;

        fp = fopen(path, "rb+");
        if (fp == NULL)
                die_path(path);
        total = image_blocks(fp);
        if (d6fs_base == 0U || d6fs_blocks == 0U ||
            d6fs_base >= total || d6fs_blocks > total - d6fs_base)
                die("invalid D6FS region");
        if (d6fs_super_a >= d6fs_blocks || d6fs_super_b >= d6fs_blocks ||
            d6fs_super_a == d6fs_super_b)
                die("invalid D6FS superblock locations");
        aux_start = d6fs_base + d6fs_blocks;
        if (log_blocks > total - aux_start)
                die("LOGSTORE region exceeds image");
        log_start = aux_start;
        back_start = log_start + log_blocks;
        if (back_blocks > total - back_start)
                die("BACKSTORE region exceeds image");
        cache_start = back_start + back_blocks;
        if (cache_blocks > total - cache_start ||
            cache_start + cache_blocks != total)
                die("AUXSTORE regions must exactly fill the image tail");

        zero_range(fp, log_start, log_blocks);
        zero_range(fp, back_start, back_blocks);
        zero_range(fp, cache_start, cache_blocks);
        memset(desc, 0, sizeof(desc));
        desc[AUXSTORE_DESC_MAGIC] = AUXSTORE_MAGIC;
        desc[AUXSTORE_DESC_VERSION] = AUXSTORE_VERSION;
        desc[AUXSTORE_DESC_BACK] = pair(back_start, back_blocks);
        desc[AUXSTORE_DESC_LOG] = pair(log_start, log_blocks);
        desc[AUXSTORE_DESC_CACHE] = pair(cache_start, cache_blocks);
        desc[AUXSTORE_DESC_D6FS] = pair(d6fs_base, d6fs_blocks);
        desc[AUXSTORE_DESC_D6SUPER] = pair(d6fs_super_a, d6fs_super_b);
        write_block(fp, 0U, desc);
        if (fclose(fp) != 0)
                die_path(path);

        printf("mkauxstore: %s: D6FS %o+%o, LOGSTORE %o+%o, "
            "BACKSTORE %o+%o, CACHESTORE %o+%o\n", path,
            d6fs_base, d6fs_blocks, log_start, log_blocks,
            back_start, back_blocks, cache_start, cache_blocks);
}

static void
inspect_auxstore(const char *path)
{
        FILE *fp;
        uint64_t desc[BLOCK_WORDS];
        unsigned i;

        fp = fopen(path, "rb");
        if (fp == NULL)
                die_path(path);
        (void)image_blocks(fp);
        if (fseek(fp, 0L, SEEK_SET) != 0)
                die("cannot seek image");
        for (i = 0U; i < BLOCK_WORDS; ++i)
                desc[i] = get64le(fp);
        if (fclose(fp) != 0)
                die_path(path);
        if (desc[AUXSTORE_DESC_MAGIC] != AUXSTORE_MAGIC ||
            desc[AUXSTORE_DESC_VERSION] != AUXSTORE_VERSION)
                die("AUXSTORE descriptor not found");
        printf("BACKSTORE %o+%o\n",
            (unsigned)(desc[AUXSTORE_DESC_BACK] >> 18),
            (unsigned)(desc[AUXSTORE_DESC_BACK] & HALF_MASK));
        printf("LOGSTORE %o+%o\n",
            (unsigned)(desc[AUXSTORE_DESC_LOG] >> 18),
            (unsigned)(desc[AUXSTORE_DESC_LOG] & HALF_MASK));
        printf("CACHESTORE %o+%o\n",
            (unsigned)(desc[AUXSTORE_DESC_CACHE] >> 18),
            (unsigned)(desc[AUXSTORE_DESC_CACHE] & HALF_MASK));
        printf("D6FS %o+%o SUPER %o/%o\n",
            (unsigned)(desc[AUXSTORE_DESC_D6FS] >> 18),
            (unsigned)(desc[AUXSTORE_DESC_D6FS] & HALF_MASK),
            (unsigned)(desc[AUXSTORE_DESC_D6SUPER] >> 18),
            (unsigned)(desc[AUXSTORE_DESC_D6SUPER] & HALF_MASK));
}

static void
usage(void)
{
        fprintf(stderr,
            "usage: mkauxstore --blank IMAGE --blocks N\n"
            "       mkauxstore --inspect IMAGE\n"
            "       mkauxstore -i IMAGE --backstore-blocks N "
            "--logstore-blocks N --cache-blocks N --d6fs-base N "
            "--d6fs-blocks N --d6fs-super-a N --d6fs-super-b N\n");
        exit(2);
}

int
main(int argc, char **argv)
{
        const char *image, *blank, *inspect;
        unsigned blocks, back_blocks, log_blocks, cache_blocks;
        unsigned d6fs_base, d6fs_blocks, d6fs_super_a, d6fs_super_b;
        int i;

        image = NULL;
        blank = NULL;
        inspect = NULL;
        blocks = back_blocks = log_blocks = cache_blocks = 0U;
        d6fs_base = d6fs_blocks = 0U;
        d6fs_super_a = d6fs_super_b = HALF_MASK + 1U;
        for (i = 1; i < argc; ++i) {
                if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
                        image = argv[++i];
                else if (strcmp(argv[i], "--blank") == 0 && i + 1 < argc)
                        blank = argv[++i];
                else if (strcmp(argv[i], "--inspect") == 0 && i + 1 < argc)
                        inspect = argv[++i];
                else if (strcmp(argv[i], "--blocks") == 0 && i + 1 < argc)
                        blocks = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--backstore-blocks") == 0 && i + 1 < argc)
                        back_blocks = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--logstore-blocks") == 0 && i + 1 < argc)
                        log_blocks = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--cache-blocks") == 0 && i + 1 < argc)
                        cache_blocks = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--d6fs-base") == 0 && i + 1 < argc)
                        d6fs_base = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--d6fs-blocks") == 0 && i + 1 < argc)
                        d6fs_blocks = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--d6fs-super-a") == 0 && i + 1 < argc)
                        d6fs_super_a = (unsigned)parse_number(argv[++i]);
                else if (strcmp(argv[i], "--d6fs-super-b") == 0 && i + 1 < argc)
                        d6fs_super_b = (unsigned)parse_number(argv[++i]);
                else
                        usage();
        }
        if (blank != NULL) {
                if (image != NULL || inspect != NULL || blocks == 0U || argc != 5)
                        usage();
                make_blank(blank, blocks);
                return 0;
        }
        if (inspect != NULL) {
                if (image != NULL || blocks != 0U || argc != 3)
                        usage();
                inspect_auxstore(inspect);
                return 0;
        }
        if (image == NULL || blocks != 0U || d6fs_base == 0U ||
            d6fs_blocks == 0U || d6fs_super_a > HALF_MASK ||
            d6fs_super_b > HALF_MASK)
                usage();
        format_auxstore(image, back_blocks, log_blocks, cache_blocks,
            d6fs_base, d6fs_blocks, d6fs_super_a, d6fs_super_b);
        return 0;
}
