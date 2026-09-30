#include "u.h"
#include "exec.h"

#define D6LZ_WINDOW 128U
#define D6LZ_LOOK 130U
#define D6LZ_RING (D6LZ_WINDOW + D6LZ_LOOK)
#define D6LZ_GROUP_TOKENS 36U
#define D6LZ_MIN_MATCH 3U
#define D6LZ_CONTROL_FIRST 0400000000000UL
#define D6LZ_DESC_LEN_SHIFT 7U
#define D6LZ_HALF_MASK 0777777UL
#define D6LZ_WORD_MASK 0777777777777UL
#define D6LZ_DXR_MAGIC \
    ((PDP10_SIX6('D','X','R',' ',' ',' ') >> 18U) & D6LZ_HALF_MASK)

static kword_t d6lz_ring[D6LZ_RING];
static kword_t d6lz_group[D6LZ_GROUP_TOKENS + 1U];
static unsigned int d6lz_head;
static unsigned int d6lz_hist;
static unsigned int d6lz_avail;
static kword_t d6lz_left;
static int d6lz_infd;
static int d6lz_outfd;
static kword_t d6lz_out_words;

static int
s6_same(const kword_t *a, const kword_t *b)
{
        unsigned int i;
        unsigned int words;

        if (a == 0 || b == 0 || a[0] != b[0]) return 0;
        words = 1U + ((unsigned int)a[0] + 5U) / 6U;
        for (i = 0U; i < words; ++i)
                if (a[i] != b[i]) return 0;
        return 1;
}

static unsigned int
ring_index(int offset)
{
        int i;

        i = (int)d6lz_head + offset;
        if (i < 0) i += (int)D6LZ_RING;
        else if (i >= (int)D6LZ_RING) i -= (int)D6LZ_RING;
        return (unsigned int)i;
}

static int
fill_lookahead(void)
{
        unsigned int free_words;
        unsigned int need;
        unsigned int tail;
        unsigned int chunk;
        int n;

        while (d6lz_avail < D6LZ_LOOK && d6lz_left != 0) {
                free_words = D6LZ_RING - d6lz_hist - d6lz_avail;
                if (free_words == 0U) return -1;
                need = D6LZ_LOOK - d6lz_avail;
                if ((kword_t)need > d6lz_left) need = (unsigned int)d6lz_left;
                if (need > free_words) need = free_words;
                tail = d6lz_head + d6lz_avail;
                if (tail >= D6LZ_RING) tail -= D6LZ_RING;
                chunk = D6LZ_RING - tail;
                if (chunk > need) chunk = need;
                n = dsys_read_words(d6lz_infd, &d6lz_ring[tail], chunk);
                if (n <= 0) return -1;
                d6lz_avail += (unsigned int)n;
                d6lz_left -= (kword_t)(unsigned int)n;
        }
        return 0;
}

static void
consume_words(unsigned int n)
{
        d6lz_head += n;
        while (d6lz_head >= D6LZ_RING) d6lz_head -= D6LZ_RING;
        d6lz_avail -= n;
        if (d6lz_hist + n >= D6LZ_WINDOW) d6lz_hist = D6LZ_WINDOW;
        else d6lz_hist += n;
}

static unsigned int
best_match(unsigned int *distp)
{
        unsigned int dist;
        unsigned int len;
        unsigned int best_len;
        unsigned int best_dist;

        best_len = 0U;
        best_dist = 0U;
        for (dist = 1U; dist <= d6lz_hist; ++dist) {
                len = 0U;
                while (len < d6lz_avail && len < D6LZ_LOOK &&
                    d6lz_ring[ring_index((int)len)] ==
                    d6lz_ring[ring_index((int)len - (int)dist)])
                        ++len;
                if (len >= D6LZ_MIN_MATCH && len > best_len) {
                        best_len = len;
                        best_dist = dist;
                        if (best_len == D6LZ_LOOK) break;
                }
        }
        *distp = best_dist;
        return best_len;
}

static int
write_words(const kword_t *words, unsigned int n)
{
        if (u_write_words_all(d6lz_outfd, words, n) != 0)
                return -1;
        d6lz_out_words += (kword_t)n;
        return 0;
}

static int
compress_words(kword_t words)
{
        unsigned int ntok;
        unsigned int len;
        unsigned int dist;
        kword_t control;
        kword_t mask;

        d6lz_head = 0U;
        d6lz_hist = 0U;
        d6lz_avail = 0U;
        d6lz_left = words;
        for (;;) {
                control = 0;
                mask = D6LZ_CONTROL_FIRST;
                ntok = 0U;
                while (ntok < D6LZ_GROUP_TOKENS) {
                        if (fill_lookahead() != 0) return -1;
                        if (d6lz_avail == 0U) break;
                        len = best_match(&dist);
                        if (len >= D6LZ_MIN_MATCH) {
                                control |= mask;
                                d6lz_group[ntok + 1U] =
                                    ((kword_t)(len - D6LZ_MIN_MATCH) <<
                                    D6LZ_DESC_LEN_SHIFT) |
                                    (kword_t)(dist - 1U);
                                consume_words(len);
                        } else {
                                d6lz_group[ntok + 1U] =
                                    d6lz_ring[d6lz_head] & D6LZ_WORD_MASK;
                                consume_words(1U);
                        }
                        ++ntok;
                        mask >>= 1U;
                }
                if (ntok == 0U) break;
                d6lz_group[0] = control;
                if (write_words(d6lz_group, ntok + 1U) != 0) return -1;
        }
        return d6lz_left == 0 && d6lz_avail == 0U ? 0 : -1;
}

static int
copy_words(kword_t words)
{
        unsigned int n;
        int got;

        while (words != 0) {
                n = D6LZ_RING;
                if ((kword_t)n > words) n = (unsigned int)words;
                got = dsys_read_words(d6lz_infd, d6lz_ring, n);
                if (got != (int)n) return -1;
                if (write_words(d6lz_ring, n) != 0) return -1;
                words -= (kword_t)n;
        }
        return 0;
}

static int
compress_raw(const struct vfs_stat *st)
{
        return compress_words(st->size_words);
}

static int
compress_exec(const struct vfs_stat *st)
{
        kword_t hdr[EXEC_DXR_EXT_HDR_WORDS];
        kword_t expected;
        kword_t image_words;
        kword_t reloc_words;
        kword_t header_words;
        kword_t text_words;
        unsigned int bss_words;
        unsigned int flags;

        if (st->size_words < EXEC_DXR_BASE_HDR_WORDS ||
            dsys_read_words(d6lz_infd, hdr, EXEC_DXR_BASE_HDR_WORDS) !=
            (int)EXEC_DXR_BASE_HDR_WORDS ||
            ((hdr[0] >> 18U) & D6LZ_HALF_MASK) != D6LZ_DXR_MAGIC)
                return -1;
        image_words = (hdr[1] >> 18U) & D6LZ_HALF_MASK;
        bss_words = (unsigned int)(hdr[1] & EXEC_DXR_BSS_MASK);
        flags = (unsigned int)(hdr[1] &
            (EXEC_DXR_F_COMPRESSED | EXEC_DXR_F_PURE | EXEC_DXR_F_RT_REQUIRED));
        if (image_words == 0 || image_words > EXEC_DXR_MAX_IMAGE_WORDS ||
            bss_words > EXEC_DXR_MAX_BSS_WORDS ||
            (hdr[0] & D6LZ_HALF_MASK) >= image_words ||
            (flags & EXEC_DXR_F_COMPRESSED) != 0U)
                return -1;
        reloc_words = (image_words + 35U) / 36U;
        expected = EXEC_DXR_BASE_HDR_WORDS + image_words + reloc_words;
        header_words = EXEC_DXR_BASE_HDR_WORDS;
        text_words = 0;
        if (st->size_words == expected + 1U) {
                if (dsys_read_words(d6lz_infd, &hdr[2], 1U) != 1 ||
                    (hdr[2] & D6LZ_HALF_MASK) != EXEC_DXR_TEXT_TAG)
                        return -1;
                text_words = (hdr[2] >> 18U) & D6LZ_HALF_MASK;
                if (text_words > image_words) return -1;
                header_words = EXEC_DXR_EXT_HDR_WORDS;
        } else if (st->size_words != expected) {
                return -1;
        }
        hdr[1] |= EXEC_DXR_F_COMPRESSED;
        if (header_words == EXEC_DXR_BASE_HDR_WORDS)
                hdr[2] = EXEC_DXR_TEXT_TAG;
        if (dsys_seek(d6lz_infd, header_words, SYS_SEEK_SET) ==
            (kword_t)-1)
                return -1;
        if (write_words(hdr, EXEC_DXR_EXT_HDR_WORDS) != 0 ||
            compress_words(image_words) != 0 ||
            copy_words(reloc_words) != 0)
                return -1;
        return 0;
}

static int
fail(const char *what, const kword_t *path)
{
        (void)u_puts(2, "?D6LZ ");
        (void)u_puts(2, what);
        if (path != 0) {
                (void)u_putc(2, ' ');
                (void)u_put_s6(2, path);
        }
        (void)u_crlf(2);
        return 1;
}

int
main(int argc, kword_t **argv)
{
        struct vfs_stat st;
        const kword_t *inpath;
        const kword_t *outpath;
        int exec_mode;
        int rc;

        exec_mode = 0;
        if (argc == 3) {
                inpath = argv[1];
                outpath = argv[2];
        } else if (argc == 4 && u_s6_eq(argv[1], "-X")) {
                exec_mode = 1;
                inpath = argv[2];
                outpath = argv[3];
        } else {
                return fail("USAGE", 0);
        }
        if (s6_same(inpath, outpath)) return fail("SAME FILE", inpath);
        if (dsys_stat((kword_t *)inpath, &st) != 0 || st.type != VFS_TYPE_REG)
                return fail("INPUT", inpath);
        d6lz_infd = dsys_open((kword_t *)inpath, SYS_O_RDONLY);
        if (d6lz_infd < 0) return fail("INPUT", inpath);
        d6lz_outfd = dsys_open((kword_t *)outpath,
            SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC);
        if (d6lz_outfd < 0) {
                (void)dsys_close(d6lz_infd);
                return fail("OUTPUT", outpath);
        }
        d6lz_out_words = 0;
        rc = exec_mode ? compress_exec(&st) : compress_raw(&st);
        if (dsys_close(d6lz_infd) != 0) rc = -1;
        if (dsys_close(d6lz_outfd) != 0) rc = -1;
        if (rc == 0 && dsys_chmod((kword_t *)outpath, st.mode & 0777U) != 0)
                rc = -1;
        return rc == 0 ? 0 : fail("FAILED", inpath);
}
