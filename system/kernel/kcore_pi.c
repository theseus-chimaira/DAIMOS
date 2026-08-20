#include "kcore_pi.h"

struct pdp10_pi_slot {
        pdp10_pi_handler handler;
        kword_t opaque;
};

/* V0.1 KCORE needs one CTY handler during KINIT/MINIT bring-up. */
struct pdp10_pi_slot pdp10_pi_slots[1];
kword_t pdp10_pi_level_span[PDP10_PI_LEVELS + 1U];
unsigned int pdp10_pi_count_words[PDP10_PI_LEVELS + 1U];
unsigned int pdp10_pi_unhandled_words[PDP10_PI_LEVELS + 1U];

#define PI_SPAN_START(w) ((unsigned int)((w) & 0777777UL))
#define PI_SPAN_COUNT(w) ((unsigned int)(((w) >> 18) & 0777777UL))
#define PI_SPAN_PACK(start, count) \
    ((((kword_t)(count) & 0777777UL) << 18) | \
     ((kword_t)(start) & 0777777UL))
#define PI_LEVEL_VALID(level) \
    ((unsigned int)((level) - PDP10_PI_LEVEL_MIN) < PDP10_PI_LEVELS)

void
pdp10_pi_init(void)
{
        mach_pi_stack_prepare();
        pdp10_pi_hw_clear();
        mach_words_zero((kword_t *)pdp10_pi_slots, 2U);
        mach_words_zero(pdp10_pi_level_span, PDP10_PI_LEVELS + 1U);
        mach_words_zero((kword_t *)pdp10_pi_count_words,
            PDP10_PI_LEVELS + 1U);
        mach_words_zero((kword_t *)pdp10_pi_unhandled_words,
            PDP10_PI_LEVELS + 1U);
}

int
pdp10_pi_register(unsigned int level, pdp10_pi_handler handler, kword_t opaque)
{
        if (!PI_LEVEL_VALID(level) || handler == 0)
                return -1;
        if (PI_SPAN_START(pdp10_pi_level_span[0]) != 0U)
                return -1;
        pdp10_pi_slots[0].handler = handler;
        pdp10_pi_slots[0].opaque = opaque;
        pdp10_pi_level_span[level] = PI_SPAN_PACK(0U, 1U);
        pdp10_pi_level_span[0] = PI_SPAN_PACK(1U, 0U);
        return 0;
}
