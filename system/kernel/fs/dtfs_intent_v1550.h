#ifndef DAIMOS_DTFS_INTENT_V1550_H
#define DAIMOS_DTFS_INTENT_V1550_H

#include "dtfs_identity_v1520.h"

/* Experimental native DTFS write-ahead intent record in reserved RBN 0.
 * Do not emit this record except with the explicit host formatter option
 * until the provider and owner-safe unmount transitions are implemented.
 * A DTVOL2/CLEAN record by itself does not confer writable admission.
 *
 * The volume-id checksum is compatible with the older four-word DTVOL1
 * scheme, but the magic differs and two checked 36-bit words are added.
 * The rest of the 128-word physical block MUST be zero.
 */
#define DTFS_INTENT_MAGIC_V1550        0446466575422UL
#define DTFS_INTENT_STATE_WORD_V1550   4U
#define DTFS_INTENT_SEAL_WORD_V1550    5U
#define DTFS_INTENT_USED_WORDS_V1550   6U
#define DTFS_INTENT_DIRTY_V1550        0552255225522UL
#define DTFS_INTENT_SALT_V1550         0246134572610UL

/* Native validator return ABI: 0=CLEAN, 1=WRITE_INTENT, -1=INVALID.
 * All-zero legacy and DTVOL1 records are INVALID for durable admission.
 */
#define DTFS_INTENT_CLEAN_RESULT_V1550 0
#define DTFS_INTENT_DIRTY_RESULT_V1550 1
#define DTFS_INTENT_BAD_RESULT_V1550  (-1)

#endif
