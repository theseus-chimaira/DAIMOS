/**
 * @file card.h
 * @brief PDP-6 card-reader and card-punch hardware interface.
 *
 * The card reader (CR, device 0150) and card punch (CP, device 0110) are
 * independent optional devices but share the same 80-column card format and
 * native PI level 7.  KINIT probes each device separately and installs only
 * the corresponding MRES package, so an absent reader or punch consumes no
 * driver text or private transfer state in permanent memory.
 *
 * Both resident drivers expose only the packed WORDTOKEN12 whole-card
 * operations used by the native word-I/O ABI.  Runtime transfer is synchronous
 * and polled with the PIA disabled; one resident busy word prevents overlap.
 */
#ifndef DAIMON_CARD_H
#define DAIMON_CARD_H

#include "kcore_pi.h"

/** PDP-6 card-punch I/O device number. */
#define CP_DEVICE               0110U
/** PDP-6 card-reader I/O device number. */
#define CR_DEVICE               0150U
/** Architectural PI level shared by CR and CP. */
#define CARD_NATIVE_PI_LEVEL    7U
/** Number of physical columns in one card image. */
#define CARD_COLUMNS            80U
/** Low 12 bits of a card column; PDP-10 bits 24..35. */
#define CARD_COLUMN_MASK        07777UL
/** Packed WORDTOKEN12 words required for one complete 80-column card. */
#define CARD_WORDS              27U
/** Bounded polling count used while waiting for device progress. */
#define CARD_WAIT_READY         0200000U

/* CR CONO/CONI bits.  PDP-10 bit numbering is 0 at the sign bit. */
/** CR CONO bit 32: clear DATA READY. */
#define CR_CO_CLR_DRDY          0000010UL
/** CR CONO bit 31: clear END CARD. */
#define CR_CO_CLR_END_CARD      0000020UL
/** CR CONO bit 28: clear DATA MISS. */
#define CR_CO_CLR_DATA_MISS     0000200UL
/** CR CONO bit 26: begin reading one card. */
#define CR_CO_READ_CARD         0001000UL
/** CR CONO bit 23: clear/reset the reader. */
#define CR_CO_CLR_READER        0010000UL
/** CR CONI bits 33..35: assigned PI level. */
#define CR_ST_PI_MASK           0000007UL
/** CR CONI bit 32: one column is ready for DATAI. */
#define CR_ST_DATA_RDY          0000010UL
/** CR CONI bit 31: the current card has ended. */
#define CR_ST_END_CARD          0000020UL
/** CR CONI bit 29: reader is ready to start another card. */
#define CR_ST_RDY_READ          0000100UL
/** CR CONI bit 27: reader trouble condition. */
#define CR_ST_TROUBLE           0000400UL

/* CP CONO/CONI bits. */
/** CP CONO bit 30: start/enable punching. */
#define CP_CO_SET_PUNCH_ON      0000040UL
/** CP CONO bit 29: clear END CARD. */
#define CP_CO_CLR_END_CARD      0000100UL
/** CP CONO bit 28: enable END CARD interrupt generation. */
#define CP_CO_EN_END_CARD       0000200UL
/** CP CONO bit 26: clear punch error. */
#define CP_CO_CLR_ERROR         0001000UL
/** CP CONO bit 23: finish and eject the current card. */
#define CP_CO_EJECT             0010000UL
/** CP CONO bit 20: clear/reset punch status and enables. */
#define CP_CO_CLR_PUNCH         0100000UL
/** CP CONI bits 33..35: assigned PI level. */
#define CP_ST_PI_MASK           0000007UL
/** CP CONI bit 32: punch requests the next DATAO column. */
#define CP_ST_DATA_REQ          0000010UL
/** CP CONI bit 29: current card has completed/ejected. */
#define CP_ST_END_CARD          0000100UL
/** CP CONI bit 26: punch error condition. */
#define CP_ST_ERROR             0001000UL
/** CP CONI bit 24: punch trouble condition. */
#define CP_ST_TROUBLE           0004000UL

/** Invalid card buffer or argument. */
#define CARD_E_ARG             -1
/** Hardware did not make progress before the bounded wait expired. */
#define CARD_E_TIMEOUT         -2
/** Hardware reported an I/O/trouble condition. */
#define CARD_E_IO              -3
/** Another whole-card operation already owns the device. */
#define CARD_E_BUSY            -4
/** Card operation exceeded an implementation limit. */
#define CARD_E_LIMIT           -5

/** Read one card as 27 WORDTOKEN12 words; the final third token is zero. */
int cr_read_words(kword_t words[CARD_WORDS], unsigned int nwords);
/** Punch one complete 27-word WORDTOKEN12 card image. */
int cp_write_words(const kword_t words[CARD_WORDS], unsigned int nwords);


#endif
