/**
 * @file wordtoken.h
 * @brief Fixed-width token packing inside one 36-bit DAIMOS word.
 *
 * DAIMOS transports opaque 36-bit words.  Text and character-like devices
 * translate only at the format/device boundary.  This header defines the one
 * common token layout used by those adapters without introducing a callable
 * resident codec or a runtime-selectable bit width.
 *
 * Tokens are packed most-significant first:
 *
 *   6 bit:  [0][1][2][3][4][5]                 6 tokens, no padding
 *   7 bit:  [0][1][2][3][4][0]                 5 tokens, 1 low pad bit
 *   8 bit:  [0][1][2][3][0000]                 4 tokens, 4 low pad bits
 *  12 bit:  [0][1][2]                          3 tokens, no padding
 *
 * A partially filled word leaves all trailing token slots and pad bits zero.
 * Framing is intentionally outside this interface: S6REC owns SIXBIT length,
 * TTY owns cooked/raw character semantics, paper tape owns its byte count,
 * and card I/O owns its 80-column record boundary.
 *
 * The helpers are static inline and width-specialized.  There is no generic
 * width argument, division, modulo, or shared KCORE routine in the hot path.
 * Assembly drivers may implement the same layout directly with fixed shifts
 * or byte pointers; this header is the authoritative source-level contract.
 */
#ifndef DAIMON_WORDTOKEN_H
#define DAIMON_WORDTOKEN_H

#include "kcore.h"

#define WORDTOKEN6_BITS          6U
#define WORDTOKEN6_PER_WORD      6U
#define WORDTOKEN6_MASK          077U
#define WORDTOKEN6_FIRST_SHIFT   30
#define WORDTOKEN6_PAD_BITS      0U

#define WORDTOKEN7_BITS          7U
#define WORDTOKEN7_PER_WORD      5U
#define WORDTOKEN7_MASK          0177U
#define WORDTOKEN7_FIRST_SHIFT   29
#define WORDTOKEN7_PAD_BITS      1U

#define WORDTOKEN8_BITS          8U
#define WORDTOKEN8_PER_WORD      4U
#define WORDTOKEN8_MASK          0377U
#define WORDTOKEN8_FIRST_SHIFT   28
#define WORDTOKEN8_PAD_BITS      4U

#define WORDTOKEN12_BITS         12U
#define WORDTOKEN12_PER_WORD     3U
#define WORDTOKEN12_MASK         07777U
#define WORDTOKEN12_FIRST_SHIFT  24
#define WORDTOKEN12_PAD_BITS     0U

/**
 * Sequential fixed-width packer state.
 *
 * Call the width-specific begin function before the first token.  Each put
 * stores one token and returns nonzero after the final slot has been filled.
 * TOKEN must already fit the selected width; keeping validation at the caller
 * avoids an extra mask/branch in hardware and text hot loops.
 */
struct wordtoken_pack {
        kword_t word;
        int shift;
};

static inline int
wordtoken6_valid(unsigned int token)
{
        return token <= WORDTOKEN6_MASK;
}

static inline int
wordtoken7_valid(unsigned int token)
{
        return token <= WORDTOKEN7_MASK;
}

static inline int
wordtoken8_valid(unsigned int token)
{
        return token <= WORDTOKEN8_MASK;
}

static inline int
wordtoken12_valid(unsigned int token)
{
        return token <= WORDTOKEN12_MASK;
}

static inline void
wordtoken6_begin(struct wordtoken_pack *pack)
{
        pack->word = 0UL;
        pack->shift = WORDTOKEN6_FIRST_SHIFT;
}

static inline int
wordtoken6_put(struct wordtoken_pack *pack, unsigned int token)
{
        pack->word |= (kword_t)token << pack->shift;
        pack->shift -= (int)WORDTOKEN6_BITS;
        return pack->shift < 0;
}

static inline unsigned int
wordtoken6_take(kword_t *word)
{
        unsigned int token;

        token = (unsigned int)((*word >> WORDTOKEN6_FIRST_SHIFT) &
            WORDTOKEN6_MASK);
        *word <<= WORDTOKEN6_BITS;
        return token;
}

static inline void
wordtoken7_begin(struct wordtoken_pack *pack)
{
        pack->word = 0UL;
        pack->shift = WORDTOKEN7_FIRST_SHIFT;
}

static inline int
wordtoken7_put(struct wordtoken_pack *pack, unsigned int token)
{
        pack->word |= (kword_t)token << pack->shift;
        pack->shift -= (int)WORDTOKEN7_BITS;
        return pack->shift < 0;
}

static inline unsigned int
wordtoken7_take(kword_t *word)
{
        unsigned int token;

        token = (unsigned int)((*word >> WORDTOKEN7_FIRST_SHIFT) &
            WORDTOKEN7_MASK);
        *word <<= WORDTOKEN7_BITS;
        return token;
}

static inline void
wordtoken8_begin(struct wordtoken_pack *pack)
{
        pack->word = 0UL;
        pack->shift = WORDTOKEN8_FIRST_SHIFT;
}

static inline int
wordtoken8_put(struct wordtoken_pack *pack, unsigned int token)
{
        pack->word |= (kword_t)token << pack->shift;
        pack->shift -= (int)WORDTOKEN8_BITS;
        return pack->shift < 0;
}

static inline unsigned int
wordtoken8_take(kword_t *word)
{
        unsigned int token;

        token = (unsigned int)((*word >> WORDTOKEN8_FIRST_SHIFT) &
            WORDTOKEN8_MASK);
        *word <<= WORDTOKEN8_BITS;
        return token;
}

static inline void
wordtoken12_begin(struct wordtoken_pack *pack)
{
        pack->word = 0UL;
        pack->shift = WORDTOKEN12_FIRST_SHIFT;
}

static inline int
wordtoken12_put(struct wordtoken_pack *pack, unsigned int token)
{
        pack->word |= (kword_t)token << pack->shift;
        pack->shift -= (int)WORDTOKEN12_BITS;
        return pack->shift < 0;
}

static inline unsigned int
wordtoken12_take(kword_t *word)
{
        unsigned int token;

        token = (unsigned int)((*word >> WORDTOKEN12_FIRST_SHIFT) &
            WORDTOKEN12_MASK);
        *word <<= WORDTOKEN12_BITS;
        return token;
}

#endif
