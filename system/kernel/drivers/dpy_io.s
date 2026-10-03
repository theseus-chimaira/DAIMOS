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
 * dpy_putword() retains the historical one-word synchronous path.  Raw vector
 * applications may instead submit a complete display list with
 * dpy_write_words(); the list is copied into dynamically allocated core and
 * recycled by this same DONE interrupt.  No fixed display-list buffer is
 * reserved in KCORE or the DPY MRES.
 */

        .text
        .globl dpy_pi_handler
        .globl dpy_putword
        .globl dpy_write_words
        .globl dpy_putchar
        .globl dpy_clk_pi_service_call
        .globl pdp10_pi_handler_return
        .globl kret_ok
        .globl kret_busy
        .globl mm_alloc
        .globl mm_free

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
        skipn dpy_list_base
        jrst dpy_pi_single_done

        ; Persistent-list mode.  dpy_list_next is one-past the word most
        ; recently submitted.  Wrap at dpy_list_end and feed exactly one new
        ; packed word per DONE interrupt.  AC1 is the only dispatcher scratch
        ; register available here.
        move 1,dpy_list_next
        came 1,dpy_list_end
        jrst dpy_pi_list_have
        move 1,dpy_list_base
dpy_pi_list_have:
        addi 1,1
        movem 1,dpy_list_next
        subi 1,1
        move 1,(1)
        datao 0130,1
        cono 6,0
        jrst pdp10_pi_handler_return

dpy_pi_single_done:
        setzm dpy_pending
        cono 6,0
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
        skipn dpy_list_base
        jrst dpy_putword_idle
        jrst kret_busy
dpy_putword_idle:
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
 * @brief Replace/stop the persistent Type-340 display list.
 * @param AC1 Source packed-word address; ignored when AC2 is zero.
 * @param AC2 Packed 36-bit word count; zero stops the current list.
 * @return AC1 = accepted word count, or -1 on failure.
 *
 * Replacement is transactional with respect to allocation: the old list keeps
 * refreshing while the new list is allocated and copied.  Only after the copy
 * succeeds is DPY reset, the new state published, and the old extent freed.
 * The controller PIA is disabled during publication so the PI handler can
 * never observe a partially initialized list.
 */
dpy_write_words:
        push 17,010
        push 17,011
        move 010,1                    ; mapped source
        move 011,2                    ; requested packed words
        jumpe 011,dpy_list_stop
        jumpe 010,dpy_list_fail

        ; Allocate exactly the submitted list size.  Owner 014 is reserved for
        ; DPY dynamic list storage; no display RAM is consumed while idle.
        push 17,[0]                   ; returned base
        movei 5,(17)
        push 17,5                     ; fifth mm_alloc arg: basep
        move 1,011
        movei 2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei 3,014                   ; DPY_LIST_MM_OWNER
        setz 4,                       ; MM_ALLOC_LOW
        pushj 17,mm_alloc
        sub 17,[1,,1]
        jumpn 1,dpy_list_alloc_fail
        move 6,(17)                   ; new list base
        jumpe 6,dpy_list_alloc_fail

        ; Copy the mapped userspace words before taking over the controller.
        setz 1,
        hrl 1,010                     ; source,,destination BLT pointer
        hrr 1,6
        move 2,6
        add 2,011
        subi 2,1
        blt 1,(2)

        ; Disable the DPY PIA and reset execution before changing live state.
        move 7,dpy_list_base           ; old extent, if any
        setzm dpy_list_base
        cono 0130,000100              ; INIT, PIA disabled
        setzm dpy_pending
        move 1,6
        add 1,011
        movem 1,dpy_list_end
        movei 1,1(6)
        movem 1,dpy_list_next
        movem 6,dpy_list_base          ; publish only after next/end are valid
        move 1,(6)
        datao 0130,1                   ; start first packed display word
        cono 0130,000006              ; normal DPY DONE PIA

        ; The interrupt path now references only the new extent.
        jumpe 7,dpy_list_installed
        move 1,7
        movei 2,3
        movei 3,014
        pushj 17,mm_free
dpy_list_installed:
        move 1,011
        sub 17,[1,,1]                  ; allocation-result local
        jrst dpy_list_done

dpy_list_alloc_fail:
        sub 17,[1,,1]                  ; allocation-result local
dpy_list_fail:
        seto 1,
        jrst dpy_list_done

dpy_list_stop:
        move 7,dpy_list_base
        setzm dpy_list_base            ; PI handler must stop recycling first
        cono 0130,000100               ; INIT, PIA disabled
        setzm dpy_list_next
        setzm dpy_list_end
        setzm dpy_pending
        cono 0130,000006               ; leave normal DPY PIA armed
        jumpe 7,dpy_list_stopped
        move 1,7
        movei 2,3
        movei 3,014
        pushj 17,mm_free
dpy_list_stopped:
        setz 1,
dpy_list_done:
        pop 17,011
        pop 17,010
        popj 17,

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
/** Base of the active dynamically allocated packed display list, or zero. */
dpy_list_base:
        .block 1
/** Address of the next packed word to submit on DONE. */
dpy_list_next:
        .block 1
/** One-past-end address of the active packed display list. */
dpy_list_end:
        .block 1
