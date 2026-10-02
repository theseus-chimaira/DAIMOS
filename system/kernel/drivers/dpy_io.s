/**
 * @file dpy_io.s
 * @brief Resident interrupt-driven PDP-6 Type 340 display driver.
 *
 * DPY device 0130 shares PI6 with the APR line clock. MINIT patches the right
 * half of dpy_clk_pi_service_call with the relocated CLK service routine and
 * replaces the clock PI-table entry with dpy_pi_handler. If CLK is absent, the
 * call remains pointed at kret_ok, making the shared entry a cheap no-op before
 * Type 340 status is tested.
 *
 * Exactly one display word may be in flight. dpy_pending is set before DATAO
 * and cleared only by a real DONE interrupt, so a caller cannot observe
 * completion before the Type 340 has executed both 18-bit halves. The one-word
 * BSS flag and this code exist only when the optional DPY MRES is installed.
 */

        .text
        .globl dpy_pi_handler
        .globl dpy_putword
        .globl dpy_putchar
        .globl dpy_clk_pi_service_call
        .globl pdp10_pi_handler_return
        .globl kret_ok
        .globl kret_busy

/**
 * @brief Service the shared CLK/DPY PI6 vector.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * The patched clock service executes first and preserves the generic PI ABI.
 * DPY then tests DONE. AC1 may be clobbered by the clock service; AC2, AC3,
 * and AC17 remain valid for the dispatcher. A display DONE clears dpy_pending
 * and acknowledges PI6 with CONO 6 before returning through the shared stub.
 */
dpy_pi_handler:
dpy_clk_pi_service_call:
        pushj 017,kret_ok
dpy_pi_display:
        conso 0130,000200
        jrst pdp10_pi_handler_return
        setzm dpy_pending
        cono 0130,000006
        jrst pdp10_pi_handler_return

/**
 * @brief Submit one Type 340 word and synchronously await interrupt completion.
 * @param AC1 One 36-bit word containing two Type 340 instructions.
 * @return AC1 = DPY_E_OK (0) or DPY_E_BUSY (-3).
 *
 * AC17 is only the normal return stack. No scratch AC is needed. Setting the
 * pending flag before DATAO closes the completion race; the spin loop exits
 * only after dpy_pi_handler observes DONE and clears the flag.
 */
dpy_putword:
        skipe dpy_pending
        jrst kret_busy
dpy_put_start:
        setom dpy_pending
        datao 0130,1
dpy_put_wait:
        skipe dpy_pending
        jrst dpy_put_wait
dpy_put_ok:
        jrst kret_ok

/**
 * @brief Render one terminal byte with the Type 342 character generator.
 * @param AC1 ASCII byte.
 * @return dpy_putword() status.
 *
 * Every call is self contained: PARAM->CHAR, SI/SO + character/control + ESC.
 * Thus no cursor/shift state is retained in resident RAM.  Type 342 CR/LF are
 * native controls; shifted code 072 is the six-unit cursor-left used for BS.
 */
dpy_putchar:
        andi 1,0377
        movei 2,035                  ; SI / primary character set
        caie 1,010
        jrst dpy_putchar_lf
        movei 2,036                  ; SO / shifted set
        movei 1,072                  ; cursor left six units
        jrst dpy_putchar_pack
dpy_putchar_lf:
        caie 1,012
        jrst dpy_putchar_cr
        movei 1,033
        jrst dpy_putchar_pack
dpy_putchar_cr:
        caie 1,015
        jrst dpy_putchar_lower
        movei 1,034
        jrst dpy_putchar_pack
dpy_putchar_lower:
        caige 1,0141
        jrst dpy_putchar_primary
        caile 1,0172
        jrst dpy_putchar_bad
        subi 1,0140
        movei 2,036                  ; SO / lower-case set
        jrst dpy_putchar_pack
dpy_putchar_primary:
        caige 1,040
        jrst dpy_putchar_bad
        caile 1,077
        jrst dpy_putchar_upper
        jrst dpy_putchar_pack
dpy_putchar_upper:
        caige 1,0101
        jrst dpy_putchar_bad
        caile 1,0132
        jrst dpy_putchar_bad
        andi 1,077                   ; A..Z -> Type-342 codes 1..032
        jrst dpy_putchar_pack
dpy_putchar_bad:
        movei 1,077                  ; unsupported byte -> '?'
dpy_putchar_pack:
        lsh 2,014                    ; first Type-342 character
        lsh 1,6
        ior 1,2
        ori 1,037                    ; ESC returns display to parameter mode
        hrli 1,060000                ; left half: PARAM -> CHAR mode
        jrst dpy_putword

        .bss
/** Nonzero while one DATAO word is awaiting the Type 340 DONE interrupt. */
dpy_pending:
        .block 1
