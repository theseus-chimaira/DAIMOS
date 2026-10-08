/** Reclaimable AUXSTORE discovery and descriptor validation. */
#include "auxstore.h"
#include "bstore.h"
#include "blockset_layout.h"
#include "dsk270.h"
#include "drm236.h"
#include "fs_backing.h"
#include "fs_mres.h"
#include "module.h"

#define AUXSTORE_HALF_MASK   0777777UL

unsigned int auxstore_kind;
unsigned int auxstore_unit;
kword_t auxstore_backstore_start;
kword_t auxstore_backstore_blocks;
kword_t auxstore_logstore_start;
kword_t auxstore_logstore_blocks;
kword_t auxstore_cache_start;
kword_t auxstore_cache_blocks;

static int
auxstore_phys_read(unsigned int kind, unsigned int unit, kword_t block,
    kword_t *buf)
{
        if (kind == AUXSTORE_KIND_DRMSET)
                return drm236_read_block((unsigned int)(block & 3UL),
                    block >> 2U, buf);
        if (kind == AUXSTORE_KIND_DRM)
                return drm236_read_block(unit, block, buf);
        return dsk270_read_sector(unit, block, buf);
}

static int
auxstore_phys_write(unsigned int kind, unsigned int unit, kword_t block,
    const kword_t *buf)
{
        if (kind == AUXSTORE_KIND_DRMSET)
                return drm236_write_block((unsigned int)(block & 3UL),
                    block >> 2U, buf);
        if (kind == AUXSTORE_KIND_DRM)
                return drm236_write_block(unit, block, buf);
        return dsk270_write_sector(unit, block, buf);
}

static int
auxstore_overlap(kword_t a, kword_t an, kword_t b, kword_t bn)
{
        if (an == 0UL || bn == 0UL)
                return 0;
        return a < b + bn && b < a + an;
}

static int
auxstore_try(unsigned int kind, unsigned int unit)
{
        kword_t *block;
        kword_t limit;
        kword_t s;
        kword_t sn;
        kword_t l;
        kword_t ln;
        kword_t c;
        kword_t cn;
        kword_t tail_begin = 0UL;
        kword_t tail_limit = 0UL;

        block = fs_block_workspace;
        if (auxstore_phys_read(kind, unit, 0UL, block) != 0 ||
            block[AUXSTORE_DESC_MAGIC] != AUXSTORE_MAGIC ||
            block[AUXSTORE_DESC_VERSION] !=
                (kind == AUXSTORE_KIND_DRMSET ? AUXSTORE_SET_VERSION :
                AUXSTORE_VERSION))
                return -1;
        if (kind == AUXSTORE_KIND_DRMSET) {
                kword_t raw[5];
                unsigned int member, i;
                for (i = 0U; i < 5U; ++i)
                        raw[i] = block[i];
                if (block[BLOCKSET_LAYOUT_MAGIC_WORD] !=
                    BLOCKSET_LAYOUT_MAGIC_D6FSR2)
                        return -1;
                tail_begin = (((block[BLOCKSET_LAYOUT_RANGE_WORD] >> 18U) &
                    AUXSTORE_HALF_MASK) +
                    (block[BLOCKSET_LAYOUT_RANGE_WORD] & AUXSTORE_HALF_MASK))
                    * 4UL;
                tail_limit = tail_begin + block[BLOCKSET_LAYOUT_SWAP_TAIL]
                    * 4UL;
                /* Reject incomplete or inconsistent four-member sets. */
                for (member = 1U; member < 4U; ++member) {
                        if (drm236_read_block(member, 0UL, block) != 0)
                                return -1;
                        for (i = 0U; i < 5U; ++i)
                                if (block[i] != raw[i])
                                        return -1;
                        if (block[BLOCKSET_LAYOUT_MAGIC_WORD] !=
                            BLOCKSET_LAYOUT_MAGIC_D6FSR2 ||
                            (((block[BLOCKSET_LAYOUT_RANGE_WORD] >> 18U) &
                            AUXSTORE_HALF_MASK) +
                            (block[BLOCKSET_LAYOUT_RANGE_WORD] &
                            AUXSTORE_HALF_MASK)) * 4UL != tail_begin ||
                            tail_limit != tail_begin +
                            block[BLOCKSET_LAYOUT_SWAP_TAIL] * 4UL)
                                return -1;
                }
        }
        limit = kind == AUXSTORE_KIND_DRMSET ?
            (kword_t)(4U * DRM236_BLOCKS_PER_UNIT) :
            (kind == AUXSTORE_KIND_DRM ? DRM236_BLOCKS_PER_UNIT :
            DSK270_SECTORS_PER_UNIT);
        s = (block[AUXSTORE_DESC_BACKSTORE] >> 18U) & AUXSTORE_HALF_MASK;
        sn = block[AUXSTORE_DESC_BACKSTORE] & AUXSTORE_HALF_MASK;
        l = (block[AUXSTORE_DESC_LOGSTORE] >> 18U) & AUXSTORE_HALF_MASK;
        ln = block[AUXSTORE_DESC_LOGSTORE] & AUXSTORE_HALF_MASK;
        c = (block[AUXSTORE_DESC_CACHE] >> 18U) & AUXSTORE_HALF_MASK;
        cn = block[AUXSTORE_DESC_CACHE] & AUXSTORE_HALF_MASK;
        if ((kind == AUXSTORE_KIND_DRMSET &&
            ((sn != 0UL && (s < tail_begin || s + sn > tail_limit)) ||
             (ln != 0UL && (l < tail_begin || l + ln > tail_limit)) ||
             (cn != 0UL && (c < tail_begin || c + cn > tail_limit)))) ||
            (sn != 0UL && (s == 0UL || s >= limit || sn > limit - s)) ||
            (ln != 0UL && (l == 0UL || l >= limit || ln > limit - l)) ||
            (cn != 0UL && (c == 0UL || c >= limit || cn > limit - c)) ||
            auxstore_overlap(s, sn, l, ln) ||
            auxstore_overlap(s, sn, c, cn) ||
            auxstore_overlap(l, ln, c, cn))
                return -1;
        auxstore_kind = kind;
        auxstore_unit = unit;
        auxstore_backstore_start = s;
        auxstore_backstore_blocks = sn;
        auxstore_logstore_start = l;
        auxstore_logstore_blocks = ln;
        auxstore_cache_start = c;
        auxstore_cache_blocks = cn;
        return 0;
}

int
auxstore_boot_discover(void)
{
        auxstore_kind = AUXSTORE_KIND_NONE;
        auxstore_backstore_blocks = 0UL;
        auxstore_logstore_blocks = 0UL;
        auxstore_cache_blocks = 0UL;

        if (module_service_get(MODULE_SERVICE_DRM_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK) != 0U &&
            auxstore_try(AUXSTORE_KIND_DRMSET, 0U) == 0)
                return 0;
        /* V0.9 auto-discovery is deliberately limited to unit zero of each
         * controller class.  Type-270 has no harmless media-presence probe:
         * issuing a transfer to an unattached trailing unit can leave the
         * controller unusable for the attached root unit.  Root discovery
         * follows the same rule and stops before absent trailing units.  A
         * future explicit AUXSTORE selector may name higher units without
         * making boot guess by destructive I/O.  Systems with the intended
         * single auxiliary drum naturally use DRM0; DSK0 is the fallback. */
        if (module_service_get(MODULE_SERVICE_DRM_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK) != 0U &&
            auxstore_try(AUXSTORE_KIND_DRM, 0U) == 0)
                return 0;
        if (module_service_get(MODULE_SERVICE_DSK_READ_SECTOR) != 0U &&
            module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR) != 0U &&
            auxstore_try(AUXSTORE_KIND_DSK, 0U) == 0)
                return 0;
        return -1;
}

/** Publish AUXSTORE BACKSTORE after all storage MINITs are installed. */
void
auxstore_post_minits(void)
{
        unsigned int selector;

        if (auxstore_kind == AUXSTORE_KIND_NONE ||
            auxstore_backstore_blocks == 0UL)
                return;
        selector = auxstore_unit;
        if (auxstore_kind == AUXSTORE_KIND_DRM ||
            auxstore_kind == AUXSTORE_KIND_DRMSET)
                selector |= (unsigned int)FS_BACKING_DIRECT_DRM_TAG;
        if (auxstore_kind == AUXSTORE_KIND_DRMSET)
                selector |= (unsigned int)FS_BACKING_DIRECT_SET_TAG;
        blockset_direct_configure(selector, auxstore_backstore_start, 0UL,
            auxstore_backstore_blocks);
        backstore_blocks = auxstore_backstore_blocks;
}

int
auxstore_boot_log_read(kword_t first, kword_t *buf)
{
        if (auxstore_kind == AUXSTORE_KIND_NONE || buf == 0 ||
            first >= auxstore_logstore_blocks)
                return -1;
        return auxstore_phys_read(auxstore_kind, auxstore_unit,
            auxstore_logstore_start + first, buf);
}

int
auxstore_boot_log_write(kword_t first, const kword_t *buf)
{
        if (auxstore_kind == AUXSTORE_KIND_NONE || buf == 0 ||
            first >= auxstore_logstore_blocks)
                return -1;
        return auxstore_phys_write(auxstore_kind, auxstore_unit,
            auxstore_logstore_start + first, buf);
}
