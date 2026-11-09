/**
 * @file kpack_ram_rle_v1376.c
 * @brief Bounded 36-bit-word run codec for swapless mutable process state.
 *
 * This deliberately favors low scratch RAM and simple bounded loops over
 * maximal compression ratio. The output is a sequence of 36-bit control
 * words with a two-bit kind (bits 35..34) and an 18-bit count (bits 17..0):
 *
 *   kind 0: zero run, no payload
 *   kind 1: repeated nonzero word, one payload word
 *   kind 2: literal run, COUNT payload words
 *   kind 3: invalid
 *
 * Control bits 33..18 must be zero. Count is 1..0777777 octal.
 * The stream is valid only if it consumes exactly ENCODED_WORDS and
 * reconstructs exactly DECODED_WORDS. There is no persistent state.
 *
 * Neither routine is linked into production KCORE. Opt-in eligibility,
 * ownership, PURE prefix boundaries, transactional MM publication and
 * restoration headroom are the caller's responsibility.
 */
#include "kpack_ram_rle_v1376.h"

#define KPACK_RAM_COUNT_V1376 0777777UL
#define KPACK_RAM_WORD_V1376 0777777777777UL
#define KPACK_RAM_REPEAT_V1376 0200000000000UL
#define KPACK_RAM_LITERAL_V1376 0400000000000UL
#define KPACK_RAM_KIND_V1376 0600000000000UL
#define KPACK_RAM_CONTROL_V1376 (KPACK_RAM_KIND_V1376 | KPACK_RAM_COUNT_V1376)

/* Cap a run at a PDP-6 halfword count; never access past SOURCE. */
static unsigned int
kpack_ram_run_v1376(const kword_t *source, unsigned int words,
    unsigned int offset)
{
        unsigned int length = 1U;
        kword_t value = source[offset];

        while (length < KPACK_RAM_COUNT_V1376 &&
            offset + length < words &&
            source[offset + length] == value)
                ++length;
        return length;
}

static int
kpack_ram_scan_v1376(const kword_t *source, unsigned int words,
    kword_t *destination, unsigned int capacity, int emit)
{
        unsigned int index = 0U;
        unsigned int used = 0U;
        unsigned int count;
        unsigned int start;
        unsigned int i;
        kword_t value;
        unsigned int payload;
        kword_t header;

        if ((words && !source) || (emit && !destination) ||
            words > KPACK_RAM_COUNT_V1376)
                return -1;

        while (index < words) {
                value = source[index];
                if (value > KPACK_RAM_WORD_V1376)
                        return -1;
                count = kpack_ram_run_v1376(source, words, index);
                if (value == 0UL && count >= 2U) {
                        header = (kword_t)count;
                        payload = 0U;
                } else if (value != 0UL && count >= 3U) {
                        header = KPACK_RAM_REPEAT_V1376 | (kword_t)count;
                        payload = 1U;
                } else {
                        start = index;
                        ++index;
                        while (index < words) {
                                value = source[index];
                                if (value > KPACK_RAM_WORD_V1376)
                                        return -1;
                                count = kpack_ram_run_v1376(source, words,
                                    index);
                                if ((value == 0UL && count >= 2U) ||
                                    (value != 0UL && count >= 3U))
                                        break;
                                ++index;
                        }
                        count = index - start;
                        header = KPACK_RAM_LITERAL_V1376 | (kword_t)count;
                        payload = count;
                        /* The longest literal input is a halfword count. */
                        if (used > KPACK_RAM_COUNT_V1376 - 1U ||
                            payload > KPACK_RAM_COUNT_V1376 - used - 1U)
                                return -1;
                        if (emit) {
                                if (used + 1U + payload > capacity)
                                        return -1;
                                destination[used] = header;
                                for (i = 0U; i < payload; ++i)
                                        destination[used + 1U + i] =
                                            source[start + i];
                        }
                        used += 1U + payload;
                        continue;
                }

                if (used > KPACK_RAM_COUNT_V1376 - 1U - payload)
                        return -1;
                if (emit) {
                        if (used + 1U + payload > capacity)
                                return -1;
                        destination[used] = header;
                        if (payload)
                                destination[used + 1U] = value;
                }
                used += 1U + payload;
                index += count;
        }
        return (int)used;
}

int
kpack_ram_measure_v1376(const kword_t *source, unsigned int decoded_words)
{
        return kpack_ram_scan_v1376(source, decoded_words, 0, 0U, 0);
}

int
kpack_ram_encode_v1376(const kword_t *source, unsigned int decoded_words,
    kword_t *destination, unsigned int capacity)
{
        if (capacity > KPACK_RAM_COUNT_V1376)
                return -1;
        return kpack_ram_scan_v1376(source, decoded_words, destination,
            capacity, 1);
}

int
kpack_ram_decode_v1376(const kword_t *source, unsigned int encoded_words,
    kword_t *destination, unsigned int decoded_words)
{
        unsigned int input = 0U;
        unsigned int output = 0U;
        unsigned int count;
        unsigned int i;
        kword_t control;
        kword_t kind;
        kword_t value;

        if ((encoded_words && !source) || (decoded_words && !destination) ||
            encoded_words > KPACK_RAM_COUNT_V1376 ||
            decoded_words > KPACK_RAM_COUNT_V1376)
                return -1;
        while (input < encoded_words) {
                control = source[input++];
                if ((control & ~((kword_t)KPACK_RAM_CONTROL_V1376)) != 0UL)
                        return -1;
                kind = control & KPACK_RAM_KIND_V1376;
                count = (unsigned int)(control & KPACK_RAM_COUNT_V1376);
                if (!count || count > decoded_words - output ||
                    kind == KPACK_RAM_KIND_V1376)
                        return -1;
                if (kind == KPACK_RAM_REPEAT_V1376) {
                        if (input >= encoded_words)
                                return -1;
                        value = source[input++];
                        if (!value || value > KPACK_RAM_WORD_V1376)
                                return -1;
                        for (i = 0U; i < count; ++i)
                                destination[output + i] = value;
                } else if (kind == KPACK_RAM_LITERAL_V1376) {
                        if (count > encoded_words - input)
                                return -1;
                        for (i = 0U; i < count; ++i) {
                                value = source[input + i];
                                if (value > KPACK_RAM_WORD_V1376)
                                        return -1;
                                destination[output + i] = value;
                        }
                        input += count;
                } else {
                        for (i = 0U; i < count; ++i)
                                destination[output + i] = 0UL;
                }
                output += count;
        }
        if (output != decoded_words)
                return -1;
        return (int)output;
}
