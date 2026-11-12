#ifndef DAIMOS_DTFS_IDENTITY_V1520_H
#define DAIMOS_DTFS_IDENTITY_V1520_H

/* Optional native DTFS V1 volume identity extension, in reserved RBN 0.
 * The directory at RBN 0144 and all old file/allocation fields stay V1.
 * Unstamped old tapes have a zero-filled RBN 0 and remain legacy media.
 * A stamped tape uses four 36-bit words followed by zero padding:
 *
 *   [0] SIXBIT "DTVOL1" (extension marker)
 *   [1] independent random high 36-bit volume identifier
 *   [2] independent random low 36-bit volume identifier
 *   [3] 36-bit complement of (marker XOR high XOR low)
 *
 * A nonempty malformed record must never be interpreted as legacy.
 * The checksum detects accidental damage, not adversarial tampering.
 * The 72-bit identifier is an equality token, not a security secret.
 * No extra physical block, native-directory word or RAM slab is needed.
 */
#define DTFS_ID_BLOCK_V1520        0U
#define DTFS_ID_MAGIC_V1520        0446466575421UL
#define DTFS_ID_MAGIC_WORD_V1520   0U
#define DTFS_ID_HIGH_WORD_V1520    1U
#define DTFS_ID_LOW_WORD_V1520     2U
#define DTFS_ID_CHECK_WORD_V1520   3U
#define DTFS_ID_USED_WORDS_V1520   4U
#define DTFS_ID_WORD_MASK_V1520    0777777777777UL

#endif
