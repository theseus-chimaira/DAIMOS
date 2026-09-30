/**
 * @file pt.h
 * @brief PDP-6 paper-tape reader and punch hardware interface.
 *
 * PTR device 0104 and PTP device 0100 are independent optional peripherals but
 * share the PDP-6 paper-tape status layout and native PI7 level. KINIT probes
 * and installs each MRES separately, so a machine carrying only one device
 * pays no resident RAM/code cost for the other.
 *
 * PTR uses one state word as idle/request-active/prefetched-byte storage. PTP
 * uses one state word as an in-flight completion flag. Both services are
 * synchronous to callers but use PI7 to observe hardware completion.
 */
#ifndef DAIMON_PT_H
#define DAIMON_PT_H

#include "kcore_pi.h"

/** Paper-tape punch I/O device number. */
#define PTP_DEVICE              0100U
/** Paper-tape reader I/O device number. */
#define PTR_DEVICE              0104U
/** Native PI level used independently by PTR and PTP. */
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

/**
 * @brief Read one eight-bit paper-tape byte.
 * @param cp Destination for the byte value 0..0377.
 * @return PT_E_OK, PT_E_ARG, PT_E_BUSY, or PT_E_TIMEOUT.
 */
int ptr_getchar(int *cp);
/** @brief Resident PI7 completion/prefetch entry for PTR. */
void ptr_pi_handler(void);

/**
 * @brief Punch one eight-bit paper-tape byte.
 * @param c Byte value; only the low eight bits are transmitted.
 * @return PT_E_OK, PT_E_BUSY, PT_E_IO, or PT_E_TIMEOUT.
 */
int ptp_putchar(int c);
/** @brief Resident PI7 completion entry for PTP. */
void ptp_pi_handler(void);


#endif
