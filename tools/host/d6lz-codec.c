/* Sole host compression format: grouped 36-bit control and paired descriptors. */
#include "d6lz-codec.h"
#include "d6codec-group-v071.h"
#include <stdlib.h>
#include <stdint.h>
int d6lz_codec_compress(const uint64_t *in, size_t n,
    uint64_t **outp, size_t *outn)
{
    size_t cap;
    uint64_t *buffer;
    if (!outp || !outn || (n && !in) || n > (SIZE_MAX-32U)/2U)
        return -1;
    cap=2*n+32U;
    buffer=malloc(cap*sizeof(*buffer));
    if (!buffer) return -1;
    if (d6codec_group_v072_encode(in,n,buffer,cap,outn)) {
        free(buffer);return -1;
    }
    *outp=buffer;
    return 0;
}
int d6lz_codec_decompress(const uint64_t *in,size_t n,
    uint64_t *out,size_t outn)
{
    return d6codec_group_v071_decode(in,n,out,outn);
}
