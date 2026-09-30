/**
 * @file boot-sixbit.inc
 * @brief Install the fixed bootstrap SIXBIT helper at 077760..077776.
 *
 * The destination is bootstrap scratch storage only. Stage-1 code may call
 * 077760 after installation without retaining another relocatable helper near
 * the low-core decoder. AC17 must be a valid pushdown pointer; AC1 is clobbered.
 */
install_bootstrap_sixbit:
        movei 01,bootstrap_sixbit_image
        hrl 01,01
        hrri 01,077760
        blt 01,077776
        popj 017,

bootstrap_sixbit_image:
        .include "../common/sixbit-fixed.inc"
