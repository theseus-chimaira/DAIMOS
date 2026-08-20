; image-v1.s -- KINIT V0.1 pre-relocation test image prefix.
;
; The host-side builder patches the two address-bearing manifest words after
; link relocation is known.  The manifest intentionally has no modules yet.

        .text
        .globl __kinit_image_start
        .globl __kinit_code_start
        .globl test_kcore

__kinit_image_start:
        .word 0535541562021        ; SIXBIT /KMAN01/
        .word 000000000004        ; 0 modules,,4 manifest words
        .word 000000000000        ; patched: KCORE source,,size
        .word 000000000000        ; patched: KINIT begin,,size

test_kcore:
        .word 000000000000

__kinit_code_start:
