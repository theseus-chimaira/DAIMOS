/**
 * @file load-check.inc
 * @brief Generic opaque-image header validation fragment for Stage-1 loaders.
 *
 * Inputs are AC2=magic and AC6=image_words,,entry_offset; the including source
 * supplies a `fail` label. Success leaves AC2=image words, AC3=entry offset,
 * and AC7=absolute entry. The generic load window is 040000..060000.
 * Current production compressed loaders use their own 030000-specific path;
 * this fragment remains a reusable/reference helper.
 */
; Common DAIMON opaque-image header validation.
;
; Inputs:
;   AC2 = magic word
;   AC6 = image_words,,entry_offset
;   fail = backend failure label
; Outputs:
;   AC2 = nonzero image word count
;   AC3 = entry offset
;   AC7 = absolute entry address
;
; The image load window is fixed at 040000..060000.
        came 02,daimon_magic
        jrst fail
        hlrz 02,06
        jumpe 02,fail
        hrrz 03,06
        caml 03,02
        jrst fail
        caile 02,020000
        jrst fail
        movei 07,040000
        add 07,03
