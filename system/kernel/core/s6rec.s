/**
 * @file s6rec.s
 * @brief Compact resident S6REC text-frame validation shared by word devices.
 *
 * TTY and LPT are independently movable MRES packages, but both must validate
 * exactly one complete S6REC TEXT record before emitting device output.  Keep
 * that format check once in KCORE only because the measured whole-image result
 * is smaller than carrying both copies in the MRES packages.
 */

        .text
        .globl  s6rec_text_validate
        .globl  kret_neg1

/**
 * @brief Validate one complete S6REC TEXT frame for word-device output.
 * @param AC1 Address of the first record word.
 * @param AC2 Supplied record length in 36-bit words.
 * @return AC1 Character count on success, or -1 for malformed type/length.
 * @return AC2 POINT 6 byte pointer to the first payload word on success.
 *
 * AC3..AC6 are scratch.  The routine accepts exactly one TEXT record: the
 * supplied word count must match the character count encoded in its header.
 */
s6rec_text_validate:
        move    6,1
        move    3,(1)
        ldb     4,[POINT 6,3,5]
        caie    4,1
        jrst    kret_neg1
        and     3,[077777777]
        move    4,3
        addi    4,5
        idivi   4,6
        addi    4,1
        came    4,2
        jrst    kret_neg1
        move    2,[POINT 6,0]
        movei   5,1(6)
        hrr     2,5
        move    1,3
        popj    17,
