#include "cty.h"
#include "kcore_io.h"
#include "kcore_pi.h"

static unsigned int cty_tx_pending;
static unsigned int cty_intr_pia;
static unsigned int cty_rx_head;
static unsigned int cty_rx_count;
static int cty_rx_buf[CTY_RX_BUF_SIZE];
static int cty_last_error;

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

static int
cty_rx_pop(int *cp)
{
        if (cty_rx_count == 0U)
                return CTY_E_TIMEOUT;
        *cp = cty_rx_buf[cty_rx_head];
        cty_rx_head = (cty_rx_head + 1U) & (CTY_RX_BUF_SIZE - 1U);
        --cty_rx_count;
        return CTY_E_OK;
}

static kword_t
cty_cono_word(kword_t flags)
{
        return flags | (kword_t)cty_intr_pia;
}

static int
cty_intr_service(void)
{
        kword_t st;
        kword_t word;
        int handled;

        st = cty_coni();
        handled = 0;
        if ((st & CTY_ST_INPUT_READY) != 0) {
                word = cty_datai();
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
                cty_cono(cty_cono_word(CTY_CO_CLR_OUTPUT_READY));
                handled = 1;
        }
        return handled ? CTY_E_OK : CTY_E_TIMEOUT;
}

static int
cty_intr_pi_handler(unsigned int level, kword_t opaque)
{
        (void)level;
        (void)opaque;
        return cty_intr_service() == CTY_E_OK ?
            PDP10_PI_HANDLED : PDP10_PI_NOT_HANDLED;
}

int
cty_init(void)
{
        if (cty_intr_pia != 0U)
                return CTY_E_BUSY;
        if (pdp10_pi_register(CTY_NATIVE_PI_LEVEL, cty_intr_pi_handler, 0) != 0)
                return CTY_E_ARG;
        cty_intr_pia = CTY_NATIVE_PI_LEVEL;
        cty_tx_pending = 0U;
        cty_rx_reset();
        cty_last_error = CTY_E_OK;
        cty_cono(cty_cono_word(0UL));
        pdp10_pi_hw_enable(PDP10_PI_MASK(CTY_NATIVE_PI_LEVEL));
        return CTY_E_OK;
}

int
cty_putchar(int c)
{
        unsigned int i;

        if (cty_intr_pia == 0U)
                return CTY_E_ARG;
        if (cty_tx_pending != 0U)
                return CTY_E_BUSY;
        for (i = CTY_WAIT_READY; i != 0U; --i) {
                if ((cty_coni() & CTY_ST_OUTPUT_BUSY) == 0)
                        break;
                pdp10_io_wait(1U);
        }
        if (i == 0U)
                return CTY_E_TIMEOUT;
        cty_tx_pending = 1U;
        cty_last_error = CTY_E_OK;
        cty_datao(((kword_t)c) & 0177UL);
        for (i = CTY_WAIT_READY; i != 0U; --i) {
                if (cty_tx_pending == 0U)
                        return cty_last_error;
                pdp10_io_wait(1U);
        }
        cty_tx_pending = 0U;
        return CTY_E_TIMEOUT;
}

int
cty_getchar(int *cp)
{
        unsigned int i;

        if (cp == 0)
                return CTY_E_ARG;
        if (cty_intr_pia == 0U)
                return CTY_E_ARG;
        if (cty_rx_pop(cp) == CTY_E_OK)
                return CTY_E_OK;
        for (i = CTY_WAIT_READY; i != 0U; --i) {
                if (cty_rx_pop(cp) == CTY_E_OK)
                        return cty_last_error;
                pdp10_io_wait(1U);
        }
        return CTY_E_TIMEOUT;
}
