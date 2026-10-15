/* v062 experimental grouped D6 codec VFS adapter. NOT linked into KCORE.
 * ABI: AC1 vnode, AC2 source offset, AC3 exact compressed words,
 * AC4 decoded_words,,destination. Returns AC1=0 success, -1 failure.
 * Uses a single 64-word stack window. Only complete groups reach v061.
 * Reader and decoder may clobber AC0..AC16. AC10..AC16 restored.
 * Input and output must be disjoint; owner verifies destination geometry.
 */
        .text
        .globl d6codec_vfs_v062
        .globl d6codec_v061_decode
        .globl vfs_read_words
        .equ CV_WINDOW,0100
        .equ CV_FRAME, CV_WINDOW+023
        .equ CV_VNODE,010-CV_FRAME
        .equ CV_OFFSET,011-CV_FRAME
        .equ CV_LEFT,012-CV_FRAME
        .equ CV_USED,013-CV_FRAME
        .equ CV_CURSOR,014-CV_FRAME
        .equ CV_END,015-CV_FRAME
        .equ CV_ORIGIN,016-CV_FRAME
        .equ CV_GROUP,017-CV_FRAME
        .equ CV_INPUT,024-CV_FRAME

d6codec_vfs_v062:
        add     17,[CV_FRAME,,CV_FRAME]
        movei   0,1-CV_FRAME(17)
        hrli    0,010
        blt     0,7-CV_FRAME(17)
        movem   1,CV_VNODE(17)
        movem   2,CV_OFFSET(17)
        movem   3,CV_LEFT(17)
        hrrz    1,4
        hlrz    2,4
        jumpe   1,cv_bad
        jumpe   2,cv_bad
        jumpe   3,cv_bad
        movem   1,CV_ORIGIN(17)
        movem   1,CV_CURSOR(17)
        ; The output span must fit in the 18-bit PDP-6 address space.
        ; Do not let arithmetic wrap or reach the first word beyond core.
        move    0,1
        add     0,2
        camle   0,[01000000]
        jrst    cv_bad
        add     2,1
        movem   2,CV_END(17)
        setzm   CV_USED(17)
cv_next:
        skipg   CV_USED(17)
        jrst    cv_refill
        ; AC1 input window origin, AC2 buffered words.
        movei   1,CV_INPUT(17)
        move    2,CV_USED(17)
        move    6,0(1)
        move    7,6
        lsh     7,-036
        jumpe   7,cv_bad
        caile   7,036
        jrst    cv_bad
        move    0,6
        lsh     0,6
        setz    4,                       ; number of match descriptors
        move    3,7
cv_count:
        skipge  0
        aoj     4,
        lsh     0,1
        sojg    3,cv_count
        move    3,4
        aoj     3,
        lsh     3,-1                      ; paired descriptor words
        add     3,7
        sub     3,4
        aoj     3,                       ; group includes control word
        movem   3,CV_GROUP(17)
        camle   3,2
        jrst    cv_refill
        ; Call v061 on a single fully buffered group.
        movei   1,CV_INPUT(17)
        move    2,CV_GROUP(17)
        move    3,CV_CURSOR(17)
        move    4,CV_END(17)
        move    5,CV_ORIGIN(17)
        pushj   17,d6codec_v061_decode
        jumpl   1,cv_bad
        movem   3,CV_CURSOR(17)
        jumpn   1,cv_consumed
        ; Exact output reached: no buffered or unread trailing words.
        ; Check each condition independently.  CAMN skips the NEXT
        ; instruction on equality, so pairing it with SKIPN could
        ; accidentally accept a group when CV_LEFT was still nonzero.
        move    0,CV_GROUP(17)
        camn    0,CV_USED(17)
        jrst    cv_check_left
        jrst    cv_bad
cv_check_left:
        skipn   CV_LEFT(17)
        jrst    cv_good
        jrst    cv_bad
cv_consumed:
        ; Move unconsumed words down; bounded maximum 63 words.
        move    6,CV_USED(17)
        sub     6,CV_GROUP(17)
        movem   6,CV_USED(17)
        jumpe   6,cv_next
        movei   1,CV_INPUT(17)
        move    2,1
        add     2,CV_GROUP(17)
cv_shift:
        move    0,(2)
        movem   0,(1)
        aoj     1,
        aoj     2,
        sojg    6,cv_shift
        jrst    cv_next
cv_refill:
        ; If no input remains, an incomplete buffered group is invalid.
        skipg   CV_LEFT(17)
        jrst    cv_bad
        move    6,CV_USED(17)
        movei   4,CV_WINDOW
        sub     4,6
        jumpe   4,cv_bad                  ; impossible: group <= 46
        camle   4,CV_LEFT(17)
        move    4,CV_LEFT(17)
        move    1,CV_VNODE(17)
        move    2,CV_OFFSET(17)
        movei   3,CV_INPUT(17)
        add     3,6
        ; Save the requested capacity: a faulty provider must never be
        ; allowed to publish more words than fit in the stack window.
        push    17,4
        pushj   17,vfs_read_words
        pop     17,4
        jumple  1,cv_bad
        camle   1,4
        jrst    cv_bad
        addm    1,CV_USED(17)
        addm    1,CV_OFFSET(17)
        movn    0,1
        addm    0,CV_LEFT(17)
        jrst    cv_next
cv_good:
        setz    1,
        jrst    cv_return
cv_bad:
        seto    1,
cv_return:
        movei   0,1-CV_FRAME(17)
        hrl     0,0
        hrri    0,010
        blt     0,016
        sub     17,[CV_FRAME,,CV_FRAME]
        popj    17,
