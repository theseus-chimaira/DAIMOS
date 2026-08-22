#include "pt.h"
#include "kcore_io.h"

#define PTP_STATE_PENDING       0001UL

static volatile kword_t ptp_state;

int
ptp_pi_handler(unsigned int level, kword_t opaque)
{
        kword_t st;

        (void)level;
        (void)opaque;
        st = ptp_coni();
        if ((st & PT_ST_DONE) == 0)
                return PDP10_PI_NOT_HANDLED;
        ptp_state &= ~PTP_STATE_PENDING;
        ptp_cono(PT_NATIVE_PI_LEVEL);
        return PDP10_PI_HANDLED;
}

int
ptp_putchar(int c)
{
        unsigned int i;
        kword_t st;

        if ((ptp_state & PTP_STATE_PENDING) != 0)
                return PT_E_BUSY;
        st = ptp_coni();
        if ((st & PTP_ST_NO_TAPE) != 0)
                return PT_E_IO;
        if ((st & PT_ST_BUSY) != 0)
                return PT_E_BUSY;
        ptp_state |= PTP_STATE_PENDING;
        ptp_cono(PT_NATIVE_PI_LEVEL);
        ptp_datao(((kword_t)c) & 0377UL);
        for (i = PT_WAIT_READY; i != 0U; --i) {
                if ((ptp_state & PTP_STATE_PENDING) == 0)
                        return PT_E_OK;
                pdp10_io_wait(1U);
        }
        ptp_state &= ~PTP_STATE_PENDING;
        ptp_cono(PT_NATIVE_PI_LEVEL);
        return PT_E_TIMEOUT;
}
