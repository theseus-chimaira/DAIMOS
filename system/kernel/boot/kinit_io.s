;/**
; * @file kinit_io.s
; * @brief Disposable KINIT console, call-adapter, APR, and stack helpers.
; *
; * These routines exist only while the transient KINIT image is resident.
; * They provide the minimum machine-level services needed before permanent
; * KCORE facilities are fully available: polling console output, compact
; * indirect calls into 18-bit service addresses, ABI adapters for resident
; * module exports, APR cleanup after memory probing, and optional bootstrap
; * stack watermarking.
; *
; * Register names are PDP-6/PDP-10 accumulators.  AC17 is the C pushdown
; * pointer; ordinary C arguments arrive in AC1, AC2, ... as noted below.
; */

        .text
        .globl kinit_put6
        .globl kinit_newline
        .globl kinit_error18
        .globl kinit_read_switches
        .globl kinit_call18
        .globl kinit_call18_1
        .globl kinit_halt
        .globl kinit_apr_clear

;/**
; * @brief Write one packed six-character SIXBIT word to the polling console.
; *
; * Keep KINIT diagnostics inside the KINIT image.  Stage1 installs a fixed
; * 077760 helper for its own failures, but the disposable KINIT stack grows
; * upward from the loaded image and may legitimately overwrite that scratch
; * area before late boot diagnostics run.
; *
; * Each SIXBIT character is converted to its display code by adding 040.
; * The routine waits for the final character to leave the polling interface
; * before returning.
; *
; * @param word AC1: packed six-character SIXBIT word.
; */
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

;/**
; * @brief Print a three-character SIXBIT fatal code and stop the machine.
; *
; * Keep this separate from kinit_put6(): fatal early-boot paths need neither
; * six padding characters nor a CR/LF.  The code occupies one 18-bit
; * halfword, hence the different initial shift and three-character loop.
; *
; * @param code AC1: three-character SIXBIT diagnostic halfword.
; */
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

;/**
; * @brief Read the physical console switch register.
; *
; * PDP-6 APR DATAI exposes the complete 36-bit switch word.  Root selection
; * masks the low selector bits in C; this helper intentionally returns the
; * unmodified hardware value.
; *
; * @return AC1: current 36-bit console switch word.
; */
kinit_read_switches:
        datai 0000,01
        popj 017,

;/**
; * @brief Emit CR/LF through the primitive polling console path.
; *
; * Deliberately independent of CTY module state so early diagnostics work
; * before resident terminal services have been installed.
; */
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

;/**
; * @brief Call a boot-time routine identified by an 18-bit word address.
; *
; * Masking the supplied value prevents stray high-half bits from affecting
; * the PDP-10 indirect call target.
; *
; * @param address AC1: 18-bit entry address.
; */
kinit_call18:
        andi 01,0777777
        pushj 017,(01)
        popj 017,

;/**
; * @brief Adapt a C filesystem request structure to the compact register ABI.
; *
; * The permanent filesystem service consumes its six request words directly
; * in accumulators.  Unpacking the transient C request here avoids carrying a
; * request dispatcher in permanent memory.
; *
; * @param address AC1: resident filesystem service address.
; * @param req AC2: address of the six-word filesystem request.
; * @return Service result in AC1 according to the resident service ABI.
; */
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

;/**
; * @brief Adapt a C blockset request structure to the compact register ABI.
; *
; * @param address AC1: resident blockset service address.
; * @param req AC2: address of the four-word blockset request.
; * @return Service result in AC1 according to the resident service ABI.
; */
        .globl  kinit_call_blockset_request
kinit_call_blockset_request:
        move    4,1                    ; service address
        move    5,2                    ; four-word blockset request
        move    3,3(5)
        move    2,2(5)
        move    1,1(5)
        move    5,(5)                  ; operation, after final pointer use
        jrst    (4)

;/**
; * @brief Adapt direct blockset I/O from the C ABI to the resident export ABI.
; *
; * Installed root read/write exports use AC1=logical block and AC2=buffer.
; * Keeping this shuffle in disposable KINIT means no permanent request
; * dispatcher is required.
; *
; * @param address AC1: resident blockset I/O service address.
; * @param logical AC2: logical block number.
; * @param buffer AC3: transfer buffer address.
; * @return Service result in AC1 according to the resident service ABI.
; */
        .globl  kinit_call_blockset_io
kinit_call_blockset_io:
        move    4,1
        move    1,2
        move    2,3
        jrst    (4)

;/**
; * @brief Adapt physical storage I/O from the C ABI to the resident export ABI.
; *
; * @param address AC1: resident storage service address.
; * @param unit AC2: physical unit number.
; * @param block AC3: physical block number.
; * @param buffer AC4: address of the 128-word transfer buffer.
; * @return Service result in AC1 according to the resident service ABI.
; */
        .globl  kinit_call_storage_io
kinit_call_storage_io:
        move    5,1                    ; service address
        move    1,2                    ; physical unit
        move    2,3                    ; physical block
        move    3,4                    ; 128-word buffer
        jrst    (5)

;/**
; * @brief Call an 18-bit boot-time entry with one argument.
; *
; * The C wrapper receives the target in AC1 and the service argument in AC2;
; * the target routine itself expects its first argument in AC1.
; *
; * @param address AC1: 18-bit entry address.
; * @param arg AC2: argument passed to the target as AC1.
; * @return Target routine result in AC1.
; */
kinit_call18_1:
        move 03,01
        move 01,02
        andi 03,0777777
        pushj 017,(03)
        popj 017,

;/**
; * @brief Clear the APR condition left by an intentional NXM memory probe.
; *
; * The probe runs before the line clock is enabled, so clearing the shared APR
; * flag here cannot lose a clock event.
; */
kinit_apr_clear:
        cono 0000,010000
        popj 017,

;/**
; * @brief Stop early boot permanently.
; *
; * The self-loop after HALT ensures that continuing the processor cannot fall
; * through into unrelated bootstrap code.
; */
kinit_halt:
        halt .
        jrst kinit_halt

.if KINIT_STACK_WATERMARK
        .globl kinit_stack_watermark_begin
        .globl kinit_stack_watermark_measure
        .globl __kinit_image_end

;/**
; * @brief Fill the unused KINIT pushdown list with watermark sentinels.
; *
; * The caller's fixed frame and this PUSHJ return word are intentionally left
; * unmarked so they count as stack use in the final high-water measurement.
; */
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

;/**
; * @brief Measure maximum KINIT stack use from the watermark.
; *
; * Scanning from the top tolerates arbitrary values in the initially live
; * frame.
; *
; * @return AC1: highest stack word touched, measured from __kinit_image_end.
; */
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
