#include <MagickWand/MagickWand.h>

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TARGET_SIZE 1024U
#define NEAR_SQUARE_DIVISOR 64U
#define VECTOR_MAX_DELTA 127U
#define INTENSITY_MAX 7U
#define TYPE340_MAX_WORDS 32256U

#define MODE_PARAM 0U
#define MODE_VECTOR 4U

struct word_writer {
    FILE *fp;
    const char *path;
    uint32_t left;
    int have_left;
    size_t words;
    size_t halfwords;
};

struct convert_stats {
    size_t lit_pixels;
    size_t runs;
    size_t move_segments;
    size_t draw_segments;
};

static void
usage(FILE *fp)
{
    fputs("usage: img2dpic [-t 340] INPUT OUTPUT\n"
          "       img2dpic -h\n"
          "\n"
          "Reads an opaque image through ImageMagick, scales it to 1024x1024,\n"
          "converts it to 1-bit, and emits a Type 340 display program as\n"
          "little-endian 64-bit containers whose low 36 bits hold one PDP-10\n"
          "word.\n", fp);
}

static void
die(const char *fmt, ...)
{
    va_list ap;

    fputs("img2dpic: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static void
wand_die(MagickWand *wand, const char *prefix)
{
    ExceptionType severity;
    char *description;

    description = MagickGetException(wand, &severity);
    if (description != NULL && *description != '\0')
        die("%s: %s", prefix, description);
    die("%s: ImageMagick operation failed", prefix);
}

static uint32_t
bit18(unsigned int bit)
{
    if (bit > 17U)
        die("internal invalid Type 340 bit index");
    return UINT32_C(1) << (17U - bit);
}

static uint32_t
field18(unsigned int start, unsigned int end, unsigned int value)
{
    unsigned int width;
    uint32_t mask;

    if (start > end || end > 17U)
        die("internal invalid Type 340 field");
    width = end - start + 1U;
    mask = (UINT32_C(1) << width) - UINT32_C(1);
    if (((uint32_t)value & ~mask) != 0U)
        die("internal Type 340 field overflow");
    return ((uint32_t)value & mask) << (17U - end);
}

static uint32_t
ty340_param(unsigned int next_mode, int set_intensity, unsigned int intensity)
{
    uint32_t inst;

    inst = field18(2U, 4U, next_mode & 07U);
    if (set_intensity) {
        inst |= bit18(14U);
        inst |= field18(15U, 17U, intensity & 07U);
    }
    return inst;
}

static uint32_t
ty340_vector(int escape, int intensify, int sy, unsigned int dy,
    int sx, unsigned int dx)
{
    uint32_t inst;

    if (dy > VECTOR_MAX_DELTA || dx > VECTOR_MAX_DELTA)
        die("internal vector delta out of range");
    inst = field18(3U, 9U, dy) | field18(11U, 17U, dx);
    if (escape)
        inst |= bit18(0U);
    if (intensify)
        inst |= bit18(1U);
    if (sy)
        inst |= bit18(2U);
    if (sx)
        inst |= bit18(10U);
    return inst;
}

static int
near_square(size_t width, size_t height)
{
    size_t delta;
    size_t larger;
    size_t limit;

    delta = width > height ? width - height : height - width;
    larger = width > height ? width : height;
    limit = (larger + NEAR_SQUARE_DIVISOR - 1U) / NEAR_SQUARE_DIVISOR;
    if (limit == 0U)
        limit = 1U;
    return delta <= limit;
}

static void
writer_open(struct word_writer *writer, const char *path)
{
    memset(writer, 0, sizeof(*writer));
    writer->path = path;
    writer->fp = fopen(path, "wb");
    if (writer->fp == NULL)
        die("%s: %s", path, strerror(errno));
}

static void
writer_put_word(struct word_writer *writer, uint64_t word)
{
    unsigned char data[8];
    unsigned int i;

    if ((word >> 36) != 0U)
        die("internal output word exceeds 36 bits");
    for (i = 0U; i < 8U; ++i)
        data[i] = (unsigned char)(word >> (8U * i));
    if (fwrite(data, 1U, sizeof(data), writer->fp) != sizeof(data))
        die("%s: write failed: %s", writer->path, strerror(errno));
    ++writer->words;
}

static void
writer_put_half(struct word_writer *writer, uint32_t half)
{
    if ((half >> 18) != 0U)
        die("internal output halfword exceeds 18 bits");
    if (!writer->have_left) {
        writer->left = half;
        writer->have_left = 1;
    } else {
        writer_put_word(writer,
            ((uint64_t)writer->left << 18) | (uint64_t)half);
        writer->left = 0U;
        writer->have_left = 0;
    }
    ++writer->halfwords;
}

static void
writer_finish(struct word_writer *writer)
{
    if (writer->have_left) {
        writer_put_word(writer, (uint64_t)writer->left << 18);
        writer->left = 0U;
        writer->have_left = 0;
    }
    if (writer->words == 0U)
        writer_put_word(writer, 0U);
    if (fclose(writer->fp) != 0)
        die("%s: close failed: %s", writer->path, strerror(errno));
    writer->fp = NULL;
}

static void
check_opaque(MagickWand *wand, const char *path, size_t width, size_t height)
{
    unsigned char *alpha;
    size_t x;
    size_t y;

    if (MagickGetImageAlphaChannel(wand) == MagickFalse)
        return;
    if (width == 0U || width > SIZE_MAX / sizeof(*alpha))
        die("%s: invalid image width", path);
    alpha = malloc(width * sizeof(*alpha));
    if (alpha == NULL)
        die("%s: cannot allocate alpha scanline", path);
    for (y = 0U; y < height; ++y) {
        if (MagickExportImagePixels(wand, 0, (ssize_t)y, width, 1U,
                "A", CharPixel, alpha) == MagickFalse) {
            free(alpha);
            wand_die(wand, path);
        }
        for (x = 0U; x < width; ++x) {
            if (alpha[x] != 255U) {
                free(alpha);
                die("%s: transparency is not allowed for Type 340 input",
                    path);
            }
        }
    }
    free(alpha);
}

static MagickWand *
load_image(const char *path)
{
    MagickWand *wand;
    size_t frames;
    size_t width;
    size_t height;

    wand = NewMagickWand();
    if (wand == NULL)
        die("cannot allocate ImageMagick wand");
    if (MagickReadImage(wand, path) == MagickFalse)
        wand_die(wand, path);
    frames = MagickGetNumberImages(wand);
    if (frames != 1U)
        die("%s: only single-image inputs are supported", path);
    width = MagickGetImageWidth(wand);
    height = MagickGetImageHeight(wand);
    if (width <= TARGET_SIZE || height <= TARGET_SIZE)
        die("%s: both dimensions must be greater than %u pixels (got %zux%zu)",
            path, TARGET_SIZE, width, height);
    if (!near_square(width, height))
        die("%s: image must be square or near-square (got %zux%zu; "
            "max delta 1/%u of the larger side)",
            path, width, height, NEAR_SQUARE_DIVISOR);
    check_opaque(wand, path, width, height);
    return wand;
}

static void
prepare_bilevel(MagickWand *wand, const char *path)
{
    if (MagickResizeImage(wand, TARGET_SIZE, TARGET_SIZE,
            UndefinedFilter) == MagickFalse)
        wand_die(wand, path);
    if (MagickTransformImageColorspace(wand, GRAYColorspace) == MagickFalse)
        wand_die(wand, path);
    if (MagickThresholdImage(wand, (double)QuantumRange / 2.0) == MagickFalse)
        wand_die(wand, path);
}

static void
read_row(MagickWand *wand, const char *path, size_t source_y,
    unsigned char row[TARGET_SIZE])
{
    if (MagickExportImagePixels(wand, 0, (ssize_t)source_y,
            TARGET_SIZE, 1U, "I", CharPixel, row) == MagickFalse)
        wand_die(wand, path);
}

static void
count_row_runs(const unsigned char row[TARGET_SIZE], struct convert_stats *stats)
{
    size_t x;

    x = 0U;
    while (x < TARGET_SIZE) {
        if (row[x] == 0U) {
            ++x;
            continue;
        }
        ++stats->runs;
        while (x < TARGET_SIZE && row[x] != 0U) {
            ++stats->lit_pixels;
            ++x;
        }
    }
}

static void
emit_vector_move(struct word_writer *writer, int dx, int dy,
    struct convert_stats *stats)
{
    unsigned int sx;
    unsigned int sy;

    while (dx != 0 || dy != 0) {
        sx = (unsigned int)(dx < 0 ? -dx : dx);
        sy = (unsigned int)(dy < 0 ? -dy : dy);
        if (sx > VECTOR_MAX_DELTA)
            sx = VECTOR_MAX_DELTA;
        if (sy > VECTOR_MAX_DELTA)
            sy = VECTOR_MAX_DELTA;
        writer_put_half(writer, ty340_vector(0, 0, dy < 0, sy,
            dx < 0, sx));
        ++stats->move_segments;
        dx += dx < 0 ? (int)sx : -(int)sx;
        dy += dy < 0 ? (int)sy : -(int)sy;
    }
}

static void
emit_vector_run(struct word_writer *writer, size_t length, int direction,
    int final, struct convert_stats *stats)
{
    size_t remaining;
    unsigned int delta;

    remaining = length - 1U;
    if (remaining == 0U) {
        writer_put_half(writer, ty340_vector(final, 1, 0, 0U,
            direction < 0, 0U));
        ++stats->draw_segments;
        return;
    }
    while (remaining != 0U) {
        delta = remaining > VECTOR_MAX_DELTA ?
            VECTOR_MAX_DELTA : (unsigned int)remaining;
        remaining -= delta;
        writer_put_half(writer, ty340_vector(final && remaining == 0U,
            1, 0, 0U, direction < 0, delta));
        ++stats->draw_segments;
    }
}

static void
emit_run(struct word_writer *writer, size_t start_x, size_t y, size_t length,
    int direction, size_t run_number, size_t run_count, int *cur_x,
    int *cur_y, struct convert_stats *stats)
{
    emit_vector_move(writer, (int)start_x - *cur_x, (int)y - *cur_y, stats);
    emit_vector_run(writer, length, direction, run_number == run_count, stats);
    *cur_x = (int)start_x + direction * (int)(length - 1U);
    *cur_y = (int)y;
}

static void
emit_row_runs(struct word_writer *writer, const unsigned char row[TARGET_SIZE],
    size_t y, size_t *run_number, size_t run_count, int *cur_x, int *cur_y,
    struct convert_stats *stats)
{
    size_t begin;
    size_t end;
    size_t x;

    if ((y & 1U) == 0U) {
        x = 0U;
        while (x < TARGET_SIZE) {
            if (row[x] == 0U) {
                ++x;
                continue;
            }
            begin = x;
            while (x < TARGET_SIZE && row[x] != 0U)
                ++x;
            ++*run_number;
            emit_run(writer, begin, y, x - begin, 1, *run_number,
                run_count, cur_x, cur_y, stats);
        }
        return;
    }

    x = TARGET_SIZE;
    while (x != 0U) {
        if (row[x - 1U] == 0U) {
            --x;
            continue;
        }
        end = x;
        while (x != 0U && row[x - 1U] != 0U)
            --x;
        ++*run_number;
        emit_run(writer, end - 1U, y, end - x, -1, *run_number,
            run_count, cur_x, cur_y, stats);
    }
}

static void
convert_type340(const char *inpath, const char *outpath)
{
    MagickWand *wand;
    struct word_writer writer;
    struct convert_stats stats;
    unsigned char row[TARGET_SIZE];
    size_t y;
    size_t run_number;
    int cur_x;
    int cur_y;

    memset(&stats, 0, sizeof(stats));
    wand = load_image(inpath);
    prepare_bilevel(wand, inpath);

    for (y = 0U; y < TARGET_SIZE; ++y) {
        read_row(wand, inpath, TARGET_SIZE - 1U - y, row);
        count_row_runs(row, &stats);
    }

    writer_open(&writer, outpath);
    if (stats.runs != 0U) {
        writer_put_half(&writer,
            ty340_param(MODE_VECTOR, 1, INTENSITY_MAX));
        run_number = 0U;
        cur_x = 0;
        cur_y = 0;
        for (y = 0U; y < TARGET_SIZE; ++y) {
            read_row(wand, inpath, TARGET_SIZE - 1U - y, row);
            emit_row_runs(&writer, row, y, &run_number, stats.runs,
                &cur_x, &cur_y, &stats);
        }
    } else {
        writer_put_half(&writer,
            ty340_param(MODE_PARAM, 1, INTENSITY_MAX));
    }
    writer_finish(&writer);
    DestroyMagickWand(wand);

    if (writer.words > TYPE340_MAX_WORDS) {
        (void)unlink(outpath);
        die("converted Type 340 program needs %zu words; maximum supported "
            "by DPYVIEW is %u", writer.words, TYPE340_MAX_WORDS);
    }

    fprintf(stderr,
        "img2dpic: TYPE340 %s -> %s\n"
        "img2dpic: converted to 1-bit and scaled image %ux%u, "
        "lit pixels=%zu, runs=%zu, move segments=%zu, "
        "draw segments=%zu, halfwords=%zu, words=%zu\n",
        inpath, outpath, TARGET_SIZE, TARGET_SIZE,
        stats.lit_pixels, stats.runs, stats.move_segments,
        stats.draw_segments, writer.halfwords, writer.words);
}

int
main(int argc, char **argv)
{
    const char *out_type;
    int i;

    out_type = "340";
    i = 1;
    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(stdout);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--type") == 0) {
            if (i + 1 >= argc)
                die("missing argument after -t/--type");
            out_type = argv[i + 1];
            i += 2;
            continue;
        }
        die("unknown option: %s", argv[i]);
    }
    if (argc - i != 2) {
        usage(stderr);
        return EXIT_FAILURE;
    }
    if (strcmp(out_type, "340") != 0 && strcmp(out_type, "type340") != 0)
        die("only Type 340 output is implemented");

    MagickWandGenesis();
    convert_type340(argv[i], argv[i + 1]);
    MagickWandTerminus();
    return EXIT_SUCCESS;
}
