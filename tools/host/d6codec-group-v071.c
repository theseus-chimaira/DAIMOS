/* Host reference decoder for the staged PDP-6 grouped 36-bit codec.
 * C99, bounded input/output, and no dynamic storage. Not yet production.
 */
#include "d6codec-group-v071.h"
#include <stdint.h>
#define W36 UINT64_C(0777777777777)
#define H18 UINT64_C(0777777)

int
d6codec_group_v071_decode(const uint64_t *in, size_t nw,
    uint64_t *out, size_t ow)
{
    size_t ip = 0, op = 0;
    if ((nw && !in) || (ow && !out)) return -1;
    while (ip < nw) {
        uint64_t control, bitmap;
        size_t descriptors, literals, di, li, matches = 0;
        unsigned count, token, pair = 0;
        if (op == ow) return -1;
        control = in[ip++];
        if (control > W36) return -1;
        count = (unsigned)(control >> 30);
        if (count == 0 || count > 30) return -1;
        bitmap = (control << 6) & W36;
        /* The leading flags correspond to the count tokens. */
        for (token = 0; token < count; ++token) {
            if ((bitmap & (UINT64_C(1) << 35)) != 0) ++matches;
            bitmap = (bitmap << 1) & W36;
        }
        descriptors = (matches + 1) / 2;
        literals = count - matches;
        if (descriptors + literals > nw - ip) return -1;
        di = ip;
        li = ip + descriptors;
        ip += descriptors + literals;
        bitmap = (control << 6) & W36;
        for (token = 0; token < count; ++token) {
            if ((bitmap & (UINT64_C(1) << 35)) == 0) {
                if (op == ow || in[li] > W36) return -1;
                out[op++] = in[li++];
            } else {
                uint32_t descriptor;
                unsigned kind, len, distance;
                size_t j;
                uint64_t packed = in[di];
                if (packed > W36) return -1;
                descriptor = pair == 0 ?
                    (uint32_t)((packed >> 18) & H18) :
                    (uint32_t)(packed & H18);
                if (pair != 0) { ++di; pair = 0; }
                else pair = 1;
                kind = descriptor >> 16;
                len = (descriptor >> 9) & 127U;
                distance = descriptor & 511U;
                if (len == 0 || len > ow - op) return -1;
                if (kind == 1) {
                    if (distance != 0) return -1;
                    for (j = 0; j < len; ++j) out[op++] = 0;
                } else if (kind == 0 || kind == 2) {
                    size_t back;
                    if (distance == 0) return -1;
                    if (kind == 0 || (distance & 1U) == 0) {
                        back = kind == 0 ? distance : distance / 2;
                        if (back > op) return -1;
                        for (j = 0; j < len; ++j) {
                            out[op] = out[op - back]; ++op;
                        }
                    } else {
                        back = (distance + 1U) / 2;
                        if (back > op) return -1;
                        for (j = 0; j < len; ++j) {
                            uint64_t high = (out[op - back] & H18) << 18;
                            uint64_t low;
                            out[op] = high;
                            low = (out[op - back + 1] >> 18) & H18;
                            out[op++] = high | low;
                        }
                    }
                } else return -1;
            }
            bitmap = (bitmap << 1) & W36;
        }
    }
    return op == ow ? 0 : -1;
}
