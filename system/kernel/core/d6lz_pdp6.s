/**
 * @file d6lz_pdp6.s
 * @brief PDP-6 runtime frontends for the resident D6LZ36 decoder.
 *
 * D6LZ36 token decoding lives in the fixed low-core decoder from
 * system/stand/pdp6/common/decompressor.inc.  Stage1 installs that decoder at
 * 000060 and KCORE links the same image at the same address.  This file is the
 * resident runtime adapter.  Filesystem EXEC loads use the refill entry, which
 * reads compressed words through VFS in bounded windows.  Callers which already
 * hold one complete compressed stream, such as TSFS restart extents, use the
 * direct-buffer entry and therefore share the same state setup, exact-consume
 * checks, and AC save/restore without paying VFS/provider overhead.
 *
 * The implementation is specifically PDP-6 code.  It assumes 18-bit address
 * halves, an AC17 pushdown stack, PDP-6 BLT semantics, and the fixed decoder
 * entry at 000060.  A future later-PDP-10 implementation may therefore use a
 * different source file and machine-specific instructions without weakening
 * the PDP-6 baseline.
 *
 * No permanent decode buffer exists.  Each call reserves seventy-two stack
 * words:
 * six words save AC10..AC15, two words retain the vnode and file offset, and
 * sixty-four words form the VFS refill window.  The output buffer itself is
 * the LZ history and must remain valid until decoding finishes.  A 64-word
 * window substantially reduces VFS/provider re-entry while leaving useful
 * headroom in the process-private kernel stack.  A full 128-word block window
 * was measured but rejected because its nested cold-read stack geometry is
 * too close to the process kernel-stack limit.
 */

        .text
        .globl d6lz36_decode_vfs
        .globl d6lz36_decode_buffer
        .globl d6lz36_decode_core
        .globl vfs_read_words

/** Number of compressed words fetched per VFS refill. */
        .equ D6LZ_VFS_WINDOW,0100
/** Complete frame: six saved ACs, two persistent words, and the input window. */
        .equ D6LZ_VFS_FRAME,D6LZ_VFS_WINDOW+010
        .equ D6LZ_VFS_SAVE_FIRST,1-D6LZ_VFS_FRAME
        .equ D6LZ_VFS_SAVE_LAST,6-D6LZ_VFS_FRAME
        .equ D6LZ_VFS_VNODE,7-D6LZ_VFS_FRAME
        .equ D6LZ_VFS_OFFSET,010-D6LZ_VFS_FRAME
        .equ D6LZ_VFS_INPUT,011-D6LZ_VFS_FRAME

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
 * this routine advances it by D6LZ_VFS_FRAME words for the complete frame and
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
d6lz36_decode_vfs:
        ; Reserve the whole frame once and save AC10..AC15 with one BLT.
        add     17,[D6LZ_VFS_FRAME,,D6LZ_VFS_FRAME]
        movei   0,D6LZ_VFS_SAVE_FIRST(17)
        hrli    0,010
        blt     0,D6LZ_VFS_SAVE_LAST(17)
        movem   1,D6LZ_VFS_VNODE(17)    ; vnode
        movem   2,D6LZ_VFS_OFFSET(17)   ; current file offset
        hlrz    13,4                    ; output words remaining
        hrrz    12,4                    ; current output address
        move    14,12                   ; output base
        move    15,3                    ; compressed words not yet read
        jumpn   15,d6lz_vfs_bound_ready
        seto    15,                     ; zero => trusted immutable backing
d6lz_vfs_bound_ready:
        jumpe   13,d6lz_vfs_error
        jumpe   12,d6lz_vfs_error
        setz    11,                     ; zero => load a control word
        jumpe   1,d6lz_buffer_start     ; zero vnode marks direct-buffer entry

d6lz_vfs_refill:
        jumpe   15,d6lz_vfs_error       ; core requested data past EOF
        move    1,D6LZ_VFS_VNODE(17)    ; vnode
        move    2,D6LZ_VFS_OFFSET(17)   ; file offset
        movei   3,D6LZ_VFS_INPUT(17)    ; refill window
        movei   4,D6LZ_VFS_WINDOW
        skipge  15                      ; trusted mode has no source bound
        jrst    d6lz_vfs_read
        caige   15,D6LZ_VFS_WINDOW
        move    4,15                    ; final short window
d6lz_vfs_read:
        pushj   17,vfs_read_words
        jumple  1,d6lz_vfs_error        ; EOF or provider error
        skipge  15
        jrst    d6lz_vfs_refilled
        sub     15,1                    ; words still unread from file
d6lz_vfs_refilled:
        addm    1,D6LZ_VFS_OFFSET(17)   ; advance file offset

        move    4,1                    ; source-window words returned
        movei   3,D6LZ_VFS_INPUT(17)
d6lz_decode_window:
        pushj   17,d6lz36_decode_core
        jumpe   0,d6lz_vfs_success
        jumpl   0,d6lz_vfs_error
        jrst    d6lz_vfs_refill         ; +1 = NEED_INPUT

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
d6lz36_decode_buffer:
        setz    1,                      ; select direct source in common entry
        jrst    d6lz36_decode_vfs

d6lz_buffer_start:
        move    3,2                     ; complete in-memory source
        move    4,15                    ; exact compressed word count
        setz    15,                     ; no unread VFS tail remains
        jrst    d6lz_decode_window

d6lz_vfs_success:
        skipge  15
        jrst    d6lz_vfs_success_trusted
        jumpn   4,d6lz_vfs_error        ; exact compressed payload required
        jumpn   15,d6lz_vfs_error
d6lz_vfs_success_trusted:
        setz    1,
        jrst    d6lz_vfs_return

d6lz_vfs_error:
        seto    1,
d6lz_vfs_return:
        movei   0,D6LZ_VFS_SAVE_FIRST(17)
        hrl     0,0
        hrri    0,010
        blt     0,015                   ; restore AC10..AC15
        sub     17,[D6LZ_VFS_FRAME,,D6LZ_VFS_FRAME]
        popj    17,
