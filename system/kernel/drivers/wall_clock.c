/* UTC fallback clock for PDP-6 systems without the Petit PCLK.
 * Hardware ticks are counted in the CLK interrupt path; calendar work is
 * performed lazily outside interrupt context, never on the scheduler path.
 * All persistent state occupies four kernel words.
 */
#include "pclk.h"

#ifndef WALL_CLOCK_HZ
#define WALL_CLOCK_HZ 60
#endif
#if WALL_CLOCK_HZ != 50 && WALL_CLOCK_HZ != 60
#error "WALL_CLOCK_HZ must be 50 or 60"
#endif

#define WALL_MASK 0777777777777UL
#define WALL_CENTURY 0200000000000UL

extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t state);

volatile kword_t wall_clock_ticks;
static kword_t wall_clock_time;
static kword_t wall_clock_base;
/* Installation-time hardware selection; compile-time constant uses no RAM. */

static unsigned int
days_in_month(unsigned int year, unsigned int month)
{
        static const unsigned char days[12] = {
            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
        if (month < 1U || month > 12U)
                return 0U;
        if (month == 2U && year % 4U == 0U &&
            (year % 100U != 0U || year % 400U == 0U))
                return 29U;
        return (unsigned int)days[month - 1U];
}

static int
decode_time(kword_t t, unsigned int *year, unsigned int *month,
    unsigned int *day, unsigned int *hour, unsigned int *minute,
    unsigned int *second)
{
        unsigned int bcd;
        unsigned int hi;
        unsigned int lo;

        if (t == 0UL || (t & 0400000000000UL) != 0UL)
                return -1;
        bcd = (unsigned int)((t >> 26U) & 0377UL);
        hi = (bcd >> 4U) & 017U;
        lo = bcd & 017U;
        if (hi > 9U || lo > 9U)
                return -1;
        *year = ((t & WALL_CENTURY) != 0UL ? 2100U : 2000U) +
            hi * 10U + lo;
        *month = (unsigned int)((t >> 22U) & 017UL);
        *day = (unsigned int)((t >> 17U) & 037UL);
        *hour = (unsigned int)((t >> 12U) & 037UL);
        *minute = (unsigned int)((t >> 6U) & 077UL);
        *second = (unsigned int)(t & 077UL);
        if (*year < 2026U || *year > 2125U ||
            *day == 0U || *day > days_in_month(*year, *month) ||
            *hour > 23U || *minute > 59U || *second > 59U)
                return -1;
        return 0;
}

static kword_t
encode_time(unsigned int year, unsigned int month, unsigned int day,
    unsigned int hour, unsigned int minute, unsigned int second)
{
        unsigned int yy;
        kword_t t;

        yy = year % 100U;
        t = (kword_t)(((yy / 10U) << 4U) | (yy % 10U)) << 26U;
        if (year >= 2100U)
                t |= WALL_CENTURY;
        t |= (kword_t)month << 22U;
        t |= (kword_t)day << 17U;
        t |= (kword_t)hour << 12U;
        t |= (kword_t)minute << 6U;
        t |= (kword_t)second;
        return t;
}

static kword_t
next_second(kword_t t)
{
        unsigned int year, month, day, hour, minute, second;

        if (decode_time(t, &year, &month, &day, &hour, &minute,
            &second) != 0)
                return 0UL;
        if (++second == 60U) {
                second = 0U;
                if (++minute == 60U) {
                        minute = 0U;
                        if (++hour == 24U) {
                                hour = 0U;
                                if (++day > days_in_month(year, month)) {
                                        day = 1U;
                                        if (++month == 13U) {
                                                month = 1U;
                                                ++year;
                                        }
                                }
                        }
                }
        }
        if (year > 2125U)
                return 0UL;
        return encode_time(year, month, day, hour, minute, second);
}

/* Called only after the PCLK reader reports no hardware clock. */
kword_t
wall_clock_now(void)
{
        kword_t ps, base, now, value, updated, elapsed, whole, i;

        for (;;) {
                ps = mach_pi_disable();
                value = wall_clock_time;
                base = wall_clock_base;
                now = wall_clock_ticks;
                mach_pi_restore(ps);
                if (value == 0UL)
                        return 0UL;
                elapsed = (now - base) & WALL_MASK;
                whole = elapsed / (kword_t)WALL_CLOCK_HZ;
                if (whole == 0UL)
                        return value;
                updated = value;
                for (i = 0UL; i < whole && updated != 0UL; ++i)
                        updated = next_second(updated);
                ps = mach_pi_disable();
                if (wall_clock_time == value && wall_clock_base == base) {
                        wall_clock_time = updated;
                        wall_clock_base = (base + whole *
                            (kword_t)WALL_CLOCK_HZ) & WALL_MASK;
                        mach_pi_restore(ps);
                        return updated;
                }
                mach_pi_restore(ps);
        }
}

/* Called by privileged SYS_EXT_SETTIME only. PCLK hardware is read-only. */
int
wall_clock_set(kword_t requested)
{
        unsigned int year, month, day, hour, minute, second;
        kword_t ps;

        if (decode_time(requested, &year, &month, &day, &hour,
            &minute, &second) != 0 || pclk_time36() != 0UL)
                return -1;
        ps = mach_pi_disable();
        wall_clock_time = requested;
        wall_clock_base = wall_clock_ticks;
        mach_pi_restore(ps);
        return 0;
}
