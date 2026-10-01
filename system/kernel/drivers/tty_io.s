/**
 * @file tty_io.s
 * @brief Resident logical-terminal dispatcher over CTY, DCS, and GE backends.
 *
 * Logical terminal 0 is CTY, 1..16 map to DCS lines 0..15, and 17..20 map to
 * GE consoles 0..3. MINIT patches the six backend tail jumps after the
 * corresponding physical MRES packages are installed. Missing backends retain
 * kret_arg, making an unavailable physical device indistinguishable from an
 * invalid logical id at this lowest dispatch layer.
 *
 * All backend calls are tail jumps: the physical driver returns directly to
 * the original caller, so TTY adds no stack frame or resident call wrapper.
 */

        .text
        .globl tty_putchar
        .globl tty_getchar
        .globl tty_cty_putchar_address
        .globl tty_dcs_putchar_address
        .globl tty_ge_putchar_address
        .globl tty_cty_getchar_address
        .globl tty_dcs_getchar_address
        .globl tty_ge_getchar_address
        .globl kret_arg
        .globl proc_tty_output
        .globl proc_tty_read_enter
        .globl proc_tty_input
        .globl proc_tty_line_base_get
        .globl proc_tty_line_ensure
        .globl proc_tty_line_reset
        .globl proc_tty_records

/**
 * @brief Dispatch one packed logical-terminal output byte.
 * @param AC1 TTY_PACK(id, byte).
 * @return Directly from the selected backend or kret_arg.
 *
 * AC2 is scratch for the six-bit logical id. DCS and GE ids are translated in
 * place to the zero-based packed line format expected by those drivers. One
 * MonitorFS logical-TTY output operation is counted before a valid tail jump.
 */
tty_putchar:
        ldb 2,[POINT 6,1,27]
        jumpe 2,tty_putchar_cty
        caile 2,020
        jrst tty_putchar_ge
        subi 1,0400
tty_dcs_putchar_address:
        jrst kret_arg

tty_putchar_ge:
        caile 2,024
        jrst kret_arg
        subi 1,010400
tty_ge_putchar_address:
        jrst kret_arg

tty_putchar_cty:
tty_cty_putchar_address:
        jrst kret_arg

/**
 * @brief Dispatch one blocking logical-terminal input request.
 * @param AC1 Logical TTY id 0..20.
 * @return Directly from the selected physical backend or kret_arg.
 *
 * DCS/GE ids are translated in AC1 to zero-based physical line numbers. CTY
 * receives no line argument. No additional state or buffer is owned by TTY.
 */
tty_getchar:
        jumpe 1,tty_getchar_cty
        caile 1,020
        jrst tty_getchar_ge
        subi 1,1
tty_dcs_getchar_address:
        jrst kret_arg

tty_getchar_ge:
        caile 1,024
        jrst kret_arg
        subi 1,021
tty_ge_getchar_address:
        jrst kret_arg

tty_getchar_cty:
tty_cty_getchar_address:
        jrst kret_arg

; int tty_write_s6rec(const kword_t *words, unsigned int nwords)
; Render exactly one complete S6REC text record to the caller's controlling
; logical TTY.  The caller supplies the whole record; raw terminal bytes
; continue to use WRITECHAR and never pass through this decoder.
        .globl tty_write_s6rec
tty_write_s6rec:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1                   ; record base
        hrrz    011,2                   ; supplied/return word count
        jumpe   011,tty_s6_zero

        ; Resolve/validate the controlling TTY once per call.  Keep only its
        ; packed id prefix; the probe space is not emitted.
        movei   1,040
        pushj   17,proc_tty_output
        jumpl   1,tty_s6_bad
        andi    1,037400                ; TTY_ID_MASK << 8
        move    012,1

        move    3,(010)
        ldb     4,[POINT 6,3,5]
        caie    4,1                     ; S6REC TEXT
        jrst    tty_s6_bad
        and     3,[077777777]           ; character count
        move    013,3

        ; One divide per record validates the complete frame before output.
        move    4,3
        addi    4,5
        idivi   4,6                     ; ceil(chars/6)
        addi    4,1                     ; header + payload words
        came    4,011                   ; exactly one complete record
        jrst    tty_s6_bad

        move    014,[POINT 6,0]
        movei   7,1(010)
        hrr     014,7
        jumpe   013,tty_s6_eol
tty_s6_char_loop:
        ildb    2,014
        addi    2,040                   ; SIXBIT -> terminal ASCII
        move    1,012
        ior     1,2
        pushj   17,tty_putchar
        jumpn   1,tty_s6_bad
        sojg    013,tty_s6_char_loop

tty_s6_eol:
        move    1,012
        ori     1,015
        pushj   17,tty_putchar
        jumpn   1,tty_s6_bad
        move    1,012
        ori     1,012
        pushj   17,tty_putchar
        jumpn   1,tty_s6_bad

        move    1,011
        jrst    tty_s6_done
tty_s6_zero:
        setz    1,
        jrst    tty_s6_done
tty_s6_bad:
        seto    1,
tty_s6_done:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int tty_read_s6rec(kword_t *words, unsigned int nwords)
; Return one complete canonical line as one S6REC TEXT record.  Canonical
; submission is non-draining: READCHAR drains through proc_tty_line_take,
; while this routine copies an untouched READY line in bulk.  RAW terminals
; reject word reads and continue to use READCHAR.
        .globl tty_read_s6rec
tty_read_s6rec:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        move    011,1                   ; destination
        hrrz    012,2                   ; capacity in words
        jumpe   011,tty_s6_read_bad
        jumpe   012,tty_s6_read_zero

        pushj   17,proc_tty_read_enter
        jumpl   1,tty_s6_read_bad
        move    010,1                   ; logical controlling tty
        move    3,proc_tty_records(010)
        lsh     3,-031                  ; mode bits at 25..27
        trnn    3,01                    ; canonical mode required
        jrst    tty_s6_read_bad
        move    1,010
        pushj   17,proc_tty_line_ensure
        jumpe   1,tty_s6_read_bad

tty_s6_read_check:
        move    1,010
        pushj   17,proc_tty_line_base_get
        jumpe   1,tty_s6_read_not_ready
        move    013,1                   ; canonical line base
        move    6,(013)
        trnn    6,0200000               ; READY
        jrst    tty_s6_read_not_ready

        move    015,6
        andi    015,0377                ; line character count
        jumpn   015,tty_s6_read_have_line
        tlne    6,1                     ; bit 18 EOF: low bit of left half
        jrst    tty_s6_read_eof
tty_s6_read_have_line:
        move    4,015
        addi    4,5
        idivi   4,6
        move    014,4                   ; packed payload words
        addi    4,1                     ; header + payload
        camle   4,012
        jrst    tty_s6_read_bad         ; leave READY line untouched
        move    012,4                   ; final return count

        move    5,[010000000000]        ; S6REC TEXT type 1
        ior     5,015
        movem   5,(011)
        jumpe   014,tty_s6_read_reset
        movei   5,1(011)
        hrli    5,1(013)
        move    6,014
        add     6,011
        blt     5,0(6)
tty_s6_read_reset:
        move    1,010
        pushj   17,proc_tty_line_reset
        move    1,012
        jrst    tty_s6_read_done

tty_s6_read_eof:
        move    1,010
        pushj   17,proc_tty_line_reset
        setz    1,
        jrst    tty_s6_read_done

tty_s6_read_not_ready:
        ; Hardware input may block/context-switch.  Preserve bulk-read state on
        ; the process-private kernel stack; resumed scratch registers are not
        ; an ABI storage location.
        push    17,010
        push    17,011
        push    17,012
        move    1,010
        pushj   17,tty_getchar
        push    17,1
        move    012,-1(17)
        move    011,-2(17)
        move    010,-3(17)
        pop     17,1
        sub     17,[3,,3]
        jumpl   1,tty_s6_read_bad
        move    2,1
        move    1,010
        pushj   17,proc_tty_input
        camn    1,[-3]                  ; editing, signal, or READY line
        jrst    tty_s6_read_check
        camn    1,[-2]                  ; historical empty EOF path
        jrst    tty_s6_read_zero
        jumpl   1,tty_s6_read_bad

        ; Historical NO_TTY/CTY input bypasses canonical storage and returns
        ; one raw byte.  Preserve that bootstrap behavior without carrying
        ; fallback state through the normal attached-TTY path.
        move    015,1
        caie    015,012                 ; newline => empty text record
        jrst    tty_s6_read_single
        movei   3,1
        camg    3,012
        jrst    tty_s6_read_empty_store
        jrst    tty_s6_read_bad
tty_s6_read_empty_store:
        move    3,[010000000000]
        movem   3,(011)
        movei   1,1
        jrst    tty_s6_read_done
tty_s6_read_single:
        movei   3,2
        camg    3,012
        jrst    tty_s6_read_single_store
        jrst    tty_s6_read_bad
tty_s6_read_single_store:
        move    3,[010000000001]
        movem   3,(011)
        subi    015,040
        lsh     015,036
        movem   015,1(011)
        movei   1,2
        jrst    tty_s6_read_done

tty_s6_read_zero:
        setz    1,
        jrst    tty_s6_read_done
tty_s6_read_bad:
        seto    1,
tty_s6_read_done:
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
