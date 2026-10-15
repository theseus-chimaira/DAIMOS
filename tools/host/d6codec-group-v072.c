/* Staged bounded C99 host encoder for grouped 36-bit streams.
 * Fast path emits literal, zero-run and ordinary word-match tokens.
 * The native decoder also understands halfword references, but they are
 * not needed to produce a valid stream. No heap or large temporary arrays.
 */
#include "d6codec-group-v071.h"
#include <stdint.h>
#define W36 UINT64_C(0777777777777)
#define CONTROL_FLAG UINT64_C(04000000000)

int
d6codec_group_v072_encode(const uint64_t *in, size_t nw,
    uint64_t *out, size_t cap, size_t *written)
{
    size_t pos = 0, op = 0;
    if (!written || (nw && !in) || (cap && !out)) return -1;
    *written = 0;
    while (pos < nw) {
        size_t group, literal_start, tokens = 0, matches = 0, end;
        uint64_t flags = 0;
        if (op >= cap) return -1;
        group = op++;
        literal_start = op;
        /* Reserve 15 descriptor slots. They are populated in place and
         * unused slots are removed via a final bounded move. */
        if (cap - op < 15) {
            /* A small output buffer can still hold a complete group, but
             * keeping one reservation rule makes errors atomic per group. */
            return -1;
        }
        op += 15;
        while (tokens < 30 && pos < nw) {
            size_t dist, bestdist = 0, bestlen = 0;
            size_t distance_limit = pos < 128 ? pos : 128;
            uint64_t descriptor = 0;
            size_t len = 0;
            if (in[pos] > W36) return -1;
            if (in[pos] == 0) {
                len = 1;
                while (len < 127 && len < nw - pos && in[pos + len] == 0)
                    ++len;
                descriptor = (UINT64_C(1) << 16) | ((uint64_t)len << 9);
            } else {
                for (dist = 1; dist <= distance_limit; ++dist) {
                    size_t run = 0;
                    while (run < 127 && run < nw - pos &&
                        in[pos + run] == in[pos + run - dist]) ++run;
                    if (run > bestlen) { bestlen = run; bestdist = dist; }
                }
                if (bestlen >= 2) {
                    len = bestlen;
                    descriptor = ((uint64_t)len << 9) | bestdist;
                }
            }
            if (len) {
                size_t slot = matches / 2;
                if (!(matches & 1)) out[group + 1 + slot] = descriptor << 18;
                else out[group + 1 + slot] |= descriptor;
                flags |= CONTROL_FLAG >> tokens;
                ++matches;
                pos += len;
            } else {
                if (op >= cap) return -1;
                out[op++] = in[pos++];
            }
            ++tokens;
        }
        end = group + 1 + (matches + 1) / 2;
        /* The reserved descriptor tail is deleted in-place. */
        for (literal_start = group + 16; literal_start < op; ++literal_start)
            out[end++] = out[literal_start];
        op = end;
        out[group] = ((uint64_t)tokens << 30) | flags;
    }
    *written = op;
    return 0;
}
