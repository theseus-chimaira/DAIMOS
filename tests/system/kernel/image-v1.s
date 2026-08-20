; image-v1.s -- KINIT V0.1 relocation/early-KCORE test image prefix.
;
; The host-side builder patches the address-bearing manifest words after link.
; The actual KCORE image is linked separately for address 060 and embedded
; immediately after this manifest by kcore-embed-v1.s.

        .text
        .globl __kinit_image_start

__kinit_image_start:
        .word 0535541562021        ; SIXBIT /KMAN01/
        .word 000000000004        ; 0 modules,,4 manifest words
        .word 000000000000        ; patched: KCORE source,,size
        .word 000000000000        ; patched: KINIT begin,,size

