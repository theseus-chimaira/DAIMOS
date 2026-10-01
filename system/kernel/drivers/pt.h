/**
 * @file pt.h
 * @brief PDP-6 paper-tape reader and punch hardware interface.
 *
 * PTR device 0104 and PTP device 0100 are independent optional peripherals but
 * share the PDP-6 paper-tape status layout and native PI7 level. KINIT probes
 * and installs each MRES separately, so a machine carrying only one device
 * pays no resident RAM/code cost for the other.
 *
 * The resident interfaces are synchronous packed-word transfers.  KINIT uses
 * the PI assignment bits only while probing the devices; normal runtime I/O
 * polls the mechanical DONE/BUSY state with the PIA disabled.  Each driver
 * keeps one busy word so a second process cannot overlap a physical transfer.
 */
#ifndef DAIMON_PT_H
#define DAIMON_PT_H

#include "kcore_pi.h"

/*
 * Paper-tape word framing over the common WORDTOKEN8 layout.  Four-byte
 * groups are canonical WORDTOKEN8 words and therefore leave the low nibble
 * zero.  A final partial group stores its valid-byte count (1..3) in that
 * otherwise-unused nibble.  Values 4..15 are invalid encodings.
 */
#define PT_WORD_BYTES            4U
#define PT_WORD_COUNT_MASK       017UL
#define PT_WORD_PARTIAL_MAX      3U

/** Paper-tape punch I/O device number. */
#define PTP_DEVICE              0100U
/** Paper-tape reader I/O device number. */
#define PTR_DEVICE              0104U
/** Native PI level used while KINIT probes PTR and PTP. */
#define PT_NATIVE_PI_LEVEL      7U

/** CONI/CONO bit 32: operation complete/data ready. */
#define PT_ST_DONE              0010UL
/** CONI/CONO bit 31: reader/punch operation in progress. */
#define PT_ST_BUSY              0020UL
/** CONI/CONO bit 30: six-bit binary paper-tape mode. */
#define PT_ST_BINARY            0040UL
/** PTP CONI bit 29: no output tape/file attached. */
#define PTP_ST_NO_TAPE          0100UL
/** CONI/CONO bits 33..35: PI assignment. */
#define PT_ST_PI_MASK           0007UL
/** Bounded synchronous completion-wait count. */
#define PT_WAIT_READY           0200000U

#define PT_E_OK                 0
#define PT_E_ARG               -1
#define PT_E_TIMEOUT           -2
#define PT_E_BUSY              -3
#define PT_E_IO                -4

int ptr_read_words(kword_t *words, unsigned int nwords);
int ptp_write_words(const kword_t *words, unsigned int nwords);


#endif
