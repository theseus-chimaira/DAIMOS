#ifndef DAIMON_DTFS_MEDIA_H
#define DAIMON_DTFS_MEDIA_H

/* On-media DECtape layout shared by the resident provider and transient
 * userspace probe/check helper.  Keep policy out of this header: these are
 * only format constants and the compact resident media tags. */
#define DTFS_BLOCK_WORDS      0200U
#define DTFS_BLOCKS           01102U
#define DTFS_LAST_BLOCK       01101U
#define DTFS_DIR_BLOCK        0144U
#define DTFS_ITS_DIR_BLOCK    0100U
#define DTFS_MAP_WORDS        0123U
#define DTFS_NAME_BASE        0123U
#define DTFS_TENEX_EXT_BASE   0151U
#define DTFS_MAGIC_WORD       0177U
#define DTFS_DATA_WORDS       0177U

#define DTFS_OWNER_FREE       000U
#define DTFS_OWNER_NATIVE_TAG 035U
#define DTFS_OWNER_RESERVED   036U
#define DTFS_OWNER_INVALID    037U
#define DTFS_NATIVE_MAGIC     0446446632021UL
#define DTFS_TENEX_MAX_FILE   026U
#define DTFS_ITS_FILE_SLOTS   027U
#define DTFS_ITS_NAME_WORDS   056U
#define DTFS_ITS_MAP_ENTRIES  01076U
#define DTFS_ITS_END          037U
#define DTFS_ITS_DIR_OWNER    033U
#define DTFS_ITS_END_BLOCK    01067U
#define DTFS_ITS_MAP_FIRST    056U
#define DTFS_ITS_MAP_DIR      067U
#define DTFS_ITS_MAP_LAST     0177U
#define DTFS_ITS_MAP_RESERVED 0757367573674UL
#define DTFS_ITS_MAP_DIRWORD  0660000000000UL
#define DTFS_ITS_MAP_END      0777777777776UL
#define DTFS_TENEX_RESERVED   036U
#define DTFS_TENEX_INVALID    037U

#define DTFS_NEXT_SHIFT       18U
#define DTFS_FIRST_SHIFT      8U
#define DTFS_BLOCKNO_MASK     01777UL
#define DTFS_COUNT_MASK       0377UL
#define DTFS_NAME2_MASK       0777777777700UL
#define DTFS_TENEX_EXT_MASK   0777777000000UL

#define DTFS_MEDIA_UNIT_MASK  07U
#define DTFS_MEDIA_TENEX      010U
#define DTFS_MEDIA_ITS        020U

#endif
