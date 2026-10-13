/* D6 grouped 36-bit word codec, host library v060.
 * Encoder token selection matches native PDP-6 v053; decoder rejects
 * noncanonical groups and validates exact source/output consumption.
 * All allocation occurs on the host, never in the PDP-6 kernel.
 */
#include "d6codec-codec-v060.h"

#include <stdint.h>
#include <stdlib.h>

#define D6C_WORD_MASK UINT64_C(0777777777777)
#define D6C_HALF_MASK UINT64_C(0777777)
#define D6C_TOKEN_MAX 30U
#define D6C_RUN_MAX 127U
#define D6C_DISTANCE_MAX 128U

int
d6codec_encode_v060(const uint64_t *source, size_t source_words,
    uint64_t **outp, size_t *out_words)
{
    uint64_t *output;
    size_t capacity, inpos, outpos;

    if (outp == NULL || out_words == NULL ||
        (source_words != 0U && source == NULL))
        return -1;
    *outp = NULL;
    *out_words = 0U;
    if (source_words > SIZE_MAX - 29U)
        return -1;
    capacity = source_words + (source_words + 29U) / 30U;
    if (capacity > SIZE_MAX / sizeof(*output))
        return -1;
    output = capacity != 0U ?
        (uint64_t *)malloc(capacity * sizeof(*output)) : NULL;
    if (capacity != 0U && output == NULL)
        return -1;

    {
        size_t i;
        for (i = 0U; i < source_words; ++i)
            if ((source[i] & ~D6C_WORD_MASK) != 0U)
                goto failure;
    }
    inpos = 0U;
    outpos = 0U;
    while (inpos < source_words) {
        uint64_t descriptors[15], literals[30], flags;
        unsigned int count, matches, literal_count;
        size_t group_header;

        group_header = outpos++;
        flags = 0U;
        count = matches = literal_count = 0U;
        while (count < D6C_TOKEN_MAX && inpos < source_words) {
            size_t limit = source_words - inpos;
            size_t length = 0U, distance = 0U;
            unsigned int kind = 0U;

            if ((source[inpos] & ~D6C_WORD_MASK) != 0U)
                goto failure;
            if (limit > D6C_RUN_MAX)
                limit = D6C_RUN_MAX;
            if (source[inpos] == 0U) {
                length = 1U;
                while (length < limit && source[inpos + length] == 0U)
                    ++length;
                kind = 1U;
            } else {
                size_t candidate_distance = inpos;
                if (candidate_distance > D6C_DISTANCE_MAX)
                    candidate_distance = D6C_DISTANCE_MAX;
                for (; candidate_distance != 0U; --candidate_distance) {
                    size_t candidate_length;
                    if (source[inpos] != source[inpos - candidate_distance])
                        continue;
                    candidate_length = 1U;
                    while (candidate_length < limit &&
                        source[inpos + candidate_length] ==
                        source[inpos + candidate_length - candidate_distance])
                        ++candidate_length;
                    if (candidate_length > length) {
                        length = candidate_length;
                        distance = candidate_distance;
                        if (length == limit)
                            break;
                    }
                }
                if (length < 2U)
                    length = 0U;
            }
            if (length != 0U) {
                uint64_t descriptor;
                flags |= UINT64_C(1) << (29U - count);
                descriptor = ((uint64_t)kind << 16U) |
                    ((uint64_t)length << 9U) | (uint64_t)distance;
                if (descriptor > D6C_HALF_MASK)
                    goto failure;
                descriptors[matches++] = descriptor;
                inpos += length;
            } else {
                literals[literal_count++] = source[inpos++];
            }
            ++count;
        }
        output[group_header] = ((uint64_t)count << 30U) | flags;
        {
            unsigned int i;
            for (i = 0U; i < matches; i += 2U) {
                uint64_t packed = descriptors[i] << 18U;
                if (i + 1U < matches)
                    packed |= descriptors[i + 1U];
                output[outpos++] = packed;
            }
            for (i = 0U; i < literal_count; ++i)
                output[outpos++] = literals[i];
        }
        if (outpos > capacity)
            goto failure;
    }
    *outp = output;
    *out_words = outpos;
    return 0;
failure:
    free(output);
    return -1;
}

int
d6codec_decode_v060(const uint64_t *source, size_t compressed_words,
    uint64_t *output, size_t output_words)
{
    size_t inpos = 0U, outpos = 0U;

    if ((compressed_words != 0U && source == NULL) ||
        (output_words != 0U && output == NULL))
        return -1;
    while (outpos < output_words) {
        uint64_t control, flags, allowed_flags;
        size_t descriptor_words, literal_words, descriptors, literal_at;
        size_t descriptor_at;
        unsigned int count, matches, index;

        if (inpos >= compressed_words)
            return -1;
        control = source[inpos++];
        if ((control & ~D6C_WORD_MASK) != 0U)
            return -1;
        count = (unsigned int)(control >> 30U);
        if (count == 0U || count > D6C_TOKEN_MAX)
            return -1;
        flags = control & ((UINT64_C(1) << 30U) - 1U);
        allowed_flags = ((UINT64_C(1) << count) - 1U) << (30U - count);
        if ((flags & ~allowed_flags) != 0U)
            return -1;
        matches = 0U;
        for (index = 0U; index < count; ++index)
            if ((flags & (UINT64_C(1) << (29U - index))) != 0U)
                ++matches;
        descriptor_words = (matches + 1U) / 2U;
        literal_words = count - matches;
        if (descriptor_words + literal_words > compressed_words - inpos)
            return -1;
        descriptor_at = inpos;
        literal_at = inpos + descriptor_words;
        inpos += descriptor_words + literal_words;
        if (matches != 0U && (matches & 1U) != 0U &&
            (source[literal_at - 1U] & D6C_HALF_MASK) != 0U)
            return -1;
        descriptors = 0U;
        for (index = 0U; index < count; ++index) {
            uint64_t descriptor;
            unsigned int kind;
            size_t length, distance, j;

            if ((flags & (UINT64_C(1) << (29U - index))) == 0U) {
                uint64_t literal = source[literal_at++];
                if (outpos >= output_words ||
                    (literal & ~D6C_WORD_MASK) != 0U)
                    return -1;
                output[outpos++] = literal;
                continue;
            }
            descriptor = source[descriptor_at + descriptors / 2U];
            if ((descriptor & ~D6C_WORD_MASK) != 0U)
                return -1;
            descriptor = (descriptors & 1U) != 0U ?
                (descriptor & D6C_HALF_MASK) :
                ((descriptor >> 18U) & D6C_HALF_MASK);
            ++descriptors;
            kind = (unsigned int)((descriptor >> 16U) & 3U);
            length = (size_t)((descriptor >> 9U) & 127U);
            distance = (size_t)(descriptor & 511U);
            if (length == 0U || length > output_words - outpos)
                return -1;
            if (kind == 0U) {
                if (distance == 0U || distance > outpos)
                    return -1;
                for (j = 0U; j < length; ++j) {
                    output[outpos] = output[outpos - distance];
                    ++outpos;
                }
            } else if (kind == 1U) {
                if (distance != 0U)
                    return -1;
                for (j = 0U; j < length; ++j)
                    output[outpos++] = 0U;
            } else if (kind == 2U) {
                if (distance == 0U || (distance + 1U) / 2U > outpos)
                    return -1;
                for (j = 0U; j < 2U * length; ++j) {
                    size_t half = 2U * outpos + j - distance;
                    uint64_t value = output[half / 2U];
                    value >>= (half & 1U) != 0U ? 0U : 18U;
                    value &= D6C_HALF_MASK;
                    if ((j & 1U) == 0U)
                        output[outpos + j / 2U] = value << 18U;
                    else
                        output[outpos + j / 2U] |= value;
                }
                outpos += length;
            } else {
                return -1;
            }
        }
    }
    return inpos == compressed_words ? 0 : -1;
}
