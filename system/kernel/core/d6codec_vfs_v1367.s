/* Staged grouped D6CODEC VFS runtime adapter, not linked to KCORE.
 *
 * Exact C ABI (same input/output contract as d6lz36_decode_vfs):
 *   AC1 = vnode (zero selects disjoint complete-buffer input)
 *   AC2 = vnode compressed word offset (buffer address for direct mode)
 *   AC3 = exact compressed word count; zero selects immutable trusted
 *         VFS backing only (never accepted in direct-buffer mode)
 *   AC4 = output_words,,destination_address
 *   AC1 = zero success, -1 malformed/EOF/overlap/provider failure
 * AC10..AC16 are preserved; AC0..AC7 may be clobbered. AC17 is the
 * process-private kernel stack. Only the frame is persistent during
 * decode; there is no global, heap or permanently resident state.
 *
 * Compared to the legacy 72-word D6LZ36 frontend frame, the grouped
 * frame uses 67 words: seven saved ACs, vnode/offset/unread, 25-word
 * grouped continuation state and 32-word VFS window. This is frame
 * size only, NOT a worst-case nested VFS/provider stack measurement.
 */
        .text
        .globl d6codec_decode_vfs_v1367
        .globl d6codec_decode_buffer_v1367
        .globl d6codec_stream_v1367
        .globl vfs_read_words

        .equ D6C_WINDOW_V1367,040
        .equ D6C_STATE_V1367,031
        .equ D6C_FRAME_V1367,0103
        .equ D6C_SAVE_FIRST_V1367,1-D6C_FRAME_V1367
        .equ D6C_SAVE_LAST_V1367,7-D6C_FRAME_V1367
        .equ D6C_VNODE_V1367,010-D6C_FRAME_V1367
        .equ D6C_OFFSET_V1367,011-D6C_FRAME_V1367
        .equ D6C_UNREAD_V1367,012-D6C_FRAME_V1367
        .equ D6C_CONT_V1367,013-D6C_FRAME_V1367
        .equ D6C_INPUT_V1367,044-D6C_FRAME_V1367

        .equ D6C_S_PHASE_V1367,0
        .equ D6C_S_DEST_V1367,1
        .equ D6C_S_END_V1367,2
        .equ D6C_S_BASE_V1367,3
        .equ D6C_S_REMAIN_V1367,4

d6codec_decode_buffer_v1367:
        setz    1,                      ; vnode zero means direct buffer
        jrst    d6codec_decode_vfs_v1367

d6codec_decode_vfs_v1367:
        add     17,[D6C_FRAME_V1367,,D6C_FRAME_V1367]
        movei   0,D6C_SAVE_FIRST_V1367(17)
        hrli    0,010
        blt     0,D6C_SAVE_LAST_V1367(17)
        movem   1,D6C_VNODE_V1367(17)
        movem   2,D6C_OFFSET_V1367(17)
        movem   3,D6C_UNREAD_V1367(17)
        ; Initialize only the five persistent state words used before
        ; the first control group. Later group fields are set by core.
        movei   1,D6C_CONT_V1367(17)
        setzm   D6C_S_PHASE_V1367(1)
        hlrz    0,4                     ; decoded output word count
        jumpe   0,d6c_vfs_error_v1367
        hrrz    5,4                     ; decoded physical base
        jumpe   5,d6c_vfs_error_v1367
        movem   5,D6C_S_DEST_V1367(1)
        movem   5,D6C_S_BASE_V1367(1)
        add     5,0
        movem   5,D6C_S_END_V1367(1)
        ; Protect the complete frame, not merely the decoder state:
        ; output may not overwrite saved ACs or the refill window.
        hrrz    6,17
        subi    6,D6C_FRAME_V1367-1    ; first reserved frame word
        hrrz    7,17
        aoj     7,                      ; exclusive frame upper bound
        move    5,D6C_S_BASE_V1367(1)
        caml    5,7                     ; nonoverlap: base >= frame end
        jrst    d6c_vfs_dst_ok_v1367
        move    5,D6C_S_END_V1367(1)
        camle   5,6
        jrst    d6c_vfs_error_v1367
d6c_vfs_dst_ok_v1367:
        move    0,D6C_UNREAD_V1367(17)
        jumpe   0,d6c_vfs_trusted_v1367
        jumpl   0,d6c_vfs_error_v1367
        tlne    0,0777777              ; exact count fits 18 bits
        jrst    d6c_vfs_error_v1367
        jrst    d6c_vfs_exact_v1367
d6c_vfs_trusted_v1367:
        skipn   D6C_VNODE_V1367(17)    ; no trusted direct buffers
        jrst    d6c_vfs_error_v1367
        seto    0,                      ; immutable VFS backing sentinel
        movem   0,D6C_UNREAD_V1367(17)
d6c_vfs_exact_v1367:
        movem   0,D6C_S_REMAIN_V1367(1)
        skipn   D6C_VNODE_V1367(17)
        jrst    d6c_vfs_buffer_v1367

d6c_vfs_refill_v1367:
        move    0,D6C_UNREAD_V1367(17)
        jumpe   0,d6c_vfs_error_v1367
        move    1,D6C_VNODE_V1367(17)
        move    2,D6C_OFFSET_V1367(17)
        movei   3,D6C_INPUT_V1367(17)
        movei   4,D6C_WINDOW_V1367
        jumpl   0,d6c_vfs_read_v1367  ; trusted reads whole window
        camge   0,4
        move    4,0                     ; bounded final short window
d6c_vfs_read_v1367:
        pushj   17,vfs_read_words
        jumple  1,d6c_vfs_error_v1367
        move    0,D6C_UNREAD_V1367(17)
        jumpl   0,d6c_vfs_refilled_v1367
        sub     0,1
        jumpl   0,d6c_vfs_error_v1367
        movem   0,D6C_UNREAD_V1367(17)
d6c_vfs_refilled_v1367:
        addm    1,D6C_OFFSET_V1367(17)
        move    3,1                     ; words in current input window
        movei   2,D6C_INPUT_V1367(17)
        movei   1,D6C_CONT_V1367(17)
d6c_vfs_decode_v1367:
        pushj   17,d6codec_stream_v1367
        jumpe   0,d6c_vfs_success_v1367
        jumpl   0,d6c_vfs_error_v1367
        jrst    d6c_vfs_refill_v1367   ; +1 NEED_INPUT

d6c_vfs_buffer_v1367:
        ; A direct input buffer may not alias any reserved stack frame
        ; word, even though it cannot alias the decoder's state alone.
        move    5,D6C_OFFSET_V1367(17)
        move    0,5
        add     0,D6C_UNREAD_V1367(17)
        caml    5,7                     ; nonoverlap: source >= frame end
        jrst    d6c_vfs_src_ok_v1367
        camle   0,6
        jrst    d6c_vfs_error_v1367
d6c_vfs_src_ok_v1367:
        move    3,D6C_UNREAD_V1367(17)
        move    2,D6C_OFFSET_V1367(17)
        setzm   D6C_UNREAD_V1367(17)
        jrst    d6c_vfs_decode_v1367

d6c_vfs_success_v1367:
        move    0,D6C_UNREAD_V1367(17)
        jumpe   0,d6c_vfs_valid_v1367
        camn    0,[-1]
        jrst    d6c_vfs_valid_v1367
        jrst    d6c_vfs_error_v1367
d6c_vfs_valid_v1367:
        setz    1,
        jrst    d6c_vfs_return_v1367
d6c_vfs_error_v1367:
        seto    1,
d6c_vfs_return_v1367:
        movei   0,D6C_SAVE_FIRST_V1367(17)
        hrl     0,0
        hrri    0,010
        blt     0,016                   ; restore AC10..AC16
        sub     17,[D6C_FRAME_V1367,,D6C_FRAME_V1367]
        popj    17,
