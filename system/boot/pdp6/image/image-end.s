/**
 * @file image-end.s
 * @brief Link-layout marker for the end of the complete KINIT image.
 *
 * This zero-size BSS symbol is linked last.  Image construction uses
 * __kinit_image_end to validate the final KINIT span and available stack/core
 * headroom; it contains no runtime code or storage of its own.
 */
        .bss
        .globl __kinit_image_end
__kinit_image_end:
