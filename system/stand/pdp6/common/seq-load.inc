/**
 * @file seq-load.inc
 * @brief Generic sequential-device Stage-1 body for opaque images.
 *
 * The including backend supplies `read_words`: AC1=destination, AC2=nonzero
 * word count. This fragment validates the header, loads the image at 040000,
 * sets AC17=070000, clears AC1/AC2, and transfers to the validated entry.
 * It is not used by the current compressed DTC/MTC/PTR production loaders.
 */
; Common Stage1 body for sequential opaque-image boot devices.
;
; Backend contract:
;   read_words: AC1 = destination, AC2 = nonzero word count.
;               Return after storing exactly AC2 words.
;
; The including backend initializes its device before entering this body.

        movei 01,header
        movei 02,02
        pushj 017,read_words
        move 02,header
        move 06,header+01
        .include "../common/load-check.inc"
        movei 01,040000
        pushj 017,read_words
        movei 017,070000
        setzb 01,02
        jrst 0(07)
