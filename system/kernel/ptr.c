#include "ptr.h"
#include "kcore_io.h"
#include "kcore_pi.h"

#define PTR_STATE_PIA_MASK      0007UL
#define PTR_STATE_PENDING       0010UL
#define PTR_STATE_READY         0020UL
#define PTR_STATE_DATA_SHIFT    6U
#define PTR_STATE_DATA_MASK     (0377UL << PTR_STATE_DATA_SHIFT)

static volatile kword_t ptr_state;

static int
ptr_intr_pi_handler(unsigned int level, kword_t opaque)
{
        kword_t state;
        kword_t word;

        (void)level;
        (void)opaque;
        if ((ptr_coni() & PTR_ST_DONE) == 0)
                return PDP10_PI_NOT_HANDLED;
        word = ptr_datai();
        state = ptr_state & ~(PTR_STATE_PENDING | PTR_STATE_DATA_MASK);
        state |= PTR_STATE_READY |
            ((word & 0377UL) << PTR_STATE_DATA_SHIFT);
        ptr_state = state;
        return PDP10_PI_HANDLED;
}

int
ptr_init(void)
{
        if ((ptr_state & PTR_STATE_PIA_MASK) != 0)
                return PTR_E_BUSY;
        if (pdp10_pi_register(PTR_NATIVE_PI_LEVEL,
            ptr_intr_pi_handler, 0) != 0)
                return PTR_E_ARG;
        ptr_state = PTR_NATIVE_PI_LEVEL;
        ptr_cono(PTR_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(PTR_NATIVE_PI_LEVEL));
        return PTR_E_OK;
}

int
ptr_getchar(int *cp)
{
        unsigned int i;
        kword_t state;

        if (cp == 0)
                return PTR_E_ARG;
        state = ptr_state;
        if ((state & PTR_STATE_PIA_MASK) == 0)
                return PTR_E_ARG;
        if ((state & PTR_STATE_READY) != 0) {
                *cp = (int)((state & PTR_STATE_DATA_MASK) >>
                    PTR_STATE_DATA_SHIFT);
                ptr_state = state & ~PTR_STATE_READY;
                return PTR_E_OK;
        }
        if ((state & PTR_STATE_PENDING) != 0)
                return PTR_E_BUSY;

        ptr_state = state | PTR_STATE_PENDING;
        ptr_cono((state & PTR_STATE_PIA_MASK) | PTR_ST_BUSY);
        for (i = PTR_WAIT_READY; i != 0U; --i) {
                state = ptr_state;
                if ((state & PTR_STATE_READY) != 0) {
                        *cp = (int)((state & PTR_STATE_DATA_MASK) >>
                            PTR_STATE_DATA_SHIFT);
                        ptr_state = state & ~PTR_STATE_READY;
                        return PTR_E_OK;
                }
                pdp10_io_wait(1U);
        }
        ptr_state &= ~PTR_STATE_PENDING;
        ptr_cono(ptr_state & PTR_STATE_PIA_MASK);
        return PTR_E_TIMEOUT;
}
