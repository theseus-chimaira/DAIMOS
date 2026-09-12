; kinit_io.s -- disposable KINIT bootstrap helpers.

        .text
        .globl kinit_put6
        .globl kinit_newline
        .globl kinit_call18
        .globl kinit_call18_0
        .globl kinit_call18_1
        .globl kinit_call18_2
        .globl kinit_call18_3
        .globl kinit_halt
        .globl kinit_apr_clear

; void kinit_put6(kword_t word)
kinit_put6:
        pushj 017,077760
        popj 017,

; Polling CR/LF, deliberately independent of CTY module state.
kinit_newline:
        movei 03,015
        pushj 017,knl_putc
        movei 03,012
knl_putc:
        coni 0120,04
        trne 04,0020
        jrst knl_putc
        datao 0120,03
        popj 017,

; void kinit_call18(unsigned int address)
kinit_call18:
        andi 01,0777777
        pushj 017,(01)
        popj 017,

; kword_t kinit_call18_0(unsigned int address)
kinit_call18_0:
        andi 01,0777777
        pushj 017,(01)
        popj 017,

; KINIT-only adapters for modules whose permanent export is the compact
; register ABI.  Keeping request unpacking here makes it reclaimable.
        .globl  kinit_call_fs_request
kinit_call_fs_request:
        move    7,1                    ; service address
        move    6,2                    ; six-word fs request
        move    5,5(6)
        move    4,4(6)
        move    3,3(6)
        move    2,2(6)
        move    1,1(6)
        move    6,(6)                  ; operation, after final pointer use
        jrst    (7)

        .globl  kinit_call_diskset_request
kinit_call_diskset_request:
        move    4,1                    ; service address
        move    5,2                    ; four-word diskset request
        move    3,3(5)
        move    2,2(5)
        move    1,1(5)
        move    5,(5)                  ; operation, after final pointer use
        jrst    (4)

; kword_t kinit_call18_1(unsigned int address, kword_t arg)
kinit_call18_1:
        move 03,01
        move 01,02
        andi 03,0777777
        pushj 017,(03)
        popj 017,

; kword_t kinit_call18_2(unsigned int address, kword_t arg1, kword_t arg2)
kinit_call18_2:
        move 04,01
        move 01,02
        move 02,03
        andi 04,0777777
        pushj 017,(04)
        popj 017,

; kword_t kinit_call18_3(unsigned int address, kword_t arg1, kword_t arg2,
;     kword_t arg3)
kinit_call18_3:
        move 05,01
        move 01,02
        move 02,03
        move 03,04
        andi 05,0777777
        pushj 017,(05)
        popj 017,

; Clear the APR flag left by an intentional nonexistent-memory probe.
; The probe runs before the line clock is enabled, so clearing the shared APR
; flag here cannot lose a clock event.
kinit_apr_clear:
        cono 0000,010000
        popj 017,

kinit_halt:
        halt .
        jrst kinit_halt
