/* Experimental userspace grouped-codec file writer; not an active D6LZ
 * producer. Uses 76 static words, independent of executable format.
 * No allocation, no hidden I/O state. C99 with PDP-6 36-bit kword_t.
 */
#include "u.h"

#define GROUP_WORDS 30U
#define W36 0777777777777UL
#define GROUP_FIRST 04000000000UL

/* Input and output buffers never overlap; no extra descriptor workspace. */
static kword_t v075_input[GROUP_WORDS];
static kword_t v075_output[GROUP_WORDS + 16U];

int
d6codec_group_v075_compress_fd(int input_fd, int output_fd, kword_t words)
{
        while (words != 0) {
                unsigned int n, i, j, ntok, matches, literals;
                kword_t flags;
                int got;

                n = words > GROUP_WORDS ? GROUP_WORDS : (unsigned int)words;
                got = dsys_read_words(input_fd, v075_input, n);
                if (got != (int)n) return -1;
                /* Consume no more than 30 tokens; each match replaces at
                 * least one input word with a packed 18-bit descriptor. */
                i = 0U;
                ntok = 0U;
                matches = 0U;
                literals = 0U;
                flags = 0;
                while (i < n) {
                        unsigned int len, j, dist, best_len, best_dist;
                        kword_t descriptor;
                        if ((v075_input[i] & ~W36) != 0) return -1;
                        len = 0U;
                        descriptor = 0;
                        if (v075_input[i] == 0) {
                                len = 1U;
                                while (i + len < n &&
                                    v075_input[i + len] == 0) ++len;
                                descriptor = (1UL << 16) |
                                    ((kword_t)len << 9);
                        } else {
                                best_len = 0U;
                                best_dist = 0U;
                                for (dist = 1U; dist <= i; ++dist) {
                                        j = 0U;
                                        while (i + j < n &&
                                            v075_input[i + j] ==
                                            v075_input[i + j - dist]) ++j;
                                        if (j > best_len) {
                                                best_len = j;
                                                best_dist = dist;
                                        }
                                }
                                if (best_len >= 2U) {
                                        len = best_len;
                                        descriptor = ((kword_t)len << 9) |
                                            best_dist;
                                }
                        }
                        if (len != 0U) {
                                unsigned int slot = 1U + matches / 2U;
                                if ((matches & 1U) == 0U)
                                        v075_output[slot] = descriptor << 18;
                                else
                                        v075_output[slot] |= descriptor;
                                flags |= GROUP_FIRST >> ntok;
                                ++matches;
                                i += len;
                        } else {
                                /* Literals are stored temporarily after the
                                 * largest possible descriptor reservation. */
                                v075_output[16U + literals++] =
                                    v075_input[i++];
                        }
                        ++ntok;
                }
                {
                        unsigned int dwords = (matches + 1U) / 2U;
                        unsigned int output_n = 1U + dwords + literals;
                        for (j = 0U; j < literals; ++j)
                                v075_output[1U + dwords + j] =
                                    v075_output[16U + j];
                        v075_output[0] = ((kword_t)ntok << 30) | flags;
                        if (u_write_words_all(output_fd, v075_output,
                            output_n) != 0) return -1;
                }
                words -= n;
        }
        return 0;
}
