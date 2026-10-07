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
#define STROKE_COVERAGE_RADIUS 2U
#define STROKE_MAX_SOURCE_RADIUS 5U
#define STROKE_OUTPUT_RADIUS 1U
#define STROKE_MIN_COVERAGE_PERMILLE 995U
#define STROKE_RDP_TOLERANCE_SQUARED 1.0
#define STROKE_MAX_COUNT 4096U

#define MODE_PARAM 0U
#define MODE_VECTOR 4U
#define MODE_INCR 6U

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
    size_t incremental_paths;
    size_t incremental_steps;
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

struct incremental_plan {
    size_t paths;
    size_t steps;
    size_t halfwords;
    size_t words;
};

struct stroke_point {
    uint16_t x;
    uint16_t y;
};

struct stroke {
    size_t offset;
    size_t length;
};

struct stroke_set {
    struct stroke_point *points;
    struct stroke *strokes;
    size_t point_count;
    size_t point_capacity;
    size_t stroke_count;
    size_t stroke_capacity;
};

struct stroke_plan {
    int suitable;
    size_t skeleton_pixels;
    size_t strokes;
    size_t halfwords;
    size_t words;
    double usec;
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

static uint32_t
ty340_incremental(int escape, int intensify,
    unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3)
{
    uint32_t inst;

    if (p0 > 017U || p1 > 017U || p2 > 017U || p3 > 017U)
        die("internal incremental step out of range");
    inst = field18(2U, 5U, p0) |
        field18(6U, 9U, p1) |
        field18(10U, 13U, p2) |
        field18(14U, 17U, p3);
    if (escape)
        inst |= bit18(0U);
    if (intensify)
        inst |= bit18(1U);
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

static unsigned int
incremental_step_code(int dx, int dy)
{
    unsigned int code;

    code = 0U;
    if (dx < 0)
        code |= 014U;
    else if (dx > 0)
        code |= 010U;
    if (dy < 0)
        code |= 003U;
    else if (dy > 0)
        code |= 002U;
    if (code == 0U)
        die("internal zero incremental step");
    return code;
}

static void
incremental_step_delta(unsigned int code, int *dx, int *dy)
{
    *dx = 0;
    *dy = 0;
    if ((code & 010U) != 0U)
        *dx = (code & 004U) != 0U ? -1 : 1;
    if ((code & 002U) != 0U)
        *dy = (code & 001U) != 0U ? -1 : 1;
}

static int
incremental_degree(const unsigned char *remaining, int x, int y)
{
    static const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int degree;
    int i;
    int nx;
    int ny;

    degree = 0;
    for (i = 0; i < 8; ++i) {
        nx = x + dx[i];
        ny = y + dy[i];
        if (nx >= 0 && nx < (int)TARGET_SIZE &&
                ny >= 0 && ny < (int)TARGET_SIZE &&
                remaining[(size_t)ny * TARGET_SIZE + (size_t)nx] != 0U)
            ++degree;
    }
    return degree;
}

static int
incremental_next(const unsigned char *remaining, int x, int y,
    int prev_dx, int prev_dy, int *next_x, int *next_y,
    unsigned int *step_code)
{
    static const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int best_degree;
    int best_turn;
    int degree;
    int have_best;
    int i;
    int nx;
    int ny;
    int turn;

    have_best = 0;
    best_degree = 0;
    best_turn = 0;
    for (i = 0; i < 8; ++i) {
        nx = x + dx[i];
        ny = y + dy[i];
        if (nx < 0 || nx >= (int)TARGET_SIZE ||
                ny < 0 || ny >= (int)TARGET_SIZE ||
                remaining[(size_t)ny * TARGET_SIZE + (size_t)nx] == 0U)
            continue;
        degree = incremental_degree(remaining, nx, ny);
        if (prev_dx < -1) {
            turn = 0;
        } else {
            int tx;
            int ty;

            tx = dx[i] - prev_dx;
            ty = dy[i] - prev_dy;
            turn = tx * tx + ty * ty;
        }
        if (!have_best || degree < best_degree ||
                (degree == best_degree && turn < best_turn)) {
            have_best = 1;
            best_degree = degree;
            best_turn = turn;
            *next_x = nx;
            *next_y = ny;
            *step_code = incremental_step_code(dx[i], dy[i]);
        }
    }
    return have_best;
}

static void
emit_incremental(struct word_writer *writer, const unsigned char *bitmap,
    struct convert_stats *stats)
{
    unsigned char *path;
    unsigned char *remaining;
    size_t bitmap_bytes;
    size_t i;
    size_t path_length;
    size_t scan;
    int cur_x;
    int cur_y;

    bitmap_bytes = TARGET_SIZE * TARGET_SIZE;
    remaining = malloc(bitmap_bytes);
    path = malloc(bitmap_bytes);
    if (remaining == NULL || path == NULL) {
        free(path);
        free(remaining);
        die("cannot allocate incremental-path workspace");
    }
    memcpy(remaining, bitmap, bitmap_bytes);

    writer_put_half(writer, ty340_param(MODE_VECTOR, 1, INTENSITY_MAX));
    cur_x = 0;
    cur_y = 0;
    scan = 0U;
    while (scan < bitmap_bytes) {
        size_t start;
        int prev_dx;
        int prev_dy;
        int x;
        int y;

        while (scan < bitmap_bytes && remaining[scan] == 0U)
            ++scan;
        if (scan == bitmap_bytes)
            break;

        start = scan;
        x = (int)(start % TARGET_SIZE);
        y = (int)(start / TARGET_SIZE);
        emit_vector_move(writer, x - cur_x, y - cur_y, stats);

        /*
         * Light the first pixel with a zero-length vector and escape to
         * parameter mode.  Increment mode lights only after each one-pixel
         * step, so this preserves isolated pixels and path starting points.
         */
        writer_put_half(writer, ty340_vector(1, 1, 0, 0U, 0, 0U));
        ++stats->draw_segments;
        ++stats->incremental_paths;
        remaining[start] = 0U;

        path_length = 0U;
        prev_dx = -2;
        prev_dy = -2;
        for (;;) {
            unsigned int code;
            int next_x;
            int next_y;

            if (!incremental_next(remaining, x, y, prev_dx, prev_dy,
                    &next_x, &next_y, &code))
                break;
            if (path_length >= bitmap_bytes)
                die("internal incremental path overflow");
            path[path_length++] = (unsigned char)code;
            ++stats->incremental_steps;
            prev_dx = next_x - x;
            prev_dy = next_y - y;
            x = next_x;
            y = next_y;
            remaining[(size_t)y * TARGET_SIZE + (size_t)x] = 0U;
        }

        if (path_length >= 4U) {
            size_t packed_length;

            packed_length = path_length & ~(size_t)3U;
            writer_put_half(writer, ty340_param(MODE_INCR, 0, 0U));
            for (i = 0U; i < packed_length; i += 4U) {
                int final;

                final = i + 4U == packed_length;
                writer_put_half(writer, ty340_incremental(final, 1,
                    path[i], path[i + 1U], path[i + 2U], path[i + 3U]));
            }
        }

        /*
         * Increment mode has one intensity bit for all four substeps.  Do not
         * pad a short final group with zero-motion substeps: those would
         * re-intensify the endpoint and can create a brighter dot on a real
         * refresh CRT.  Emit the one-to-three residual steps as exact
         * one-pixel vectors instead.
         */
        if ((path_length & 3U) != 0U) {
            size_t tail;

            tail = path_length & ~(size_t)3U;
            writer_put_half(writer, ty340_param(MODE_VECTOR, 0, 0U));
            for (i = tail; i < path_length; ++i) {
                int dx;
                int dy;
                int final;

                incremental_step_delta(path[i], &dx, &dy);
                final = i + 1U == path_length;
                writer_put_half(writer, ty340_vector(final, 1,
                    dy < 0, (unsigned int)(dy < 0 ? -dy : dy),
                    dx < 0, (unsigned int)(dx < 0 ? -dx : dx)));
                ++stats->draw_segments;
            }
        }

        cur_x = x;
        cur_y = y;
        scan = start + 1U;
        while (scan < bitmap_bytes && remaining[scan] == 0U)
            ++scan;
        if (scan < bitmap_bytes)
            writer_put_half(writer, ty340_param(MODE_VECTOR, 0, 0U));
    }

    free(path);
    free(remaining);
}

static struct incremental_plan
measure_incremental(const unsigned char *bitmap)
{
    struct convert_stats stats;
    struct incremental_plan plan;
    struct word_writer writer;

    memset(&stats, 0, sizeof(stats));
    memset(&writer, 0, sizeof(writer));
    emit_incremental(&writer, bitmap, &stats);
    writer_finish(&writer);
    plan.paths = stats.incremental_paths;
    plan.steps = stats.incremental_steps;
    plan.halfwords = writer.halfwords;
    plan.words = writer.words;
    return plan;
}

static void
stroke_set_init(struct stroke_set *set)
{
    memset(set, 0, sizeof(*set));
}

static void
stroke_set_free(struct stroke_set *set)
{
    free(set->points);
    free(set->strokes);
    memset(set, 0, sizeof(*set));
}

static void
stroke_set_reserve_points(struct stroke_set *set, size_t need)
{
    struct stroke_point *new_points;
    size_t capacity;

    if (need <= set->point_capacity)
        return;
    capacity = set->point_capacity == 0U ? 1024U : set->point_capacity;
    while (capacity < need) {
        if (capacity > SIZE_MAX / 2U)
            die("stroke point table overflow");
        capacity *= 2U;
    }
    new_points = realloc(set->points, capacity * sizeof(*new_points));
    if (new_points == NULL)
        die("cannot allocate stroke point table");
    set->points = new_points;
    set->point_capacity = capacity;
}

static void
stroke_set_reserve_strokes(struct stroke_set *set, size_t need)
{
    struct stroke *new_strokes;
    size_t capacity;

    if (need <= set->stroke_capacity)
        return;
    capacity = set->stroke_capacity == 0U ? 128U : set->stroke_capacity;
    while (capacity < need) {
        if (capacity > SIZE_MAX / 2U)
            die("stroke table overflow");
        capacity *= 2U;
    }
    new_strokes = realloc(set->strokes, capacity * sizeof(*new_strokes));
    if (new_strokes == NULL)
        die("cannot allocate stroke table");
    set->strokes = new_strokes;
    set->stroke_capacity = capacity;
}

static size_t
stroke_begin(struct stroke_set *set)
{
    size_t index;

    stroke_set_reserve_strokes(set, set->stroke_count + 1U);
    index = set->stroke_count++;
    set->strokes[index].offset = set->point_count;
    set->strokes[index].length = 0U;
    return index;
}

static void
stroke_add_point(struct stroke_set *set, size_t stroke_index,
    unsigned int x, unsigned int y)
{
    stroke_set_reserve_points(set, set->point_count + 1U);
    set->points[set->point_count].x = (uint16_t)x;
    set->points[set->point_count].y = (uint16_t)y;
    ++set->point_count;
    ++set->strokes[stroke_index].length;
}

static int
thinning_transitions(const unsigned char *pixels, size_t x, size_t y)
{
    int q[8];
    int i;
    int transitions;

    q[0] = pixels[(y - 1U) * TARGET_SIZE + x] != 0U;
    q[1] = pixels[(y - 1U) * TARGET_SIZE + x + 1U] != 0U;
    q[2] = pixels[y * TARGET_SIZE + x + 1U] != 0U;
    q[3] = pixels[(y + 1U) * TARGET_SIZE + x + 1U] != 0U;
    q[4] = pixels[(y + 1U) * TARGET_SIZE + x] != 0U;
    q[5] = pixels[(y + 1U) * TARGET_SIZE + x - 1U] != 0U;
    q[6] = pixels[y * TARGET_SIZE + x - 1U] != 0U;
    q[7] = pixels[(y - 1U) * TARGET_SIZE + x - 1U] != 0U;
    transitions = 0;
    for (i = 0; i < 8; ++i)
        if (!q[i] && q[(i + 1) & 7])
            ++transitions;
    return transitions;
}

static int
thinning_neighbors(const unsigned char *pixels, size_t x, size_t y)
{
    int dx;
    int dy;
    int neighbors;

    neighbors = 0;
    for (dy = -1; dy <= 1; ++dy) {
        for (dx = -1; dx <= 1; ++dx) {
            if ((dx != 0 || dy != 0) &&
                    pixels[(size_t)((int)y + dy) * TARGET_SIZE +
                    (size_t)((int)x + dx)] != 0U)
                ++neighbors;
        }
    }
    return neighbors;
}

static unsigned char *
thin_bitmap(const unsigned char *bitmap, size_t *pixel_countp)
{
    unsigned char *mark;
    unsigned char *pixels;
    size_t bytes;
    size_t i;
    size_t x;
    size_t y;
    int changed;
    int phase;

    bytes = TARGET_SIZE * TARGET_SIZE;
    pixels = malloc(bytes);
    mark = calloc(bytes, 1U);
    if (pixels == NULL || mark == NULL) {
        free(mark);
        free(pixels);
        die("cannot allocate stroke thinning workspace");
    }
    for (i = 0U; i < bytes; ++i)
        pixels[i] = bitmap[i] != 0U;

    do {
        changed = 0;
        for (phase = 0; phase < 2; ++phase) {
            memset(mark, 0, bytes);
            for (y = 1U; y + 1U < TARGET_SIZE; ++y) {
                for (x = 1U; x + 1U < TARGET_SIZE; ++x) {
                    int a;
                    int n;
                    int p2;
                    int p4;
                    int p6;
                    int p8;

                    if (pixels[y * TARGET_SIZE + x] == 0U)
                        continue;
                    n = thinning_neighbors(pixels, x, y);
                    a = thinning_transitions(pixels, x, y);
                    if (n < 2 || n > 6 || a != 1)
                        continue;
                    p2 = pixels[(y - 1U) * TARGET_SIZE + x] != 0U;
                    p4 = pixels[y * TARGET_SIZE + x + 1U] != 0U;
                    p6 = pixels[(y + 1U) * TARGET_SIZE + x] != 0U;
                    p8 = pixels[y * TARGET_SIZE + x - 1U] != 0U;
                    if (phase == 0) {
                        if (!(p2 && p4 && p6) && !(p4 && p6 && p8))
                            mark[y * TARGET_SIZE + x] = 1U;
                    } else {
                        if (!(p2 && p4 && p8) && !(p2 && p6 && p8))
                            mark[y * TARGET_SIZE + x] = 1U;
                    }
                }
            }
            for (i = 0U; i < bytes; ++i) {
                if (mark[i] != 0U) {
                    pixels[i] = 0U;
                    changed = 1;
                }
            }
        }
    } while (changed);

    *pixel_countp = 0U;
    for (i = 0U; i < bytes; ++i)
        *pixel_countp += pixels[i] != 0U;
    free(mark);
    return pixels;
}

static const int stroke_dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int stroke_dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

static int
stroke_pixel(const unsigned char *bitmap, int x, int y)
{
    if (x < 0 || x >= (int)TARGET_SIZE || y < 0 || y >= (int)TARGET_SIZE)
        return 0;
    return bitmap[(size_t)y * TARGET_SIZE + (size_t)x] != 0U;
}

static int
stroke_edge_exists(const unsigned char *bitmap, int x, int y, int direction)
{
    int dx;
    int dy;
    int nx;
    int ny;

    dx = stroke_dx[direction];
    dy = stroke_dy[direction];
    nx = x + dx;
    ny = y + dy;
    if (!stroke_pixel(bitmap, nx, ny))
        return 0;
    if (dx != 0 && dy != 0 &&
            (stroke_pixel(bitmap, x + dx, y) ||
             stroke_pixel(bitmap, x, y + dy)))
        return 0;
    return 1;
}

static int
stroke_degree(const unsigned char *bitmap, int x, int y)
{
    int degree;
    int direction;

    degree = 0;
    for (direction = 0; direction < 8; ++direction)
        degree += stroke_edge_exists(bitmap, x, y, direction);
    return degree;
}

static int
stroke_first_unvisited_direction(const unsigned char *bitmap,
    const unsigned char *visited, int x, int y)
{
    size_t index;
    int direction;

    index = (size_t)y * TARGET_SIZE + (size_t)x;
    for (direction = 0; direction < 8; ++direction) {
        if (stroke_edge_exists(bitmap, x, y, direction) &&
                (visited[index] & (1U << direction)) == 0U)
            return direction;
    }
    return -1;
}

static void
stroke_mark_edge(unsigned char *visited, int x, int y, int direction)
{
    int nx;
    int ny;

    nx = x + stroke_dx[direction];
    ny = y + stroke_dy[direction];
    visited[(size_t)y * TARGET_SIZE + (size_t)x] |=
        (unsigned char)(1U << direction);
    visited[(size_t)ny * TARGET_SIZE + (size_t)nx] |=
        (unsigned char)(1U << ((direction + 4) & 7));
}

static void
trace_stroke(const unsigned char *skeleton, unsigned char *visited,
    struct stroke_set *set, int start_x, int start_y, int start_direction)
{
    size_t stroke_index;
    int direction;
    int x;
    int y;

    stroke_index = stroke_begin(set);
    x = start_x;
    y = start_y;
    stroke_add_point(set, stroke_index, (unsigned int)x, (unsigned int)y);
    direction = start_direction;
    for (;;) {
        stroke_mark_edge(visited, x, y, direction);
        x += stroke_dx[direction];
        y += stroke_dy[direction];
        stroke_add_point(set, stroke_index, (unsigned int)x, (unsigned int)y);
        if (stroke_degree(skeleton, x, y) != 2)
            break;
        direction = stroke_first_unvisited_direction(skeleton, visited, x, y);
        if (direction < 0)
            break;
    }
}

static int
trace_strokes(const unsigned char *skeleton, struct stroke_set *set)
{
    unsigned char *visited;
    size_t bytes;
    size_t index;
    int degree;
    int direction;
    int x;
    int y;

    bytes = TARGET_SIZE * TARGET_SIZE;
    visited = calloc(bytes, 1U);
    if (visited == NULL)
        die("cannot allocate stroke edge map");

    for (index = 0U; index < bytes; ++index) {
        if (skeleton[index] == 0U)
            continue;
        x = (int)(index % TARGET_SIZE);
        y = (int)(index / TARGET_SIZE);
        degree = stroke_degree(skeleton, x, y);
        if (degree == 0) {
            size_t stroke_index;

            stroke_index = stroke_begin(set);
            stroke_add_point(set, stroke_index, (unsigned int)x,
                (unsigned int)y);
            continue;
        }
        if (degree == 2)
            continue;
        while ((direction = stroke_first_unvisited_direction(skeleton,
                visited, x, y)) >= 0) {
            trace_stroke(skeleton, visited, set, x, y, direction);
            if (set->stroke_count > STROKE_MAX_COUNT) {
                free(visited);
                return 0;
            }
        }
    }

    for (index = 0U; index < bytes; ++index) {
        if (skeleton[index] == 0U)
            continue;
        x = (int)(index % TARGET_SIZE);
        y = (int)(index / TARGET_SIZE);
        while ((direction = stroke_first_unvisited_direction(skeleton,
                visited, x, y)) >= 0) {
            trace_stroke(skeleton, visited, set, x, y, direction);
            if (set->stroke_count > STROKE_MAX_COUNT) {
                free(visited);
                return 0;
            }
        }
    }
    free(visited);
    return 1;
}

static double
stroke_point_segment_distance_squared(const struct stroke_point *point,
    const struct stroke_point *a, const struct stroke_point *b)
{
    double dx;
    double dy;
    double px;
    double py;
    double t;
    double x;
    double y;

    dx = (double)b->x - (double)a->x;
    dy = (double)b->y - (double)a->y;
    if (dx == 0.0 && dy == 0.0) {
        dx = (double)point->x - (double)a->x;
        dy = (double)point->y - (double)a->y;
        return dx * dx + dy * dy;
    }
    px = (double)point->x - (double)a->x;
    py = (double)point->y - (double)a->y;
    t = (px * dx + py * dy) / (dx * dx + dy * dy);
    if (t < 0.0)
        t = 0.0;
    else if (t > 1.0)
        t = 1.0;
    x = (double)a->x + t * dx;
    y = (double)a->y + t * dy;
    dx = (double)point->x - x;
    dy = (double)point->y - y;
    return dx * dx + dy * dy;
}

static void
simplify_one_stroke(const struct stroke_set *source, size_t stroke_index,
    struct stroke_set *dest)
{
    const struct stroke *stroke;
    const struct stroke_point *points;
    unsigned char *keep;
    size_t *stack;
    size_t a;
    size_t b;
    size_t i;
    size_t split;
    size_t stack_count;
    size_t dest_index;

    stroke = &source->strokes[stroke_index];
    points = source->points + stroke->offset;
    dest_index = stroke_begin(dest);
    if (stroke->length <= 2U) {
        for (i = 0U; i < stroke->length; ++i)
            stroke_add_point(dest, dest_index, points[i].x, points[i].y);
        return;
    }

    keep = calloc(stroke->length, 1U);
    stack = malloc(2U * stroke->length * sizeof(*stack));
    if (keep == NULL || stack == NULL) {
        free(stack);
        free(keep);
        die("cannot allocate stroke simplification workspace");
    }
    keep[0] = 1U;
    keep[stroke->length - 1U] = 1U;
    stack_count = 0U;
    stack[stack_count++] = 0U;
    stack[stack_count++] = stroke->length - 1U;
    while (stack_count != 0U) {
        double max_distance;
        int dx;
        int dy;

        b = stack[--stack_count];
        a = stack[--stack_count];
        dx = (int)points[b].x - (int)points[a].x;
        dy = (int)points[b].y - (int)points[a].y;
        if (dx < 0)
            dx = -dx;
        if (dy < 0)
            dy = -dy;
        split = 0U;
        max_distance = -1.0;
        if (dx > (int)VECTOR_MAX_DELTA || dy > (int)VECTOR_MAX_DELTA) {
            split = a + (b - a) / 2U;
        } else {
            for (i = a + 1U; i < b; ++i) {
                double distance;

                distance = stroke_point_segment_distance_squared(&points[i],
                    &points[a], &points[b]);
                if (distance > max_distance) {
                    max_distance = distance;
                    split = i;
                }
            }
            if (max_distance <= STROKE_RDP_TOLERANCE_SQUARED)
                split = 0U;
        }
        if (split != 0U) {
            keep[split] = 1U;
            stack[stack_count++] = a;
            stack[stack_count++] = split;
            stack[stack_count++] = split;
            stack[stack_count++] = b;
        }
    }
    for (i = 0U; i < stroke->length; ++i)
        if (keep[i] != 0U)
            stroke_add_point(dest, dest_index, points[i].x, points[i].y);
    free(stack);
    free(keep);
}

static void
simplify_strokes(const struct stroke_set *source, struct stroke_set *dest)
{
    size_t i;

    for (i = 0U; i < source->stroke_count; ++i)
        simplify_one_stroke(source, i, dest);
}

static void
stroke_raster_line(unsigned char *bitmap, int x0, int y0, int x1, int y1)
{
    int dx;
    int dy;
    int error;
    int error2;
    int sx;
    int sy;

    dx = x1 > x0 ? x1 - x0 : x0 - x1;
    sx = x0 < x1 ? 1 : -1;
    dy = y1 > y0 ? y0 - y1 : y1 - y0;
    sy = y0 < y1 ? 1 : -1;
    error = dx + dy;
    for (;;) {
        bitmap[(size_t)y0 * TARGET_SIZE + (size_t)x0] = 1U;
        if (x0 == x1 && y0 == y1)
            break;
        error2 = 2 * error;
        if (error2 >= dy) {
            error += dy;
            x0 += sx;
        }
        if (error2 <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

static unsigned char *
render_strokes(const struct stroke_set *set)
{
    unsigned char *bitmap;
    size_t i;
    size_t j;

    bitmap = calloc(TARGET_SIZE * TARGET_SIZE, 1U);
    if (bitmap == NULL)
        die("cannot allocate stroke validation raster");
    for (i = 0U; i < set->stroke_count; ++i) {
        const struct stroke *stroke;
        const struct stroke_point *points;

        stroke = &set->strokes[i];
        points = set->points + stroke->offset;
        if (stroke->length == 0U)
            continue;
        if (stroke->length == 1U) {
            bitmap[(size_t)points[0].y * TARGET_SIZE + points[0].x] = 1U;
            continue;
        }
        for (j = 1U; j < stroke->length; ++j)
            stroke_raster_line(bitmap, points[j - 1U].x, points[j - 1U].y,
                points[j].x, points[j].y);
    }
    return bitmap;
}

static int
bitmap_near(const unsigned char *bitmap, size_t x, size_t y,
    unsigned int radius)
{
    size_t x0;
    size_t x1;
    size_t xx;
    size_t y0;
    size_t y1;
    size_t yy;

    x0 = x > radius ? x - radius : 0U;
    y0 = y > radius ? y - radius : 0U;
    x1 = x + radius < TARGET_SIZE ? x + radius : TARGET_SIZE - 1U;
    y1 = y + radius < TARGET_SIZE ? y + radius : TARGET_SIZE - 1U;
    for (yy = y0; yy <= y1; ++yy)
        for (xx = x0; xx <= x1; ++xx)
            if (bitmap[yy * TARGET_SIZE + xx] != 0U)
                return 1;
    return 0;
}

static int
stroke_candidate_is_safe(const unsigned char *source,
    const unsigned char *candidate)
{
    size_t covered;
    size_t lit;
    size_t x;
    size_t y;

    lit = 0U;
    covered = 0U;
    for (y = 0U; y < TARGET_SIZE; ++y) {
        for (x = 0U; x < TARGET_SIZE; ++x) {
            if (candidate[y * TARGET_SIZE + x] != 0U &&
                    !bitmap_near(source, x, y, STROKE_OUTPUT_RADIUS))
                return 0;
            if (source[y * TARGET_SIZE + x] == 0U)
                continue;
            ++lit;
            if (bitmap_near(candidate, x, y, STROKE_COVERAGE_RADIUS))
                ++covered;
            else if (!bitmap_near(candidate, x, y,
                    STROKE_MAX_SOURCE_RADIUS))
                return 0;
        }
    }
    if (lit == 0U)
        return 0;
    return covered * 1000U >= lit * STROKE_MIN_COVERAGE_PERMILLE;
}

static void
order_strokes(const struct stroke_set *set, size_t *order,
    unsigned char *reverse)
{
    unsigned char *used;
    size_t chosen;
    size_t i;
    size_t n;
    int cur_x;
    int cur_y;

    used = calloc(set->stroke_count, 1U);
    if (used == NULL)
        die("cannot allocate stroke ordering workspace");
    cur_x = 0;
    cur_y = 0;
    for (n = 0U; n < set->stroke_count; ++n) {
        int best_distance;
        int best_reverse;
        int have_best;

        chosen = 0U;
        best_distance = 0;
        best_reverse = 0;
        have_best = 0;
        for (i = 0U; i < set->stroke_count; ++i) {
            const struct stroke *stroke;
            const struct stroke_point *a;
            const struct stroke_point *b;
            int da;
            int db;
            int use_reverse;
            int distance;

            if (used[i] != 0U)
                continue;
            stroke = &set->strokes[i];
            a = &set->points[stroke->offset];
            b = &set->points[stroke->offset + stroke->length - 1U];
            da = (int)a->x - cur_x;
            if (da < 0)
                da = -da;
            distance = (int)a->y - cur_y;
            if (distance < 0)
                distance = -distance;
            if (distance > da)
                da = distance;
            db = (int)b->x - cur_x;
            if (db < 0)
                db = -db;
            distance = (int)b->y - cur_y;
            if (distance < 0)
                distance = -distance;
            if (distance > db)
                db = distance;
            use_reverse = db < da;
            distance = use_reverse ? db : da;
            if (!have_best || distance < best_distance) {
                chosen = i;
                best_distance = distance;
                best_reverse = use_reverse;
                have_best = 1;
            }
        }
        used[chosen] = 1U;
        order[n] = chosen;
        reverse[n] = (unsigned char)best_reverse;
        {
            const struct stroke *stroke;
            const struct stroke_point *end;

            stroke = &set->strokes[chosen];
            if (best_reverse)
                end = &set->points[stroke->offset];
            else
                end = &set->points[stroke->offset + stroke->length - 1U];
            cur_x = end->x;
            cur_y = end->y;
        }
    }
    free(used);
}

static size_t
stroke_move_segments(int dx, int dy)
{
    unsigned int ax;
    unsigned int ay;
    size_t sx;
    size_t sy;

    ax = (unsigned int)(dx < 0 ? -dx : dx);
    ay = (unsigned int)(dy < 0 ? -dy : dy);
    sx = (ax + VECTOR_MAX_DELTA - 1U) / VECTOR_MAX_DELTA;
    sy = (ay + VECTOR_MAX_DELTA - 1U) / VECTOR_MAX_DELTA;
    return sx > sy ? sx : sy;
}

static int
emit_stroke_candidate(struct word_writer *writer, const unsigned char *bitmap,
    struct convert_stats *stats, struct stroke_plan *plan)
{
    struct stroke_set raw;
    struct stroke_set simplified;
    unsigned char *candidate;
    unsigned char *reverse;
    unsigned char *skeleton;
    size_t *order;
    size_t i;
    int cur_x;
    int cur_y;

    memset(plan, 0, sizeof(*plan));
    stroke_set_init(&raw);
    stroke_set_init(&simplified);
    skeleton = thin_bitmap(bitmap, &plan->skeleton_pixels);
    if (!trace_strokes(skeleton, &raw)) {
        free(skeleton);
        stroke_set_free(&raw);
        return 0;
    }
    simplify_strokes(&raw, &simplified);
    candidate = render_strokes(&simplified);
    if (!stroke_candidate_is_safe(bitmap, candidate)) {
        free(candidate);
        free(skeleton);
        stroke_set_free(&simplified);
        stroke_set_free(&raw);
        return 0;
    }
    free(candidate);
    free(skeleton);

    order = malloc(simplified.stroke_count * sizeof(*order));
    reverse = malloc(simplified.stroke_count);
    if (order == NULL || reverse == NULL) {
        free(reverse);
        free(order);
        stroke_set_free(&simplified);
        stroke_set_free(&raw);
        die("cannot allocate stroke order");
    }
    order_strokes(&simplified, order, reverse);

    writer_put_half(writer, ty340_param(MODE_VECTOR, 1, INTENSITY_MAX));
    plan->usec = 3.0;
    cur_x = 0;
    cur_y = 0;
    for (i = 0U; i < simplified.stroke_count; ++i) {
        const struct stroke *stroke;
        const struct stroke_point *points;
        size_t j;
        int start_x;
        int start_y;
        int dx;
        int dy;

        stroke = &simplified.strokes[order[i]];
        points = simplified.points + stroke->offset;
        if (reverse[i] != 0U) {
            start_x = points[stroke->length - 1U].x;
            start_y = points[stroke->length - 1U].y;
        } else {
            start_x = points[0].x;
            start_y = points[0].y;
        }
        dx = start_x - cur_x;
        dy = start_y - cur_y;
        plan->usec += 3.0 * (double)stroke_move_segments(dx, dy) +
            1.5 * (double)((abs(dx) > abs(dy)) ? abs(dx) : abs(dy));
        emit_vector_move(writer, dx, dy, stats);
        if (stroke->length == 1U) {
            writer_put_half(writer, ty340_vector(
                i + 1U == simplified.stroke_count, 1, 0, 0U, 0, 0U));
            ++stats->draw_segments;
            plan->usec += 3.0;
            cur_x = start_x;
            cur_y = start_y;
            continue;
        }
        for (j = 1U; j < stroke->length; ++j) {
            const struct stroke_point *a;
            const struct stroke_point *b;
            int final;
            unsigned int ax;
            unsigned int ay;

            if (reverse[i] != 0U) {
                a = &points[stroke->length - j];
                b = &points[stroke->length - j - 1U];
            } else {
                a = &points[j - 1U];
                b = &points[j];
            }
            dx = (int)b->x - (int)a->x;
            dy = (int)b->y - (int)a->y;
            ax = (unsigned int)(dx < 0 ? -dx : dx);
            ay = (unsigned int)(dy < 0 ? -dy : dy);
            final = i + 1U == simplified.stroke_count &&
                j + 1U == stroke->length;
            writer_put_half(writer, ty340_vector(final, 1, dy < 0, ay,
                dx < 0, ax));
            ++stats->draw_segments;
            plan->usec += 3.0 +
                1.5 * (double)(ax > ay ? ax : ay);
        }
        if (reverse[i] != 0U) {
            cur_x = points[0].x;
            cur_y = points[0].y;
        } else {
            cur_x = points[stroke->length - 1U].x;
            cur_y = points[stroke->length - 1U].y;
        }
    }
    plan->suitable = 1;
    plan->strokes = simplified.stroke_count;

    free(reverse);
    free(order);
    stroke_set_free(&simplified);
    stroke_set_free(&raw);
    return 1;
}

static struct stroke_plan
measure_stroke(const unsigned char *bitmap)
{
    struct convert_stats stats;
    struct stroke_plan plan;
    struct word_writer writer;

    memset(&stats, 0, sizeof(stats));
    memset(&writer, 0, sizeof(writer));
    if (!emit_stroke_candidate(&writer, bitmap, &stats, &plan))
        return plan;
    writer_finish(&writer);
    plan.halfwords = writer.halfwords;
    plan.words = writer.words;
    return plan;
}

static void
convert_type340(const char *inpath, const char *outpath)
{
    MagickWand *wand;
    struct word_writer writer;
    struct convert_stats stats;
    struct scan_plan best;
    struct scan_plan candidate;
    struct incremental_plan incremental;
    struct stroke_plan stroke;
    struct stroke_plan emitted_stroke;
    unsigned char *bitmap;
    int use_incremental;
    int use_stroke;
    size_t horizontal_runs;
    size_t vertical_runs;
    size_t lit_pixels;

    memset(&stats, 0, sizeof(stats));
    wand = load_image(inpath);
    prepare_bilevel(wand, inpath);
    bitmap = read_bitmap(wand, inpath, &lit_pixels);
    horizontal_runs = count_runs(bitmap, SCAN_HORIZONTAL);
    vertical_runs = count_runs(bitmap, SCAN_VERTICAL);

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
        incremental = measure_incremental(bitmap);
        use_incremental = incremental.words < best.words ||
            (incremental.words == best.words &&
             incremental.halfwords < best.halfwords);
        stroke = measure_stroke(bitmap);
        use_stroke = stroke.suitable &&
            stroke.words < (use_incremental ? incremental.words : best.words);

        writer_open(&writer, outpath);
        stats.lit_pixels = lit_pixels;
        if (use_stroke) {
            if (!emit_stroke_candidate(&writer, bitmap, &stats,
                    &emitted_stroke))
                die("internal stroke candidate changed during emission");
        } else if (use_incremental) {
            emit_incremental(&writer, bitmap, &stats);
        } else {
            stats.runs = best.runs;
            emit_scan(&writer, bitmap, best.axis, best.reverse_first,
                best.runs, &stats);
        }
    } else {
        use_incremental = 0;
        use_stroke = 0;
        best.axis = SCAN_HORIZONTAL;
        best.reverse_first = 0;
        best.runs = 0U;
        memset(&incremental, 0, sizeof(incremental));
        memset(&stroke, 0, sizeof(stroke));
        memset(&emitted_stroke, 0, sizeof(emitted_stroke));
        writer_open(&writer, outpath);
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
        "encoding=%s, scan=%s%s, lit pixels=%zu, runs=%zu, "
        "move segments=%zu, draw segments=%zu, incremental paths=%zu, "
        "incremental steps=%zu, stroke pixels=%zu, strokes=%zu, "
        "estimated type340=%.3f ms, halfwords=%zu, words=%zu\n",
        inpath, outpath, TARGET_SIZE, TARGET_SIZE,
        use_stroke ? "strokes" :
            (use_incremental ? "incremental" : "vector-runs"),
        best.axis == SCAN_HORIZONTAL ? "horizontal" : "vertical",
        best.reverse_first ? "-reverse" : "",
        stats.lit_pixels, stats.runs, stats.move_segments,
        stats.draw_segments, stats.incremental_paths,
        stats.incremental_steps,
        use_stroke ? emitted_stroke.skeleton_pixels : 0U,
        use_stroke ? emitted_stroke.strokes : 0U,
        use_stroke ? emitted_stroke.usec / 1000.0 : 0.0,
        writer.halfwords, writer.words);
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
