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
        /* A standalone MKSWAP medium has no D6FS partition.  The kernel
         * AUXST1 validator explicitly accepts zero D6FS and super fields.
         * Keep the existing co-resident D6FS formatting rules unchanged. */
        if (d6fs_blocks == 0U) {
                if (d6fs_base != 0U || d6fs_super_a != 0U ||
                    d6fs_super_b != 0U)
                        die("invalid absent D6FS region");
                aux_start = 1U;
        } else {
                if (d6fs_base == 0U || d6fs_base >= total ||
                    d6fs_blocks > total - d6fs_base)
                        die("invalid D6FS region");
                if (d6fs_super_a >= d6fs_blocks ||
                    d6fs_super_b >= d6fs_blocks ||
                    d6fs_super_a == d6fs_super_b)
                        die("invalid D6FS superblock locations");
                aux_start = d6fs_base + d6fs_blocks;
        }
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

/* MKSWAP shares AUXST1's formatter and safety scans.  This deliberately
 * supports only offline images of the physical unit-zero geometries that
 * KINIT presently discovers.  Higher units require a future explicit
 * boot selector; pretending to provision them would be misleading. */
static void
mkswap_usage(void)
{
        fprintf(stderr,
            "usage: mkswap --controller drm|dsk --unit 0 --image IMAGE [--inspect]\n"
            "       mkswap --controller drm|dsk --unit 0 --image IMAGE --format\n"
            "              --backstore-blocks N [--logstore-blocks N]\n"
            "              [--cache-blocks N] [--dry-run | --confirm-destructive]\n"
            "       All numbers are C-style (leading 0 means octal).\n"
            "       The descriptor and regions must fill the entire offline image.\n");
        exit(2);
}

static void
mkswap_check_image(const char *image, unsigned expected)
{
        FILE *fp;
        unsigned actual;

        fp = fopen(image, "rb");
        if (fp == NULL)
                die_path(image);
        require_regular_image(fp, image);
        actual = image_blocks(fp);
        if (fclose(fp) != 0)
                die_path(image);
        if (actual != expected)
                die("image size does not match selected controller/unit geometry");
}

static void
mkswap_inspect(const char *path, unsigned total)
{
        FILE *fp;
        uint64_t desc[BLOCK_WORDS];
        unsigned s[4], n[4], i, j;

        fp = fopen(path, "rb");
        if (fp == NULL)
                die_path(path);
        require_regular_image(fp, path);
        if (image_blocks(fp) != total || fseek(fp, 0L, SEEK_SET) != 0)
                die("invalid selected image geometry");
        for (i = 0U; i < BLOCK_WORDS; ++i)
                desc[i] = get64le(fp);
        if (fclose(fp) != 0)
                die_path(path);
        if (desc[AUXSTORE_DESC_MAGIC] != AUXSTORE_MAGIC ||
            desc[AUXSTORE_DESC_VERSION] != AUXSTORE_VERSION)
                die("not an AUXST1 image");
        /* Match KINIT's region bounds and overlap invariants.  Indices
         * 0..2 are BACKSTORE, LOGSTORE and CACHESTORE, 3 is D6FS. */
        for (i = 0U; i < 4U; ++i) {
                uint64_t v = desc[AUXSTORE_DESC_BACK + i];
                s[i] = (unsigned)((v >> 18) & HALF_MASK);
                n[i] = (unsigned)(v & HALF_MASK);
                if (n[i] != 0U && (s[i] == 0U || s[i] >= total ||
                    n[i] > total - s[i]))
                        die("invalid AUXST1 region bounds");
        }
        for (i = 0U; i < 4U; ++i)
                for (j = i + 1U; j < 4U; ++j)
                        if (n[i] != 0U && n[j] != 0U &&
                            s[i] < s[j] + n[j] && s[j] < s[i] + n[i])
                                die("overlapping AUXST1 regions");
        if (n[3] == 0U) {
                if (s[3] != 0U || desc[AUXSTORE_DESC_D6SUPER] != 0U)
                        die("invalid absent D6FS metadata");
        } else if ((unsigned)(desc[AUXSTORE_DESC_D6SUPER] >> 18) >= n[3] ||
            (unsigned)(desc[AUXSTORE_DESC_D6SUPER] & HALF_MASK) >= n[3] ||
            (unsigned)(desc[AUXSTORE_DESC_D6SUPER] >> 18) ==
            (unsigned)(desc[AUXSTORE_DESC_D6SUPER] & HALF_MASK)) {
                die("invalid D6FS superblock metadata");
        }
        for (i = AUXSTORE_DESC_D6SUPER + 1U; i < BLOCK_WORDS; ++i)
                if (desc[i] != 0U)
                        die("unknown AUXST1 descriptor extension");
        printf("mkswap: validated AUXST1 image %s (%o blocks)\n",
            path, total);
        inspect_auxstore(path);
}

static int
mkswap_main(int argc, char **argv)
{
        const char *image, *controller;
        unsigned unit, total, back, log, cache;
        unsigned seen;
        int format, inspect, dry_run, confirmed, i;

        image = controller = NULL;
        unit = total = back = log = cache = seen = 0U;
        format = inspect = dry_run = confirmed = 0;
        for (i = 1; i < argc; ++i) {
                const char *a = argv[i];
                if (strcmp(a, "--controller") == 0 && i + 1 < argc) {
                        if (seen & 1U) die("duplicate controller option");
                        controller = argv[++i]; seen |= 1U;
                } else if (strcmp(a, "--unit") == 0 && i + 1 < argc) {
                        if (seen & 2U) die("duplicate unit option");
                        unit = (unsigned)parse_number(argv[++i]); seen |= 2U;
                } else if (strcmp(a, "--image") == 0 && i + 1 < argc) {
                        if (seen & 4U) die("duplicate image option");
                        image = argv[++i]; seen |= 4U;
                } else if (strcmp(a, "--backstore-blocks") == 0 && i + 1 < argc) {
                        if (seen & 8U) die("duplicate BACKSTORE size");
                        back = (unsigned)parse_number(argv[++i]); seen |= 8U;
                } else if (strcmp(a, "--logstore-blocks") == 0 && i + 1 < argc) {
                        if (seen & 16U) die("duplicate LOGSTORE size");
                        log = (unsigned)parse_number(argv[++i]); seen |= 16U;
                } else if (strcmp(a, "--cache-blocks") == 0 && i + 1 < argc) {
                        if (seen & 32U) die("duplicate CACHESTORE size");
                        cache = (unsigned)parse_number(argv[++i]); seen |= 32U;
                } else if (strcmp(a, "--format") == 0) {
                        format = 1;
                } else if (strcmp(a, "--inspect") == 0) {
                        inspect = 1;
                } else if (strcmp(a, "--dry-run") == 0) {
                        dry_run = 1;
                } else if (strcmp(a, "--confirm-destructive") == 0) {
                        confirmed = 1;
                } else {
                        mkswap_usage();
                }
        }
        /* Omitted mode means inspection, never formatting. */
        if (!format && !inspect)
                inspect = 1;
        if ((seen & 7U) != 7U || image == NULL ||
            (strcmp(controller, "drm") != 0 &&
             strcmp(controller, "dsk") != 0) || unit != 0U ||
            (format == inspect) || (dry_run && confirmed))
                mkswap_usage();
        total = strcmp(controller, "drm") == 0 ? 020000U : 0130000U;
        mkswap_check_image(image, total);
        printf("mkswap: selected %s unit %u, offline image %s\n",
            controller, unit, image);
        if (inspect) {
                if ((seen & ~7U) != 0U || dry_run || confirmed)
                        mkswap_usage();
                mkswap_inspect(image, total);
                return 0;
        }
        if ((seen & 8U) == 0U || back == 0U ||
            log > total - 1U || back > total - 1U - log ||
            cache != total - 1U - log - back)
                die("BACKSTORE/LOGSTORE/CACHESTORE must fill the image tail");
        format_auxstore(image, back, log, cache, 0U, 0U, 0U, 0U,
            dry_run, confirmed, 0);
        return 0;
}

int
main(int argc, char **argv)
{
        const char *image, *blank, *inspect;
        unsigned blocks, back_blocks, log_blocks, cache_blocks;
        unsigned d6fs_base, d6fs_blocks, d6fs_super_a, d6fs_super_b;
        int i, dry_run, confirmed, convert_dboot;

        /* The two commands share the same AUXST1 writer, bounds checks,
         * blank-region scan and little-endian word encoding. */
        if (strrchr(argv[0], '/') != NULL &&
            strcmp(strrchr(argv[0], '/') + 1, "mkswap") == 0)
                return mkswap_main(argc, argv);
        if (strcmp(argv[0], "mkswap") == 0)
                return mkswap_main(argc, argv);
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
