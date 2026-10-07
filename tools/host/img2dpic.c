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

enum scan_axis {
    SCAN_HORIZONTAL = 0,
    SCAN_VERTICAL = 1
};

struct scan_plan {
    enum scan_axis axis;
    int reverse_first;
    size_t runs;
    size_t halfwords;
    size_t words;
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
    if (writer->fp != NULL &&
            fwrite(data, 1U, sizeof(data), writer->fp) != sizeof(data))
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
    if (writer->fp != NULL) {
        if (fclose(writer->fp) != 0)
            die("%s: close failed: %s", writer->path, strerror(errno));
        writer->fp = NULL;
    }
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

static unsigned char *
read_bitmap(MagickWand *wand, const char *path, size_t *lit_pixelsp)
{
    unsigned char *bitmap;
    size_t lit_pixels;
    size_t x;
    size_t y;

    bitmap = malloc(TARGET_SIZE * TARGET_SIZE);
    if (bitmap == NULL)
        die("%s: cannot allocate 1024x1024 bilevel raster", path);
    lit_pixels = 0U;
    for (y = 0U; y < TARGET_SIZE; ++y) {
        unsigned char *row;

        row = bitmap + y * TARGET_SIZE;
        if (MagickExportImagePixels(wand, 0,
                (ssize_t)(TARGET_SIZE - 1U - y),
                TARGET_SIZE, 1U, "I", CharPixel, row) == MagickFalse) {
            free(bitmap);
            wand_die(wand, path);
        }
        for (x = 0U; x < TARGET_SIZE; ++x)
            if (row[x] != 0U)
                ++lit_pixels;
    }
    *lit_pixelsp = lit_pixels;
    return bitmap;
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
    enum scan_axis axis, int final, struct convert_stats *stats)
{
    size_t remaining;
    unsigned int delta;

    remaining = length - 1U;
    if (remaining == 0U) {
        writer_put_half(writer, ty340_vector(final, 1,
            axis == SCAN_VERTICAL && direction < 0, 0U,
            axis == SCAN_HORIZONTAL && direction < 0, 0U));
        ++stats->draw_segments;
        return;
    }
    while (remaining != 0U) {
        delta = remaining > VECTOR_MAX_DELTA ?
            VECTOR_MAX_DELTA : (unsigned int)remaining;
        remaining -= delta;
        if (axis == SCAN_HORIZONTAL)
            writer_put_half(writer, ty340_vector(final && remaining == 0U,
                1, 0, 0U, direction < 0, delta));
        else
            writer_put_half(writer, ty340_vector(final && remaining == 0U,
                1, direction < 0, delta, 0, 0U));
        ++stats->draw_segments;
    }
}

static void
emit_run(struct word_writer *writer, size_t major, size_t start_minor,
    size_t length, int direction, enum scan_axis axis, size_t run_number,
    size_t run_count, int *cur_x, int *cur_y, struct convert_stats *stats)
{
    int start_x;
    int start_y;

    if (axis == SCAN_HORIZONTAL) {
        start_x = (int)start_minor;
        start_y = (int)major;
    } else {
        start_x = (int)major;
        start_y = (int)start_minor;
    }
    emit_vector_move(writer, start_x - *cur_x, start_y - *cur_y, stats);
    emit_vector_run(writer, length, direction, axis,
        run_number == run_count, stats);
    if (axis == SCAN_HORIZONTAL) {
        *cur_x = start_x + direction * (int)(length - 1U);
        *cur_y = start_y;
    } else {
        *cur_x = start_x;
        *cur_y = start_y + direction * (int)(length - 1U);
    }
}

static int
bitmap_pixel(const unsigned char *bitmap, enum scan_axis axis,
    size_t major, size_t minor)
{
    size_t x;
    size_t y;

    if (axis == SCAN_HORIZONTAL) {
        x = minor;
        y = major;
    } else {
        x = major;
        y = minor;
    }
    return bitmap[y * TARGET_SIZE + x] != 0U;
}

static size_t
count_runs(const unsigned char *bitmap, enum scan_axis axis)
{
    size_t major;
    size_t minor;
    size_t runs;
    int in_run;

    runs = 0U;
    for (major = 0U; major < TARGET_SIZE; ++major) {
        in_run = 0;
        for (minor = 0U; minor < TARGET_SIZE; ++minor) {
            if (bitmap_pixel(bitmap, axis, major, minor)) {
                if (!in_run) {
                    ++runs;
                    in_run = 1;
                }
            } else {
                in_run = 0;
            }
        }
    }
    return runs;
}

static void
emit_scan(struct word_writer *writer, const unsigned char *bitmap,
    enum scan_axis axis, int reverse_first, size_t run_count,
    struct convert_stats *stats)
{
    size_t begin;
    size_t end;
    size_t major;
    size_t minor;
    size_t run_number;
    int cur_x;
    int cur_y;
    int reverse;

    writer_put_half(writer, ty340_param(MODE_VECTOR, 1, INTENSITY_MAX));
    run_number = 0U;
    cur_x = 0;
    cur_y = 0;
    for (major = 0U; major < TARGET_SIZE; ++major) {
        reverse = ((major & 1U) != 0U) ^ reverse_first;
        if (!reverse) {
            minor = 0U;
            while (minor < TARGET_SIZE) {
                if (!bitmap_pixel(bitmap, axis, major, minor)) {
                    ++minor;
                    continue;
                }
                begin = minor;
                while (minor < TARGET_SIZE &&
                        bitmap_pixel(bitmap, axis, major, minor))
                    ++minor;
                ++run_number;
                emit_run(writer, major, begin, minor - begin, 1, axis,
                    run_number, run_count, &cur_x, &cur_y, stats);
            }
        } else {
            minor = TARGET_SIZE;
            while (minor != 0U) {
                if (!bitmap_pixel(bitmap, axis, major, minor - 1U)) {
                    --minor;
                    continue;
                }
                end = minor;
                while (minor != 0U &&
                        bitmap_pixel(bitmap, axis, major, minor - 1U))
                    --minor;
                ++run_number;
                emit_run(writer, major, end - 1U, end - minor, -1, axis,
                    run_number, run_count, &cur_x, &cur_y, stats);
            }
        }
    }
}

static struct scan_plan
measure_scan(const unsigned char *bitmap, enum scan_axis axis,
    int reverse_first, size_t runs)
{
    struct word_writer writer;
    struct convert_stats stats;
    struct scan_plan plan;

    memset(&writer, 0, sizeof(writer));
    memset(&stats, 0, sizeof(stats));
    emit_scan(&writer, bitmap, axis, reverse_first, runs, &stats);
    writer_finish(&writer);
    plan.axis = axis;
    plan.reverse_first = reverse_first;
    plan.runs = runs;
    plan.halfwords = writer.halfwords;
    plan.words = writer.words;
    return plan;
}

static int
plan_is_better(const struct scan_plan *a, const struct scan_plan *b)
{
    if (a->words != b->words)
        return a->words < b->words;
    if (a->halfwords != b->halfwords)
        return a->halfwords < b->halfwords;
    return a->runs < b->runs;
}

static void
convert_type340(const char *inpath, const char *outpath)
{
    MagickWand *wand;
    struct word_writer writer;
    struct convert_stats stats;
    struct scan_plan best;
    struct scan_plan candidate;
    unsigned char *bitmap;
    size_t horizontal_runs;
    size_t vertical_runs;
    size_t lit_pixels;

    memset(&stats, 0, sizeof(stats));
    wand = load_image(inpath);
    prepare_bilevel(wand, inpath);
    bitmap = read_bitmap(wand, inpath, &lit_pixels);
    horizontal_runs = count_runs(bitmap, SCAN_HORIZONTAL);
    vertical_runs = count_runs(bitmap, SCAN_VERTICAL);

    writer_open(&writer, outpath);
    if (lit_pixels != 0U) {
        best = measure_scan(bitmap, SCAN_HORIZONTAL, 0, horizontal_runs);
        candidate = measure_scan(bitmap, SCAN_HORIZONTAL, 1,
            horizontal_runs);
        if (plan_is_better(&candidate, &best))
            best = candidate;
        candidate = measure_scan(bitmap, SCAN_VERTICAL, 0, vertical_runs);
        if (plan_is_better(&candidate, &best))
            best = candidate;
        candidate = measure_scan(bitmap, SCAN_VERTICAL, 1, vertical_runs);
        if (plan_is_better(&candidate, &best))
            best = candidate;
        stats.lit_pixels = lit_pixels;
        stats.runs = best.runs;
        emit_scan(&writer, bitmap, best.axis, best.reverse_first,
            best.runs, &stats);
    } else {
        best.axis = SCAN_HORIZONTAL;
        best.reverse_first = 0;
        best.runs = 0U;
        writer_put_half(&writer,
            ty340_param(MODE_PARAM, 1, INTENSITY_MAX));
    }
    writer_finish(&writer);
    free(bitmap);
    DestroyMagickWand(wand);

    if (writer.words > TYPE340_MAX_WORDS) {
        (void)unlink(outpath);
        die("converted Type 340 program needs %zu words; maximum supported "
            "by DPYVIEW is %u", writer.words, TYPE340_MAX_WORDS);
    }

    fprintf(stderr,
        "img2dpic: TYPE340 %s -> %s\n"
        "img2dpic: converted to 1-bit and scaled image %ux%u, "
        "scan=%s%s, lit pixels=%zu, runs=%zu, move segments=%zu, "
        "draw segments=%zu, halfwords=%zu, words=%zu\n",
        inpath, outpath, TARGET_SIZE, TARGET_SIZE,
        best.axis == SCAN_HORIZONTAL ? "horizontal" : "vertical",
        best.reverse_first ? "-reverse" : "",
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
