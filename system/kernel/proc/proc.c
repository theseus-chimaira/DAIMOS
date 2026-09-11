#include "proc.h"
#include "fs_mres.h"
#include "mm.h"
#include "proc_swap.h"
#include "procfs.h"

#define PROC_UAREA_MM_OWNER_BASE 01000U

unsigned int proc_sched_age_phase;

static inline void
proc_set_field(struct proc *p, kword_t mask, unsigned int shift,
    unsigned int value)
{
        p->sched = (p->sched & ~(mask << shift)) |
            (((kword_t)value & mask) << shift);
}

static inline void
proc_set_cpu(struct proc *p, unsigned int value)
{
        proc_set_field(p, PROC_CPU_MASK, PROC_CPU_SHIFT, value);
}

static inline void
proc_set_sleep_age(struct proc *p, unsigned int value)
{
        proc_set_field(p, PROC_SLEEP_MASK, PROC_SLEEP_SHIFT, value);
}

static unsigned int
proc_effective(const struct proc *p)
{
        return PROC_NICE_ENCODED(p) + PROC_CPU_PENALTY(p);
}

static inline unsigned int
proc_uarea_owner(unsigned int slot)
{
        return PROC_UAREA_MM_OWNER_BASE + slot;
}

static inline int
proc_uarea_release(unsigned int slot, struct proc *p)
{
        kword_t base;

        if (!PROC_HAS_UAREA(p))
                return 0;
        base = PROC_UAREA_BASE(p);
        if (base == 0UL || mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
            proc_uarea_owner(slot)) != MM_OK)
                return -1;
        p->meta &= ~((kword_t)PROC_F_UAREA << PROC_FLAGS_SHIFT);
        PROC_SET_META_LH(p, 0UL);
        return 0;
}

static inline int
proc_slot_cleanup(unsigned int slot, struct proc *p)
{
        proc_swap_detach(slot);
        if (proc_uarea_release(slot, p) != 0)
                return -1;
        p->meta = 0UL;
        p->mem_layout = 0UL;
        p->sched = 0UL;
        while (proc_high_slot > 1U &&
            PROC_STATE(&proc_table[proc_high_slot - 1U]) == PROC_FREE)
                --proc_high_slot;
        return 0;
}

/*
 * Finish EXIT after the assembly boundary has moved execution off the
 * process-private u-area stack and disabled priority interrupts.  No code may
 * touch the old process extent after mm_free() succeeds.
 *
 * Return 1 if another user process still exists, 0 for the normal final-user
 * shutdown case, and -1 if the exiting slot could not be released safely.
 */
int
proc_exit_finish(void)
{
        unsigned int slot;
        struct proc *p;
        kword_t base;

        slot = (unsigned int)proc_current_slot;
        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
                return -1;
        p = &proc_table[slot];
        base = PROC_MEM_BASE(p);
        if (PROC_STATE(p) == PROC_FREE || base == 0UL || PROC_TRANSITION(p))
                return -1;

        p->meta |= (kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT;
        proc_current_slot = 0UL;
        if (mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK) {
                proc_current_slot = (kword_t)slot;
                p->meta &= ~((kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT);
                return -1;
        }

        if (proc_slot_cleanup(slot, p) != 0)
                return -1;
        return proc_high_slot > 1U ? 1 : 0;
}

kword_t
proc_comm(const struct proc *p)
{
        unsigned int slot;

        if (p == 0 || proc_table == 0)
                return VFS_SIX6('U','S','E','R',' ',' ');
        slot = (unsigned int)(p - proc_table);
        if (slot == 0U)
                return VFS_SIX6('S','W','A','P','P','E');
        if (slot == 1U)
                return VFS_SIX6('I','N','I','T',' ',' ');
        return VFS_SIX6('U','S','E','R',' ',' ');
}

int
proc_nice_value(unsigned int slot)
{
        if (proc_table == 0 || slot == 0U || slot >= proc_slots ||
            PROC_STATE(&proc_table[slot]) == PROC_FREE)
                return 0;
        return (int)PROC_NICE_ENCODED(&proc_table[slot]) -
            (int)PROC_NICE_BIAS;
}

int
proc_nice_current(int value)
{
        unsigned int slot;
        struct proc *p;
        unsigned int encoded;

        slot = (unsigned int)proc_current_slot;
        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
                return -1;
        if (value < PROC_NICE_MIN)
                value = PROC_NICE_MIN;
        if (value > PROC_NICE_MAX)
                value = PROC_NICE_MAX;
        p = &proc_table[slot];
        encoded = (unsigned int)(value + (int)PROC_NICE_BIAS);
        proc_set_field(p, PROC_NICE_MASK, PROC_NICE_SHIFT, encoded);
        return value;
}

static unsigned int
proc_select_runnable(void)
{
        unsigned int n;
        unsigned int limit;
        unsigned int best;
        unsigned int best_prio;

        if (proc_table == 0 || proc_high_slot <= 1U)
                return 0U;
        limit = proc_high_slot;
        best = 0U;
        best_prio = ~0U;
        for (n = 1U; n <= limit; ++n) {
                struct proc *p;
                unsigned int slot;
                unsigned int prio;

                slot = ((unsigned int)proc_sched_cursor + n) % limit;
                if (slot == 0U)
                        continue;
                p = &proc_table[slot];
                if (PROC_STATE(p) != PROC_SRUN || PROC_MEM_BASE(p) == 0UL ||
                    PROC_TRANSITION(p))
                        continue;
                prio = proc_effective(p);
                if (best == 0U || prio < best_prio) {
                        best = slot;
                        best_prio = prio;
                }
        }
        if (best != 0U)
                proc_sched_cursor = (kword_t)best;
        return best;
}

unsigned int
proc_sched_resched_select(void)
{
        return proc_select_runnable();
}

unsigned int
proc_sched_tick_select(void)
{
        unsigned int cur;
        unsigned int i;
        unsigned int limit;
        int age_tick;

        if (proc_table == 0 || proc_high_slot <= 1U)
                return 0U;
        cur = (unsigned int)proc_current_slot;
        limit = proc_high_slot;
        age_tick = 0;
        if (++proc_sched_age_phase >= 64U) {
                proc_sched_age_phase = 0U;
                age_tick = 1;
        }

        for (i = 1U; i < limit; ++i) {
                struct proc *p;
                unsigned int cpu;

                p = &proc_table[i];
                if (PROC_STATE(p) == PROC_FREE)
                        continue;
                cpu = PROC_CPU_PENALTY(p);
                if (cpu != 0U)
                        --cpu;
                if (i == cur && PROC_STATE(p) == PROC_SRUN &&
                    !PROC_TRANSITION(p)) {
                        cpu += 2U;
                        if (cpu > (unsigned int)PROC_CPU_MASK)
                                cpu = (unsigned int)PROC_CPU_MASK;
                }
                proc_set_cpu(p, cpu);
                if (PROC_STATE(p) == PROC_SLEEP) {
                        unsigned int age;

                        age = PROC_SLEEP_AGE(p);
                        if (age_tick && age < (unsigned int)PROC_SLEEP_MASK)
                                ++age;
                        proc_set_sleep_age(p, age);
                } else if (PROC_SLEEP_AGE(p) != 0U) {
                        proc_set_sleep_age(p, 0U);
                }
        }
        return proc_select_runnable();
}

int
proc_swap_victim(unsigned int exclude_owner)
{
        unsigned int i;
        unsigned int best;
        unsigned int best_score;

        if (proc_table == 0)
                return -1;
        best = 0U;
        best_score = 0U;
        for (i = 1U; i < proc_high_slot; ++i) {
                const struct proc *p;
                unsigned int state;
                unsigned int score;
                int nice;

                if (i == exclude_owner || i == (unsigned int)proc_current_slot)
                        continue;
                p = &proc_table[i];
                state = PROC_STATE(p);
                if ((state != PROC_SLEEP && state != PROC_STOP &&
                    state != PROC_SRUN) || PROC_TRANSITION(p))
                        continue;
                if (PROC_MEM_BASE(p) == 0UL || mm_is_pinned(PROC_MEM_BASE(p)))
                        continue;

                nice = proc_nice_value(i);
                if (state == PROC_SLEEP || state == PROC_STOP) {
                        score = 04000U + PROC_SLEEP_AGE(p) * 0100U;
                        if (PROC_WAIT_CLASS(p) != PROC_WAIT_NONE)
                                score += 040U;
                } else {
                        score = 01000U + proc_effective(p);
                        if (nice > 0)
                                score += 02000U + (unsigned int)nice * 010U;
                }
                if (best == 0U || score > best_score) {
                        best = i;
                        best_score = score;
                }
        }
        return best == 0U ? -1 : (int)best;
}
