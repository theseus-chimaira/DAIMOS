/* Pair the v078 no-allocation native KPACK codec with the v077/v46
 * MM-authoritative packed transaction. Inactive until scheduler, KCORE
 * resident-map and full PDP-6 transaction acceptance are complete.
 */
#include "proc_packed_inline_v079.h"
#include "proc_packed_evict_v077.h"

/* PDP-6 assembly ABI: C arguments AC1..AC4, result AC1. */
extern int d6codec_kpack_inline_size_v078(const kword_t *src,
    unsigned int words);
extern int d6codec_kpack_enc_v078(const kword_t *src,
    unsigned int words, kword_t *dst, unsigned int capacity);
extern int d6codec_kpack_dec_v078(const kword_t *src,
    unsigned int words, kword_t *dst, unsigned int expected_words);

int
proc_packed_inline_evict_v079(unsigned int slot, kword_t retained_offset)
{
    return proc_packed_evict_codec_v077(slot, retained_offset,
        d6codec_kpack_inline_size_v078,
        d6codec_kpack_enc_v078);
}

int
proc_packed_inline_restore_v079(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload_pure)
{
    return proc_packed_restore_reg_v46(slot, backing,
        d6codec_kpack_dec_v078, reload_pure);
}
