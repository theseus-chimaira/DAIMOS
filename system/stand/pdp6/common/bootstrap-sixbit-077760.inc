; Install the fixed bootstrap SIXBIT helper at 077760..077777.
; AC17 must be a valid pushdown pointer before entry.
; Clobbers AC1.
install_bootstrap_sixbit:
        movei 01,bootstrap_sixbit_image
        hrl 01,01
        hrri 01,077760
        blt 01,077777
        popj 017,

bootstrap_sixbit_image:
        .include "../common/sixbit-output-077760.inc"
