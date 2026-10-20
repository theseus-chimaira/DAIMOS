/**
 * @file drm236.s
 * @brief Two-word KCORE dispatch for optional Type-167/236 drum hardware.
 *
 * MINIT patches each jump to the validated entrypoint in DRM MRES.
 * Without installed drum hardware these calls fail safely.  The address
 * conversion is implemented in drm236_mres_v1.s.
 */
        .text
        .globl drm236_read_block
        .globl drm236_write_block
        .globl drm236_read_jump
        .globl drm236_write_jump
        .globl kret_neg1

drm236_read_block:
drm236_read_jump:
        jrst kret_neg1

drm236_write_block:
drm236_write_jump:
        jrst kret_neg1
