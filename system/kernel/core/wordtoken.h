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
 * There is deliberately no callable or inline reference codec here.  The
 * hardware-facing assembly drivers implement these fixed layouts directly,
 * avoiding duplicate source implementations and any generic runtime width
 * selection.  The constants below are the authoritative source-level layout
 * contract for C code which needs to describe the formats.
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

#endif
