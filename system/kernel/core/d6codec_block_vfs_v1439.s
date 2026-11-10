/* Experimental optimized block LZ/RLE register-resident VFS adapter v1439.
 * NOT a production codec, but uses the exact v1421 wire format.
 * Based on the existing compact legacy VFS refill calling convention:
 * AC11 is literal words pending, AC12 destination, AC13 remaining
 * output words, AC14 destination origin, AC15 unread VFS words.
 * The fixed-size refill window is 32 words. No static data or BSS.
 * AC10 is never modified by this adapter/core; the VFS provider must
 * preserve AC10 by the kernel ABI. Only AC11..AC15 need a save frame.
 * Exact source lengths and physical address geometry are validated
 * before writes. The complete frame is protected against output
 * overlap; direct source buffers must not alias output or frame.
 */
        .text
        .globl d6codec_decode_vfs_v1439
        .globl d6codec_decode_buffer_v1439
        .globl d6codec_block_core_v1439
        .globl vfs_read_words

/** Number of compressed words fetched per VFS refill. */
        .equ D6B_V1439_VFS_WINDOW,040
/** Frame: five saved ACs (11..15), two locals, and 32 refill words. */
        .equ D6B_V1439_VFS_FRAME,D6B_V1439_VFS_WINDOW+07
        .equ D6B_V1439_VFS_SAVE_FIRST,1-D6B_V1439_VFS_FRAME
        .equ D6B_V1439_VFS_SAVE_LAST,5-D6B_V1439_VFS_FRAME
        .equ D6B_V1439_VFS_VNODE,6-D6B_V1439_VFS_FRAME
        .equ D6B_V1439_VFS_OFFSET,07-D6B_V1439_VFS_FRAME
        .equ D6B_V1439_VFS_INPUT,010-D6B_V1439_VFS_FRAME

/**
 * @brief Decode one exactly framed block v1421 payload into memory.
 *
 * C ABI input:
 *   AC1 = vnode
 *   AC2 = compressed payload word offset in the vnode
 *   AC3 = exact compressed payload length in words, or zero only when the
 *         caller already owns validated immutable backing and decoding may
 *         stop as soon as the requested output image is complete
 *   AC4 = output_word_count,,destination_address
 *
 * C ABI output:
 *   AC1 = 0 on success, -1 on malformed input, invalid geometry, premature
 *         EOF, VFS failure, or non-exact compressed-payload consumption in
 *         bounded mode.
 *
 * AC10..AC15 are callee-saved: AC10 stays untouched, AC11..AC15
 * are saved and restored in the smaller frame.  AC0..AC7 may be
 * clobbered according to the normal kernel ABI.  AC17 is the pushdown pointer;
 * this routine advances it by D6B_V1439_VFS_FRAME words for the complete frame and
 * restores it exactly before POPJ.
 *
 * The register-resident core owns resumable state in AC11..AC14. AC15 holds the
 * number of compressed words not yet fetched from VFS.  vfs_read_words() may
 * clobber caller-saved ACs, so vnode and file offset live in stack locals and
 * the source pointer is reconstructed after every refill.  The decoder's +1
 * NEED_INPUT result resumes the same state with the next window.  Normal
 * bounded calls accept zero only when the declared payload has been consumed
 * exactly; trusted immutable-backing calls use AC3=0 and accept completion as
 * soon as the requested output image is complete.
 */
d6codec_decode_vfs_v1439:
        ; Reserve the frame; AC10 is untouched, save AC11..AC15.
        add     17,[D6B_V1439_VFS_FRAME,,D6B_V1439_VFS_FRAME]
        movei   0,D6B_V1439_VFS_SAVE_FIRST(17)
        hrli    0,011
        blt     0,D6B_V1439_VFS_SAVE_LAST(17)
        movem   1,D6B_V1439_VFS_VNODE(17)    ; vnode
        movem   2,D6B_V1439_VFS_OFFSET(17)   ; current file offset
        hlrz    13,4                    ; output words remaining
        hrrz    12,4                    ; current output address
        move    14,12                   ; output base
        move    15,3                    ; compressed words not yet read
        ; Full LH check also rejects negative counts (sign bit set).
        tlne    15,0777777              ; reject >18-bit and signed counts
        jrst    d6b_vfs_error_v1439
        jumpn   15,d6b_vfs_bound_ready_v1439
        skipn   1                       ; zero direct count is not trusted
        jrst    d6b_vfs_error_v1439
        seto    15,                     ; zero => trusted immutable backing
d6b_vfs_bound_ready_v1439:
        jumpe   13,d6b_vfs_error_v1439
        jumpe   12,d6b_vfs_error_v1439
        ; Output end may equal physical 01000000 but may not exceed it.
        move    5,12
        add     5,13
        camle   5,[01000000]
        jrst    d6b_vfs_error_v1439
        ; Disallow the entire caller-owned VFS frame from output.
        hrrz    6,17
        subi    6,D6B_V1439_VFS_FRAME-1
        hrrz    7,17
        aoj     7,
        caml    12,7
        jrst    d6b_vfs_dst_ok_v1439
        camle   5,6
        jrst    d6b_vfs_error_v1439
d6b_vfs_dst_ok_v1439:
        setz    11,                     ; zero => load a control word
        jumpe   1,d6b_buffer_start_v1439     ; zero vnode marks direct-buffer entry

d6b_vfs_refill_v1439:
        jumpe   15,d6b_vfs_error_v1439       ; core requested data past EOF
        move    1,D6B_V1439_VFS_VNODE(17)    ; vnode
        move    2,D6B_V1439_VFS_OFFSET(17)   ; file offset
        movei   3,D6B_V1439_VFS_INPUT(17)    ; refill window
        movei   4,D6B_V1439_VFS_WINDOW
        skipge  15                      ; trusted mode has no source bound
        jrst    d6b_vfs_read_v1439
        caige   15,D6B_V1439_VFS_WINDOW
        move    4,15                    ; final short window
d6b_vfs_read_v1439:
        pushj   17,vfs_read_words
        jumple  1,d6b_vfs_error_v1439        ; EOF or provider error
        skipge  15
        jrst    d6b_vfs_refilled_v1439
        sub     15,1                    ; words still unread from file
d6b_vfs_refilled_v1439:
        addm    1,D6B_V1439_VFS_OFFSET(17)   ; advance file offset

        move    4,1                    ; source-window words returned
        movei   3,D6B_V1439_VFS_INPUT(17)
d6b_decode_window_v1439:
        pushj   17,d6codec_block_core_v1439
        jumpe   0,d6b_vfs_success_v1439
        jumpl   0,d6b_vfs_error_v1439
        jrst    d6b_vfs_refill_v1439         ; +1 = NEED_INPUT

/**
 * @brief Decode one complete compressed buffer through the common runtime frame.
 * @param AC2 Address of the first compressed block-format word.
 * @param AC3 Exact compressed word count.
 * @param AC4 Output-word-count,,destination-address.
 * @return AC1 Zero on exact successful decode, or -1 on malformed/truncated
 *         input, invalid geometry, or trailing compressed words.
 *
 * This assembly-only entry shares the VFS frontend's AC10..AC15 save frame and
 * result checks.  It deliberately does not copy the source into the refill
 * window: callers such as TSFS already own a stable in-memory extent buffer.
 */
d6codec_decode_buffer_v1439:
        setz    1,                      ; select direct source in common entry
        jrst    d6codec_decode_vfs_v1439

d6b_buffer_start_v1439:
        ; Check full physical source geometry and source/stack/output
        ; disjointness before the first compressed word is consumed.
        tlne    2,0777777
        jrst    d6b_vfs_error_v1439
        move    0,2
        add     0,15
        camle   0,[01000000]
        jrst    d6b_vfs_error_v1439
        caml    2,5                     ; source starts past output end
        jrst    d6b_vfs_src_output_ok_v1439
        camle   0,12                    ; source ends before output start
        jrst    d6b_vfs_error_v1439
d6b_vfs_src_output_ok_v1439:
        caml    2,7                     ; source starts past frame end
        jrst    d6b_vfs_src_frame_ok_v1439
        camle   0,6                     ; source ends before frame start
        jrst    d6b_vfs_error_v1439
d6b_vfs_src_frame_ok_v1439:
        move    3,2                     ; complete in-memory source
        move    4,15                    ; exact compressed word count
        setz    15,                     ; no unread VFS tail remains
        jrst    d6b_decode_window_v1439

d6b_vfs_success_v1439:
        skipge  15
        jrst    d6b_vfs_success_trusted_v1439
        jumpn   4,d6b_vfs_error_v1439        ; exact compressed payload required
        jumpn   15,d6b_vfs_error_v1439
d6b_vfs_success_trusted_v1439:
        setz    1,
        jrst    d6b_vfs_return_v1439

d6b_vfs_error_v1439:
        seto    1,
d6b_vfs_return_v1439:
        movei   0,D6B_V1439_VFS_SAVE_FIRST(17)
        hrl     0,0
        hrri    0,011
        blt     0,015                   ; restore AC11..AC15
        sub     17,[D6B_V1439_VFS_FRAME,,D6B_V1439_VFS_FRAME]
        popj    17,
