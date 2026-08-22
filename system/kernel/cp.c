#include "card.h"
#include "kcore_io.h"

#define CARD_STATE_PENDING      0001UL
#define CARD_STATE_DONE         0002UL
#define CARD_STATE_ERROR        0004UL
#define CARD_STATE_COL_SHIFT    3U
#define CARD_STATE_COL_MASK     (0177UL << CARD_STATE_COL_SHIFT)

static volatile kword_t cp_state;
static const kword_t *cp_cols;

static unsigned int
card_state_col(kword_t state)
{
        return (unsigned int)((state & CARD_STATE_COL_MASK) >> CARD_STATE_COL_SHIFT);
}
int
cp_pi_handler(unsigned int level, kword_t opaque)
{
        kword_t st;
        kword_t state;
        unsigned int col;

        (void)level;
        (void)opaque;
        state = cp_state;
        if ((state & CARD_STATE_PENDING) == 0)
                return PDP10_PI_NOT_HANDLED;
        st = cp_coni();
        if ((st & (CP_ST_ERROR | CP_ST_TROUBLE)) != 0) {
                cp_state = CARD_STATE_DONE | CARD_STATE_ERROR;
                return PDP10_PI_HANDLED;
        }
        if ((st & CP_ST_DATA_REQ) != 0) {
                col = card_state_col(state);
                if (col >= CARD_COLUMNS) {
                        cp_state = CARD_STATE_DONE | CARD_STATE_ERROR;
                        return PDP10_PI_HANDLED;
                }
                cp_datao(cp_cols[col] & CARD_COLUMN_MASK);
                ++col;
                cp_state = CARD_STATE_PENDING |
                    ((kword_t)col << CARD_STATE_COL_SHIFT);
                if (col >= CARD_COLUMNS)
                        cp_cono((kword_t)CARD_NATIVE_PI_LEVEL | CP_CO_EJECT |
                            CP_CO_EN_END_CARD);
                return PDP10_PI_HANDLED;
        }
        if ((st & CP_ST_END_CARD) != 0) {
                cp_cono((kword_t)CARD_NATIVE_PI_LEVEL | CP_CO_CLR_END_CARD);
                cp_state = CARD_STATE_DONE |
                    (state & CARD_STATE_COL_MASK);
                return PDP10_PI_HANDLED;
        }
        return PDP10_PI_NOT_HANDLED;
}

int
cp_punch_card(const kword_t cols[CARD_COLUMNS])
{
        unsigned int i;
        kword_t state;

        if (cols == 0)
                return CARD_E_ARG;
        if ((cp_state & CARD_STATE_PENDING) != 0)
                return CARD_E_BUSY;
        cp_cols = cols;
        cp_state = CARD_STATE_PENDING;
        cp_cono((kword_t)CARD_NATIVE_PI_LEVEL | CP_CO_SET_PUNCH_ON |
            CP_CO_CLR_END_CARD | CP_CO_CLR_ERROR | CP_CO_EN_END_CARD);
        for (i = CARD_WAIT_READY; i != 0U; --i) {
                state = cp_state;
                if ((state & CARD_STATE_DONE) != 0) {
                        if ((state & CARD_STATE_ERROR) != 0)
                                return CARD_E_IO;
                        return card_state_col(state) == CARD_COLUMNS ?
                            (int)CARD_COLUMNS : CARD_E_LIMIT;
                }
                pdp10_io_wait(1U);
        }
        cp_state = 0;
        cp_cono(CARD_NATIVE_PI_LEVEL);
        return CARD_E_TIMEOUT;
}
