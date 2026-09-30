/**
 * @file d6lz.c
 * @brief Portable in-memory decoder for the D6LZ36 word-stream format.
 *
 * This file provides the architecture-independent decoder used when the
 * kernel core is built for a non-PDP-10 host.  PDP-6/PDP-10 kernels do not
 * compile this C decoder: their resident D6LZ36 path uses the fixed low-core
 * resumable token engine plus the allocation-free VFS frontend instead.  The
 * split keeps the production decoder small while retaining a straightforward
 * in-memory implementation of the same stream format for non-target builds.
 *
 * D6LZ36 operates on 36-bit words.  One control word describes up to 36
 * following tokens, from control bit 35 down to bit 0.  A clear control bit
 * selects a literal word.  A set bit selects a 14-bit match descriptor:
 * bits 0..6 hold distance-1, bits 7..13 hold length-3, and bits 14..35 must
 * be zero.  Match copying deliberately proceeds one word at a time so source
 * and destination may overlap, as required for repeated-pattern expansion.
 *
 * Decoding uses only caller-supplied source and destination storage and has
 * no allocation or persistent state.  The produced prefix of @p dst is the
 * history window.  A zero-length destination is a successful no-op, even
 * when the pointers are null.  For non-empty output both pointers must be
 * valid for the word counts supplied by the caller.
 */
#include "d6lz.h"

#ifndef __PDP10__
/* Match descriptor bits 0..6 encode distance-1. */
#define D6LZ36_DISTANCE_MASK    0177UL
/* Match descriptor bits 7..13 encode length-3. */
#define D6LZ36_LENGTH_MASK      0177UL
#define D6LZ36_LENGTH_SHIFT     7U
/* Bits 14..35 of a match descriptor are reserved and must remain zero. */
#define D6LZ36_DESCRIPTOR_MASK  037777UL
/* Control tokens are consumed from bit 35 down to bit 0. */
#define D6LZ36_CONTROL_TOP      (((kword_t)1UL) << 35)

/**
 * @brief Decode a complete requested D6LZ36 output prefix in memory.
 *
 * Control words and tokens are consumed sequentially from @p src until
 * exactly @p dst_words words have been produced.  Literal tokens are copied
 * directly.  Match tokens copy from already-produced output; copying is
 * forward and therefore preserves the overlap semantics needed when a match
 * length exceeds its distance.
 *
 * The function validates source exhaustion, reserved descriptor bits,
 * backward-match distance, and destination overrun.  It stops as soon as the
 * requested output length has been produced; trailing source words, if any,
 * are not rejected or consumed further.  This differs from callers that need
 * an exact compressed-payload check, which must verify source consumption at
 * their framing layer.
 *
 * @param dst Destination buffer for decompressed 36-bit words.
 * @param dst_words Exact number of words to produce.  Zero is a successful
 *        no-op and does not require valid source or destination pointers.
 * @param src Source buffer containing D6LZ36 control words and tokens.
 * @param src_words Number of source words available in @p src.
 * @return 0 after producing exactly @p dst_words words; -1 for malformed or
 *         truncated input, an invalid pointer for non-empty output, an
 *         impossible match distance, or a match that would overrun @p dst.
 *
 * @invariant At the start of each token, @c dp is the number of valid words
 *            already present in @p dst and therefore the complete match
 *            history available to the decoder.
 * @invariant A nonzero @c control_mask identifies exactly one bit of the
 *            current control word; shifting it right advances token order
 *            monotonically from bit 35 to bit 0.
 */
int
d6lz36_decode(kword_t *dst, unsigned int dst_words,
    const kword_t *src, unsigned int src_words)
{
        kword_t control;
        kword_t control_mask;
        kword_t token;
        unsigned int sp;
        unsigned int dp;
        unsigned int distance;
        unsigned int length;
        unsigned int i;

        if (dst_words == 0U)
                return 0;
        if (dst == 0 || src == 0)
                return -1;

        sp = 0U;
        dp = 0U;
        control = 0;
        control_mask = 0;
        while (dp < dst_words) {
                if (control_mask == 0) {
                        if (sp >= src_words)
                                return -1;
                        control = src[sp++];
                        control_mask = D6LZ36_CONTROL_TOP;
                }
                if (sp >= src_words)
                        return -1;
                token = src[sp++];
                if ((control & control_mask) == 0) {
                        dst[dp++] = token;
                } else {
                        if ((token & ~((kword_t)D6LZ36_DESCRIPTOR_MASK)) != 0)
                                return -1;
                        distance = (unsigned int)(token &
                            (kword_t)D6LZ36_DISTANCE_MASK) + 1U;
                        length = (unsigned int)((token >> D6LZ36_LENGTH_SHIFT) &
                            (kword_t)D6LZ36_LENGTH_MASK) + D6LZ36_MIN_MATCH;
                        if (distance > dp || length > dst_words - dp)
                                return -1;
                        for (i = 0U; i < length; ++i) {
                                dst[dp] = dst[dp - distance];
                                ++dp;
                        }
                }
                control_mask >>= 1;
        }
        return 0;
}
#endif
