/* Bind native v088 relocated inline codec to the MM-authoritative
 * packed eviction (v077) and restoration (v046) transactions. Active only
 * in the PACKED_RUNTIME_V088 development kernel, never in production mode.
 */
#include "proc_packed_inline_v088.h"
#include "proc_packed_evict_v077.h"

/* PDP-6 assembly ABI: C arguments AC1..AC4, result AC1. */
extern int d6codec_kpack_inline_size_v088(const kword_t *src,
    unsigned int words);
extern int d6codec_kpack_inline_encode_v088(const kword_t *src,
    unsigned int words, kword_t *dst, unsigned int capacity);
extern int d6codec_kpack_inline_decode_v088(const kword_t *src,
    unsigned int words, kword_t *dst, unsigned int expected_words);

int
proc_packed_inline_evict_v088(unsigned int slot, kword_t retained_offset)
{
    return proc_packed_evict_transaction_v077(slot, retained_offset,
        d6codec_kpack_inline_size_v088,
        d6codec_kpack_inline_encode_v088);
}

int
proc_packed_inline_restore_v088(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload_pure)
{
    return proc_packed_restore_transaction_v46(slot, backing,
        d6codec_kpack_inline_decode_v088, reload_pure);
}
