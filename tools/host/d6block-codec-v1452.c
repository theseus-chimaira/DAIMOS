/*
 * Host-only block LZ/RLE producer and independent strict decoder v1452.
 * Byte-compatible with v1421/v1441 PDP-6 assembly.
 *
 * Coding policy: a greedy zero run >= 2, else the longest match
 * >= 3 among preceding distances 1..128, else a literal. The order
 * of match probes is descending so ties reproduce the existing
 * v1421 Python reference and v1429 native C reference.
 */
#include "d6block-codec-v1452.h"

#include <stdlib.h>

#define DB_WORD_MASK UINT64_C(0777777777777)
#define DB_LOW_MASK UINT64_C(0777777)
#define DB_MAX_RUN 128U
#define DB_WINDOW 128U

static uint64_t
db_command(unsigned int opcode, size_t length, size_t distance)
{
    return ((uint64_t)opcode << 34) |
        ((uint64_t)(length - 1U) << 27) |
        ((uint64_t)distance << 18);
}

int
d6block_compress_v1452(const uint64_t *source, size_t words,
    uint64_t **packed, size_t *packed_words)
{
    size_t capacity;
    size_t pos;
    size_t used;
    size_t literal_count;
    size_t literal_start;
    uint64_t *result;

    if (packed == NULL || packed_words == NULL ||
        (words != 0U && source == NULL) ||
        words > (((size_t)-1) - 2U) / 2U)
        return -1;
    capacity = words + (words + DB_MAX_RUN - 1U) / DB_MAX_RUN + 1U;
    result = (uint64_t *)malloc(capacity * sizeof(*result));
    if (result == NULL)
        return -1;
    pos = 0U;
    used = 0U;
    literal_count = 0U;
    literal_start = 0U;
    while (pos < words) {
        size_t maximum;
        size_t zeros;
        size_t best;
        size_t best_distance;

        if ((source[pos] & ~DB_WORD_MASK) != 0U)
            goto error;
        maximum = words - pos;
        if (maximum > DB_MAX_RUN)
            maximum = DB_MAX_RUN;
        zeros = 0U;
        if (source[pos] == 0U) {
            while (zeros < maximum && source[pos + zeros] == 0U)
                ++zeros;
        }
        best = 0U;
        best_distance = 0U;
        if (zeros < 2U) {
            size_t max_distance;
            size_t distance;

            max_distance = pos < DB_WINDOW ? pos : DB_WINDOW;
            for (distance = max_distance; distance > 0U; --distance) {
                size_t length;

                length = 0U;
                while (length < maximum &&
                    source[pos + length] ==
                    source[pos + length - distance])
                    ++length;
                if (length > best) {
                    best = length;
                    best_distance = distance;
                    if (best == DB_MAX_RUN)
                        break;
                }
            }
        }
        if (zeros >= 2U || best >= 3U) {
            size_t length;

            if (literal_count != 0U) {
                result[literal_start] =
                    db_command(0U, literal_count, 0U);
                literal_count = 0U;
            }
            if (used >= capacity)
                goto error;
            if (zeros >= 2U) {
                length = zeros;
                result[used++] = db_command(1U, length, 0U);
            } else {
                length = best;
                result[used++] = db_command(2U, length,
                    best_distance - 1U);
            }
            pos += length;
        } else {
            if (literal_count == 0U) {
                if (used >= capacity)
                    goto error;
                literal_start = used++;
            }
            if (used >= capacity)
                goto error;
            result[used++] = source[pos++];
            ++literal_count;
            if (literal_count == DB_MAX_RUN) {
                result[literal_start] =
                    db_command(0U, literal_count, 0U);
                literal_count = 0U;
            }
        }
    }
    if (literal_count != 0U)
        result[literal_start] = db_command(0U, literal_count, 0U);
    *packed = result;
    *packed_words = used;
    return 0;
error:
    free(result);
    return -1;
}

int
d6block_decompress_v1452(const uint64_t *packed, size_t packed_words,
    uint64_t *destination, size_t decoded_words)
{
    size_t inpos;
    size_t outpos;

    if ((packed_words != 0U && packed == NULL) ||
        (decoded_words != 0U && destination == NULL))
        return -1;
    inpos = 0U;
    outpos = 0U;
    while (outpos < decoded_words) {
        uint64_t cmd;
        unsigned int opcode;
        size_t length;
        size_t distance;
        size_t i;

        if (inpos >= packed_words)
            return -1;
        cmd = packed[inpos++];
        if ((cmd & ~DB_WORD_MASK) != 0U ||
            (cmd & DB_LOW_MASK) != 0U)
            return -1;
        opcode = (unsigned int)(cmd >> 34);
        length = (size_t)((cmd >> 27) & UINT64_C(0177)) + 1U;
        distance = (size_t)((cmd >> 18) & UINT64_C(0777));
        if (length > decoded_words - outpos)
            return -1;
        switch (opcode) {
        case 0U:
            if (distance != 0U || length > packed_words - inpos)
                return -1;
            for (i = 0U; i < length; ++i) {
                if ((packed[inpos] & ~DB_WORD_MASK) != 0U)
                    return -1;
                destination[outpos++] = packed[inpos++];
            }
            break;
        case 1U:
            if (distance != 0U)
                return -1;
            for (i = 0U; i < length; ++i)
                destination[outpos++] = 0U;
            break;
        case 2U:
            ++distance;
            if (distance > outpos)
                return -1;
            for (i = 0U; i < length; ++i) {
                destination[outpos] = destination[outpos - distance];
                ++outpos;
            }
            break;
        default:
            return -1;
        }
    }
    return inpos == packed_words ? 0 : -1;
}
