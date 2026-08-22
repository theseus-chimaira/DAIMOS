#include "cty.h"
#include "kcore_io.h"
#include "kcore_pi.h"

static volatile unsigned int cty_tx_pending;
static volatile kword_t cty_rx_state;

#define CTY_RX_HEAD(state) ((unsigned int)((state) & 0777777UL))
#define CTY_RX_COUNT(state) ((unsigned int)(((state) >> 18) & 0777777UL))
#define CTY_RX_STATE(head, count) \
        ((((kword_t)(count)) << 18) | (kword_t)(head))
static volatile unsigned char cty_rx_buf[CTY_RX_BUF_SIZE];

static int
cty_rx_push(int ch)
{
        kword_t state;
        unsigned int head;
        unsigned int count;
        unsigned int tail;

        state = cty_rx_state;
        head = CTY_RX_HEAD(state);
        count = CTY_RX_COUNT(state);
        if (count >= CTY_RX_BUF_SIZE)
                return CTY_E_BUSY;
        tail = (head + count) & (CTY_RX_BUF_SIZE - 1U);
        cty_rx_buf[tail] = (unsigned char)ch;
        cty_rx_state = CTY_RX_STATE(head, count + 1U);
        return CTY_E_OK;
}

static int
cty_rx_pop(int *cp)
{
        kword_t state;
        unsigned int head;
        unsigned int count;

        state = cty_rx_state;
        head = CTY_RX_HEAD(state);
        count = CTY_RX_COUNT(state);
        if (count == 0U)
                return CTY_E_TIMEOUT;
        *cp = (int)cty_rx_buf[head];
        head = (head + 1U) & (CTY_RX_BUF_SIZE - 1U);
        cty_rx_state = CTY_RX_STATE(head, count - 1U);
        return CTY_E_OK;
}

int
cty_pi_handler(unsigned int level, kword_t opaque)
{
        kword_t st;
        kword_t word;
        int handled;

        (void)level;
        (void)opaque;
        st = cty_coni();
        handled = 0;
        if ((st & CTY_ST_INPUT_READY) != 0) {
                word = cty_datai();
                (void)cty_rx_push((int)(word & 0177UL));
                handled = 1;
        }
        if ((st & CTY_ST_OUTPUT_READY) != 0) {
                cty_tx_pending = 0U;
                cty_cono((kword_t)CTY_NATIVE_PI_LEVEL |
                    CTY_CO_CLR_OUTPUT_READY);
                handled = 1;
        }
        return handled ? PDP10_PI_HANDLED : PDP10_PI_NOT_HANDLED;
}

int
cty_putchar(int c)
{
        unsigned int i;

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
        cty_datao(((kword_t)c) & 0177UL);
        for (i = CTY_WAIT_READY; i != 0U; --i) {
                if (cty_tx_pending == 0U)
                        return CTY_E_OK;
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
        if (cty_rx_pop(cp) == CTY_E_OK)
                return CTY_E_OK;
        for (i = CTY_WAIT_READY; i != 0U; --i) {
                if (cty_rx_pop(cp) == CTY_E_OK)
                        return CTY_E_OK;
                pdp10_io_wait(1U);
        }
        return CTY_E_TIMEOUT;
}

int
cty_put6(kword_t word)
{
        unsigned int shift;
        int error;

        for (shift = 30U; ; shift -= 6U) {
                error = cty_putchar((int)(((word >> shift) & 077UL) + 040UL));
                if (error != CTY_E_OK)
                        return error;
                if (shift == 0U)
                        break;
        }
        return CTY_E_OK;
}

int
cty_newline(void)
{
        int error;

        error = cty_putchar(015);
        if (error != CTY_E_OK)
                return error;
        return cty_putchar(012);
}

int
cty_put6_spaces(unsigned int words)
{
        while (words != 0U) {
                if (cty_put6(0) != CTY_E_OK)
                        return CTY_E_TIMEOUT;
                --words;
        }
        return CTY_E_OK;
}
