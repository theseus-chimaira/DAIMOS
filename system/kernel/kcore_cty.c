#include "kcore_cty.h"
#include "kcore_io.h"
#include "kcore_pi.h"

static unsigned int cty_tx_pending;
static unsigned int cty_intr_pia;
static unsigned int cty_rx_head;
static unsigned int cty_rx_count;
static int cty_rx_buf[CTY_RX_BUF_SIZE];
static int cty_last_error;

static int cty_intr_service(void);

static void
cty_rx_reset(void)
{
        cty_rx_head = 0U;
        cty_rx_count = 0U;
}

static int
cty_rx_push(int ch)
{
        unsigned int tail;

        if (cty_rx_count >= CTY_RX_BUF_SIZE)
                return CTY_E_BUSY;
        tail = (cty_rx_head + cty_rx_count) & (CTY_RX_BUF_SIZE - 1U);
        cty_rx_buf[tail] = ch;
        ++cty_rx_count;
        return CTY_E_OK;
}

static kword_t
cty_cono_word(kword_t flags)
{
        return flags | (kword_t)cty_intr_pia;
}

static int
cty_ready(void)
{
        return (pdp10_coni(CTY_DEVICE) & CTY_ST_OUTPUT_BUSY) == 0 ? 1 : 0;
}

static int
cty_intr_service(void)
{
        kword_t st;
        kword_t word;
        int handled;

        st = pdp10_coni(CTY_DEVICE);
        handled = 0;
        if ((st & CTY_ST_INPUT_READY) != 0) {
                word = pdp10_datai(CTY_DEVICE);
                if (cty_rx_push((int)(word & 0177UL)) != CTY_E_OK)
                        cty_last_error = CTY_E_BUSY;
                else
                        cty_last_error = CTY_E_OK;
                handled = 1;
        }
        if ((st & CTY_ST_OUTPUT_READY) != 0) {
                cty_tx_pending = 0U;
                if (cty_last_error != CTY_E_BUSY)
                        cty_last_error = CTY_E_OK;
                pdp10_cono(CTY_DEVICE,
                    cty_cono_word(CTY_CO_CLR_OUTPUT_READY));
                handled = 1;
        }
        return handled ? CTY_E_OK : CTY_E_TIMEOUT;
}

int
cty_putchar_intr(int c)
{
        unsigned int i;

        if (cty_intr_pia == 0U)
                return CTY_E_ARG;
        if (cty_tx_pending != 0U)
                return CTY_E_BUSY;
        for (i = CTY_WAIT_READY; i != 0U; i--) {
                if (cty_ready() != 0)
                        break;
                pdp10_io_wait(1U);
        }
        if (i == 0U)
                return CTY_E_TIMEOUT;
        cty_tx_pending = 1U;
        cty_last_error = CTY_E_OK;
        pdp10_datao(CTY_DEVICE, ((kword_t)c) & 0177UL);
        for (i = CTY_WAIT_READY; i != 0U; i--) {
                if (cty_tx_pending == 0U)
                        return cty_last_error;
                pdp10_io_wait(1U);
        }
        cty_tx_pending = 0U;
        return CTY_E_TIMEOUT;
}

static int cty_intr_pi_handler(unsigned int level, kword_t opaque);

int
cty_intr_connect_pi(unsigned int level)
{
        if (cty_intr_pia == level)
                return CTY_E_OK;
        if (pdp10_pi_register(level, cty_intr_pi_handler, 0) != 0)
                return CTY_E_ARG;
        cty_intr_pia = level;
        cty_tx_pending = 0U;
        cty_rx_reset();
        cty_last_error = CTY_E_OK;
        pdp10_cono(CTY_DEVICE, cty_cono_word(0UL));
        pdp10_pi_hw_enable(PDP10_PI_MASK(level));
        return CTY_E_OK;
}

static int
cty_intr_pi_handler(unsigned int level, kword_t opaque)
{
        (void)level;
        (void)opaque;
        return cty_intr_service() == CTY_E_OK ?
            PDP10_PI_HANDLED : PDP10_PI_NOT_HANDLED;
}
