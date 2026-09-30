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
