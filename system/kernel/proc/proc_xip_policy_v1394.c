/**
 * @file proc_xip_policy_v1394.c
 * @brief Validate one deliberately tiny trusted raw-XIP user program.
 *
 * The PDP-6 APR has no text write-protection. A PURE image and a no-write
 * filesystem mode do not prevent arbitrary user STORE instructions from
 * corrupting shared MEMFS. Until a real instruction verifier exists, admit
 * only MOVEI AC2,immediate followed by monitor UUO 040 (EXIT). The program
 * uses no stack, branches, indirect operands, data, or writable memory.
 * All words outside the two instructions and two-word header must be zero.
 *
 * Caller must have retained the provider's exact pin and checked immutable
 * file ownership/mode before supplying trusted_source=1. On failure the
 * caller releases the pin, without touching the potential process VM.
 *
 * This bounded, allocation-free verifier is staged for the future
 * privileged RUN/EXEC admission, not installed in production KCORE.
 */
#include "proc_xip_policy_v1394.h"

#define XIP_MAGIC_V1394         054506000000UL
#define XIP_PURE_V1394          0200000UL
#define XIP_ENTRY_V1394         020UL
#define XIP_EXIT_V1394          040000000000UL
#define XIP_ADDR_MASK_V1394     0777777UL
#define XIP_APR_WORDS_V1394     02000UL
#define XIP_MOVEI_AC2_V1394    (((kword_t)0201UL << 27U) | \
                                ((kword_t)2UL << 23U))

int
proc_xip_policy_v1394(const kword_t *image, kword_t words,
    kword_t entry, unsigned int uid, unsigned int trusted_source,
    unsigned int argc, unsigned int envc)
{
        kword_t i;

        if (!image || uid != 0U || trusted_source != 1U ||
            argc || envc || entry != XIP_ENTRY_V1394 ||
            words <= XIP_APR_WORDS_V1394 ||
            (words & (XIP_APR_WORDS_V1394 - 1UL)) != 0UL ||
            words > XIP_ADDR_MASK_V1394 ||
            image[0] != XIP_MAGIC_V1394 ||
            image[1] != ((XIP_PURE_V1394 << 18U) | XIP_ENTRY_V1394) ||
            (image[XIP_ENTRY_V1394] & ~XIP_ADDR_MASK_V1394) !=
                XIP_MOVEI_AC2_V1394 ||
            image[XIP_ENTRY_V1394 + 1UL] != XIP_EXIT_V1394)
                return -1;

        for (i = 2UL; i < words; ++i) {
                if (i == XIP_ENTRY_V1394 || i == XIP_ENTRY_V1394 + 1UL)
                        continue;
                if (image[i] != 0UL)
                        return -1;
        }
        return 0;
}
