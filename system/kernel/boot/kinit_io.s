; kinit_io.s -- disposable KINIT bootstrap helpers.

        .text
        .globl kinit_put6
        .globl kinit_newline
        .globl kinit_error18
        .globl kinit_read_switches
        .globl kinit_call18
        .globl kinit_call18_1
        .globl kinit_halt
        .globl kinit_apr_clear

; void kinit_put6(kword_t word)
; Keep KINIT diagnostics inside the KINIT image.  Stage1 installs a fixed
; 077760 helper for its own failures, but the disposable KINIT stack grows
; upward from the loaded image and may legitimately overwrite that scratch
; area before late boot diagnostics run.
kinit_put6:
        move 02,01
        movei 06,06
kinit_put6_loop:
        move 03,02
        lsh 03,-036
        andi 03,077
        addi 03,040
        pushj 017,knl_putc
        lsh 02,06
        sojg 06,kinit_put6_loop
        jrst knl_wait

; void kinit_error18(kword_t code)
; Print one three-character SIXBIT halfword from AC1, then halt.  Keep this
; separate from kinit_put6(): fatal early-boot paths need neither six padding
; characters nor a CR/LF.
kinit_error18:
        move 02,01
        movei 06,03
kinit_error18_loop:
        move 03,02
        lsh 03,-014
        andi 03,077
        addi 03,040
        pushj 017,knl_putc
        lsh 02,06
        sojg 06,kinit_error18_loop
        pushj 017,knl_wait
kinit_error18_halt:
        halt .
        jrst kinit_error18_halt

; kword_t kinit_read_switches(void)
; PDP-6 APR DATAI exposes the 36-bit console switch register.
kinit_read_switches:
        datai 0000,01
        popj 017,

; Polling CR/LF, deliberately independent of CTY module state.
kinit_newline:
        movei 03,015
        pushj 017,knl_putc
        movei 03,012
knl_putc:
        pushj 017,knl_wait
        datao 0120,03
        popj 017,
knl_wait:
        coni 0120,04
        trne 04,0020
        jrst knl_wait
        popj 017,

; void kinit_call18(unsigned int address)
kinit_call18:
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

        .globl  kinit_call_blockset_request
kinit_call_blockset_request:
        move    4,1                    ; service address
        move    5,2                    ; four-word blockset request
        move    3,3(5)
        move    2,2(5)
        move    1,1(5)
        move    5,(5)                  ; operation, after final pointer use
        jrst    (4)

; Direct BLOCKSET I/O adapter used only while KINIT is resident.
; C ABI: address, logical block, buffer.  Installed root read/write exports use
; AC1=logical and AC2=buffer, so no permanent request dispatcher is needed.
        .globl  kinit_call_blockset_io
kinit_call_blockset_io:
        move    4,1
        move    1,2
        move    2,3
        jrst    (4)

        .globl  kinit_call_storage_io
kinit_call_storage_io:
        move    5,1                    ; service address
        move    1,2                    ; physical unit
        move    2,3                    ; physical block
        move    3,4                    ; 128-word buffer
        jrst    (5)

; kword_t kinit_call18_1(unsigned int address, kword_t arg)
kinit_call18_1:
        move 03,01
        move 01,02
        andi 03,0777777
        pushj 017,(03)
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

.if KINIT_STACK_WATERMARK
        .globl kinit_stack_watermark_begin
        .globl kinit_stack_watermark_measure
        .globl __kinit_image_end

; Fill the unused portion of the disposable KINIT pushdown list.  The caller's
; fixed frame and this PUSHJ return word are intentionally left unmarked so
; they count as stack use in the final high-water measurement.
kinit_stack_watermark_begin:
        hrrz 01,017
        addi 01,1
        movei 02,__kinit_image_end
        addi 02,KINIT_STACK_RESERVE_WORDS
        move 03,[0525252525252]
kinit_stack_watermark_fill:
        camle 01,02
        popj 017,
        movem 03,0(01)
        aoja 01,kinit_stack_watermark_fill

; Return the highest stack word touched, measured from __kinit_image_end.
; Scanning from the top tolerates arbitrary values in the initially live frame.
kinit_stack_watermark_measure:
        movei 02,__kinit_image_end
        addi 02,KINIT_STACK_RESERVE_WORDS
        movei 04,__kinit_image_end
        move 03,[0525252525252]
kinit_stack_watermark_scan:
        came 03,0(02)
        jrst kinit_stack_watermark_found
        camle 02,04
        sojg 02,kinit_stack_watermark_scan
        move 02,04
kinit_stack_watermark_found:
        sub 02,04
        move 01,02
        popj 017,
.endif
