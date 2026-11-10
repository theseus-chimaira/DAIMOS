/* Staged-only PDP-6 KINIT image/staging geometry validator v1364.
 * Not included by production Stage-1 until atomic grouped cutover.
 *
 * Inputs: AC1=decoded word count (>0), AC2=compressed word count (0 raw),
 *         AC3=configured physical core words, AC4=KINIT stack reserve.
 * Output: AC0=0 valid, -1 invalid; all other ACs preserved.
 * AC17 is used for the ordinary PUSHJ/POPJ return word only.
 *
 * Valid when [030000,030000+decoded) leaves reserved KINIT stack and
 * when [030000+decoded,030000+decoded+compressed) is wholly in core.
 * Since halfword header lengths are <=0777777, all sums fit 36 bits.
 * The caller checks header entry offset and compression mode separately.
 */
        .text
        .globl d6stage_geometry_v1364
d6stage_geometry_v1364:
        jumpe   1,d6stage_geometry_bad_v1364
        move    0,1
        addi    0,030000
        add     0,4
        camle   0,3
        jrst    d6stage_geometry_bad_v1364
        sub     0,4
        add     0,2
        camle   0,3
        jrst    d6stage_geometry_bad_v1364
        setz    0,
        popj    17,
d6stage_geometry_bad_v1364:
        seto    0,
        popj    17,
