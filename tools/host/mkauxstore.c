/* mkauxstore.c - format/inspect DAIMOS AUXSTORE media descriptors. */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

/* Exact one-member clean DBOOT descriptor produced by mkdsk before
 * conversion to the AUXST1 partition descriptor. */
#define DBOOT_CLEAN_HEADER      0444243000000ULL
#define DBOOT_SINGLE_MEMBER     04010000ULL
#define DBOOT_D6FS_MAGIC       0442654636222ULL

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
        if ((v & ~WORD_MASK) != 0U)
                die("image word has nonzero bits above bit 35");
        return v;
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

/* The host formatter operates on an image file, never a live block device.
 * A caller must explicitly choose and provision the actual controller/unit. */
static void
require_regular_image(FILE *fp, const char *path)
{
        struct stat st;
        struct stat lst;

        if (fstat(fileno(fp), &st) != 0 || lstat(path, &lst) != 0 ||
            !S_ISREG(st.st_mode) || !S_ISREG(lst.st_mode))
                die("refusing non-regular or symlinked image");
}

static void
make_blank(const char *path, unsigned blocks)
{
        FILE *fp;
        int fd;

        if (blocks == 0U)
                die("blank image must contain at least one block");
        fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
        if (fd < 0)
                die_path(path);
        fp = fdopen(fd, "wb");
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

/* Reject existing media contents before any destructive write.  This also
 * protects unknown formats; interpreting their superblocks is unnecessary. */
static void
require_empty(FILE *fp, unsigned start, unsigned blocks)
{
        unsigned char raw[BLOCK_BYTES];
        unsigned i, j;
        if (fseek(fp, (long)start * (long)BLOCK_BYTES, SEEK_SET) != 0)
                die("cannot seek image for safety scan");
        for (i = 0U; i < blocks; ++i) {
                if (fread(raw, 1U, sizeof(raw), fp) != sizeof(raw))
                        die("cannot read image for safety scan");
                for (j = 0U; j < sizeof(raw); ++j)
                        if (raw[j] != 0U)
                                die("refusing nonempty descriptor or backing region");
        }
}

/* Convert only a known disposable DBOOT descriptor. Never permit a generic
 * overwrite of block zero: exact layout matching is essential because the
 * existing D6FS partition must survive the conversion unchanged. */
static void
require_dboot_conversion(FILE *fp, unsigned d6fs_base,
    unsigned d6fs_blocks, unsigned super_a, unsigned super_b,
    unsigned tail_blocks)
{
        uint64_t w[BLOCK_WORDS];
        unsigned i;

        if (fseek(fp, 0L, SEEK_SET) != 0)
                die("cannot seek DBOOT descriptor");
        for (i = 0U; i < BLOCK_WORDS; ++i)
                w[i] = get64le(fp);
        if (w[0] != DBOOT_CLEAN_HEADER ||
            w[5] != DBOOT_SINGLE_MEMBER ||
            w[6] != DBOOT_D6FS_MAGIC ||
            w[7] != pair(d6fs_base, d6fs_blocks) ||
            w[8] != super_a || w[9] != super_b ||
            w[10] != tail_blocks)
                die("DBOOT conversion metadata does not match requested layout");
        for (i = 1U; i < BLOCK_WORDS; ++i) {
                if (i >= 5U && i <= 10U)
                        continue;
                if (w[i] != 0U)
                        die("DBOOT descriptor contains unexpected metadata");
        }
}

static void
format_auxstore(const char *path, unsigned back_blocks, unsigned log_blocks,
    unsigned cache_blocks, unsigned d6fs_base, unsigned d6fs_blocks,
    unsigned d6fs_super_a, unsigned d6fs_super_b, int dry_run,
    int confirmed, int convert_dboot)
{
        FILE *fp;
        uint64_t desc[BLOCK_WORDS];
        unsigned total, log_start, back_start, cache_start, aux_start;

        fp = fopen(path, dry_run ? "rb" : "rb+");
        if (fp == NULL)
                die_path(path);
        require_regular_image(fp, path);
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

        printf("mkauxstore: %s: D6FS %o+%o, LOGSTORE %o+%o, "
            "BACKSTORE %o+%o, CACHESTORE %o+%o\n", path,
            d6fs_base, d6fs_blocks, log_start, log_blocks,
            back_start, back_blocks, cache_start, cache_blocks);
        if (!dry_run && !confirmed)
                die("format requires --confirm-destructive (use --dry-run first)");
        if (convert_dboot)
                require_dboot_conversion(fp, d6fs_base, d6fs_blocks,
                    d6fs_super_a, d6fs_super_b, total - aux_start);
        else
                require_empty(fp, 0U, 1U);
        require_empty(fp, aux_start, total - aux_start);
        if (dry_run) {
                if (fclose(fp) != 0)
                        die_path(path);
                return;
        }

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
        require_regular_image(fp, path);
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
            "--d6fs-blocks N --d6fs-super-a N --d6fs-super-b N "
            "[--dry-run | --confirm-destructive] [--convert-dboot]\n");
        exit(2);
}

int
main(int argc, char **argv)
{
        const char *image, *blank, *inspect;
        unsigned blocks, back_blocks, log_blocks, cache_blocks;
        unsigned d6fs_base, d6fs_blocks, d6fs_super_a, d6fs_super_b;
        int i, dry_run, confirmed, convert_dboot;

        image = NULL;
        blank = NULL;
        inspect = NULL;
        dry_run = confirmed = 0;
        convert_dboot = 0;
        blocks = back_blocks = log_blocks = cache_blocks = 0U;
        d6fs_base = d6fs_blocks = 0U;
        d6fs_super_a = d6fs_super_b = HALF_MASK + 1U;
        for (i = 1; i < argc; ++i) {
                if (strcmp(argv[i], "--dry-run") == 0)
                        dry_run = 1;
                else if (strcmp(argv[i], "--confirm-destructive") == 0)
                        confirmed = 1;
                else if (strcmp(argv[i], "--convert-dboot") == 0)
                        convert_dboot = 1;
                else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
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
        if (dry_run && confirmed)
                usage();
        if (blank != NULL) {
                if (dry_run || confirmed || convert_dboot)
                        usage();
                if (image != NULL || inspect != NULL || blocks == 0U || argc != 5)
                        usage();
                make_blank(blank, blocks);
                return 0;
        }
        if (inspect != NULL) {
                if (dry_run || confirmed || convert_dboot)
                        usage();
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
            d6fs_base, d6fs_blocks, d6fs_super_a, d6fs_super_b,
            dry_run, confirmed, convert_dboot);
        return 0;
}
