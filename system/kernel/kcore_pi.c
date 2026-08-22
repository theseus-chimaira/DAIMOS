#include "kcore_pi.h"

#define PDP10_PI_MAX_HANDLERS 2U

struct pdp10_pi_slot {
        pdp10_pi_handler handler;
        kword_t opaque;
};

struct pdp10_pi_slot pdp10_pi_slots[PDP10_PI_MAX_HANDLERS];
kword_t pdp10_pi_level_span[PDP10_PI_LEVELS + 1U];

#define PI_SPAN_START(w) ((unsigned int)((w) & 0777777UL))
#define PI_SPAN_COUNT(w) ((unsigned int)(((w) >> 18) & 0777777UL))
#define PI_SPAN_PACK(start, count) \
    ((((kword_t)(count) & 0777777UL) << 18) | \
     ((kword_t)(start) & 0777777UL))
#define PI_LEVEL_VALID(level) \
    ((unsigned int)((level) - PDP10_PI_LEVEL_MIN) < PDP10_PI_LEVELS)

static void
pi_reindex(void)
{
        unsigned int level;
        unsigned int start;
        unsigned int count;

        start = 0U;
        for (level = PDP10_PI_LEVEL_MIN; level <= PDP10_PI_LEVEL_MAX; ++level) {
                count = PI_SPAN_COUNT(pdp10_pi_level_span[level]);
                pdp10_pi_level_span[level] = PI_SPAN_PACK(start, count);
                start += count;
        }
        pdp10_pi_level_span[0] = PI_SPAN_PACK(start, 0U);
}

void
pdp10_pi_init(void)
{
        kcore_pi_low_init();
        mach_pi_stack_prepare();
        pdp10_pi_hw_clear();
        mach_words_zero((kword_t *)pdp10_pi_slots,
            PDP10_PI_MAX_HANDLERS * 2U);
        mach_words_zero(pdp10_pi_level_span, PDP10_PI_LEVELS + 1U);
}

int
pdp10_pi_register(unsigned int level, pdp10_pi_handler handler, kword_t opaque)
{
        unsigned int start;
        unsigned int count;
        unsigned int total;
        unsigned int i;

        if (!PI_LEVEL_VALID(level) || handler == 0)
                return -1;
        start = PI_SPAN_START(pdp10_pi_level_span[level]);
        count = PI_SPAN_COUNT(pdp10_pi_level_span[level]);
        for (i = start; i < start + count; ++i) {
                if (pdp10_pi_slots[i].handler == handler &&
                    pdp10_pi_slots[i].opaque == opaque)
                        return -1;
        }
        total = PI_SPAN_START(pdp10_pi_level_span[0]);
        if (total >= PDP10_PI_MAX_HANDLERS)
                return -1;
        for (i = total; i > start + count; --i)
                pdp10_pi_slots[i] = pdp10_pi_slots[i - 1U];
        pdp10_pi_slots[start + count].handler = handler;
        pdp10_pi_slots[start + count].opaque = opaque;
        pdp10_pi_level_span[level] = PI_SPAN_PACK(start, count + 1U);
        pi_reindex();
        return 0;
}
