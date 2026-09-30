/**
 * @file slv.h
 * @brief PDP-6 slave/interprocessor interrupt register definitions.
 *
 * The slave interface at device 020 represents another processor sharing main
 * memory with the PDP-6. DAIMOS needs no resident data-transfer path: shared
 * memory carries payloads, while this driver only acknowledges the incoming
 * interprocessor interrupt and preserves its PI assignment.
 */
#ifndef DAIMON_SLV_H
#define DAIMON_SLV_H

/** PDP-6 slave/interprocessor interrupt I/O device number. */
#define SLV_DEVICE          0020U
/** CONI/CONO bits 33..35: PI assignment. */
#define SLV_PI_MASK         0000007UL
/** Probe PI value written/read by KINIT to establish device presence. */
#define SLV_PROBE_PI        7U
/** Native runtime PI level used for slave interrupts. */
#define SLV_NATIVE_PI_LEVEL 7U
/** CONO bit 32: clear the pending interprocessor interrupt. */
#define SLV_CO_CLEAR_IRQ    0000010UL

#endif
