/* PDP-6 v088 relocatable codec buffer + optional VFS frontend.
 * Derived from the validated v070 bounded-stream framing and AC saving.
 * The decoder call goes to the independent v088 KCORE inline decoder, not
 * to Stage-1's D6LZ36 low-core decode routine. Preserve both formats.
 * No static scratch or permanent decompression buffer is allocated.
 */
        .text
        .globl d6inline_decode_vfs_reloc_v088
        .globl d6inline_decode_buffer_reloc_v088
        .globl d6codec_inline_decode_reloc_v088
        .globl vfs_read_words

/** Number of compressed words fetched per VFS refill. */
        .equ D6I70_VFS_WINDOW,0100
/** Complete frame: six saved ACs, two persistent words, and the input window. */
        .equ D6I70_VFS_FRAME,D6I70_VFS_WINDOW+010
        .equ D6I70_VFS_SAVE_FIRST,1-D6I70_VFS_FRAME
        .equ D6I70_VFS_SAVE_LAST,6-D6I70_VFS_FRAME
        .equ D6I70_VFS_VNODE,7-D6I70_VFS_FRAME
        .equ D6I70_VFS_OFFSET,010-D6I70_VFS_FRAME
        .equ D6I70_VFS_INPUT,011-D6I70_VFS_FRAME

/**
 * @brief Decode one exactly framed D6LZ36 VFS payload into memory.
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
 * AC10..AC15 are callee-saved and restored before return.  AC0..AC7 may be
 * clobbered according to the normal kernel ABI.  AC17 is the pushdown pointer;
 * this routine advances it by D6I70_VFS_FRAME words for the complete frame and
 * restores it exactly before POPJ.
 *
 * The low-core decoder owns resumable state in AC10..AC14.  AC15 holds the
 * number of compressed words not yet fetched from VFS.  vfs_read_words() may
 * clobber caller-saved ACs, so vnode and file offset live in stack locals and
 * the source pointer is reconstructed after every refill.  The decoder's +1
 * NEED_INPUT result resumes the same state with the next window.  Normal
 * bounded calls accept zero only when the declared payload has been consumed
 * exactly; trusted immutable-backing calls use AC3=0 and accept completion as
 * soon as the requested output image is complete.
 */
d6inline_decode_vfs_reloc_v088:
        ; Reserve the whole frame once and save AC10..AC15 with one BLT.
        add     17,[D6I70_VFS_FRAME,,D6I70_VFS_FRAME]
        movei   0,D6I70_VFS_SAVE_FIRST(17)
        hrli    0,010
        blt     0,D6I70_VFS_SAVE_LAST(17)
        movem   1,D6I70_VFS_VNODE(17)    ; vnode
        movem   2,D6I70_VFS_OFFSET(17)   ; current file offset
        hlrz    13,4                    ; output words remaining
        hrrz    12,4                    ; current output address
        move    14,12                   ; output base
        move    15,3                    ; compressed words not yet read
        jumpn   15,d6i70_vfs_bound_ready
        seto    15,                     ; zero => trusted immutable backing
d6i70_vfs_bound_ready:
        jumpe   13,d6i70_vfs_error
        jumpe   12,d6i70_vfs_error
        setz    11,                     ; zero => load a control word
        jumpe   1,d6i70_buffer_start     ; zero vnode marks direct-buffer entry

d6i70_vfs_refill:
        jumpe   15,d6i70_vfs_error       ; core requested data past EOF
        move    1,D6I70_VFS_VNODE(17)    ; vnode
        move    2,D6I70_VFS_OFFSET(17)   ; file offset
        movei   3,D6I70_VFS_INPUT(17)    ; refill window
        movei   4,D6I70_VFS_WINDOW
        skipge  15                      ; trusted mode has no source bound
        jrst    d6i70_vfs_read
        caige   15,D6I70_VFS_WINDOW
        move    4,15                    ; final short window
d6i70_vfs_read:
        pushj   17,vfs_read_words
        jumple  1,d6i70_vfs_error        ; EOF or provider error
        skipge  15
        jrst    d6i70_vfs_refilled
        sub     15,1                    ; words still unread from file
d6i70_vfs_refilled:
        addm    1,D6I70_VFS_OFFSET(17)   ; advance file offset

        move    4,1                    ; source-window words returned
        movei   3,D6I70_VFS_INPUT(17)
d6i70_decode_window:
        pushj   17,d6codec_inline_decode_reloc_v088
        jumpe   0,d6i70_vfs_success
        jumpl   0,d6i70_vfs_error
        jrst    d6i70_vfs_refill         ; +1 = NEED_INPUT

/**
 * @brief Decode one complete compressed buffer through the common runtime frame.
 * @param AC2 Address of the first compressed D6LZ36 word.
 * @param AC3 Exact compressed word count.
 * @param AC4 Output-word-count,,destination-address.
 * @return AC1 Zero on exact successful decode, or -1 on malformed/truncated
 *         input, invalid geometry, or trailing compressed words.
 *
 * This assembly-only entry shares the VFS frontend's AC10..AC15 save frame and
 * result checks.  It deliberately does not copy the source into the refill
 * window: callers such as TSFS already own a stable in-memory extent buffer.
 */
d6inline_decode_buffer_reloc_v088:
        setz    1,                      ; select direct source in common entry
        jrst    d6inline_decode_vfs_reloc_v088

d6i70_buffer_start:
        move    3,2                     ; complete in-memory source
        move    4,15                    ; exact compressed word count
        setz    15,                     ; no unread VFS tail remains
        jrst    d6i70_decode_window

d6i70_vfs_success:
        skipge  15
        jrst    d6i70_vfs_success_trusted
        jumpn   4,d6i70_vfs_error        ; exact compressed payload required
        jumpn   15,d6i70_vfs_error
d6i70_vfs_success_trusted:
        setz    1,
        jrst    d6i70_vfs_return

d6i70_vfs_error:
        seto    1,
d6i70_vfs_return:
        movei   0,D6I70_VFS_SAVE_FIRST(17)
        hrl     0,0
        hrri    0,010
        blt     0,015                   ; restore AC10..AC15
        sub     17,[D6I70_VFS_FRAME,,D6I70_VFS_FRAME]
        popj    17,
