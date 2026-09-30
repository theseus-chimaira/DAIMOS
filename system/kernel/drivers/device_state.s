/**
 * @file device_state.s
 * @brief Shared resident device-accounting and low-cost kernel state layout.
 *
 * This file owns unconditional KCORE state shared by MonitorFS, device drivers,
 * storage routing, VFS, and machine-context code. The two dense MonitorFS I/O
 * counter arrays are indexed directly by MonitorFS device id so drivers can
 * account completed operations with a single AOS.
 *
 * Several device ids can never support one direction of I/O. Those unreachable
 * counter slots deliberately alias independent scalar KCORE state, saving one
 * 36-bit word per alias. This is safe only because MonitorFS checks the matching
 * read/write capability mask before indexing an accounting array. D6SET uses
 * explicit aggregate counters and is special-cased before dense-array access.
 *
 * All storage here is permanent zero-initialized BSS. There is no PDP-10-only
 * instruction or machine dependency, so this is generic PDP-6 implementation
 * code rather than a model-specific assembler file.
 */
        .bss
        .globl  mfsdev_names
/** Frozen detected-device SIXBIT names, indexed by MonitorFS device id 0..022. */
mfsdev_names:
        .block  023                    ; frozen detected-device SIXBIT names

        .globl  mfsdev_io_in
        .globl  mach_user_sp
        .globl  file_table
        .globl  vfs_namespace_root
        .globl  vfs_mount_ro
/**
 * Completed input/read counters indexed by MonitorFS device id.
 *
 * Dead input slots are reused as follows:
 *   01 CLK0  -> mfsdev_d6set_members
 *   03 PTP0  -> mach_user_sp
 *   05 CP0   -> file_table
 *   010 DPY0 -> vfs_namespace_root
 *   011 TTY0 -> vfs_mount_ro
 *   017 SLV0 -> drm236_active_request
 * D6SET0 (020) and DRM0 (021) are real counters. LPT0 (022) has no input
 * counter and aliases the first word of mfsdev_io_out instead of consuming a
 * separate word.
 */
mfsdev_io_in:
        .block  1                      ; 00 CTY0 IN
        .globl  mfsdev_d6set_members
mfsdev_d6set_members:
        .block  1                      ; 01 CLK0 has no IN; packed D6SET members
        .block  1                      ; 02 PTR0 IN
mach_user_sp:
        .block  1                      ; 03 PTP0 has no IN
        .block  1                      ; 04 CR0 IN
file_table:
        .block  1                      ; 05 CP0 has no IN; current FILE table pointer
        .block  2                      ; 06 DCS0, 07 GE0 IN
vfs_namespace_root:
        .block  1                      ; 010 DPY0 has no IN
vfs_mount_ro:
        .block  1                      ; packed VFS RO bits 0..3, storage-pin bits 4..7
        .block  5                      ; 012..016 remaining IN counters
        .globl  drm236_active_request
        .globl  mfsdev_d6set_reads
        .globl  mfsdev_drm_reads
drm236_active_request:
        .block  1                      ; 017 SLV0 has no IN
mfsdev_d6set_reads:
        .block  1                      ; 020 D6SET0 request reads
mfsdev_drm_reads:
        .block  1                      ; 021 DRM0 reads
        ; 022 LPT0 has no IN; it aliases mfsdev_io_out[CTY0] below.

        .globl  mfsdev_io_out
        .globl  mach_kernel_sp
        .globl  storage_state
        .globl  storage_iowd
        .globl  storage_count
/**
 * Completed output/write counters indexed by MonitorFS device id.
 *
 * Dead output slots are reused as follows:
 *   01 CLK0   -> mach_kernel_sp
 *   02 PTR0   -> storage_state
 *   04 CR0    -> storage_iowd
 *   013 OCNSLS-> storage_count
 *   017 SLV0  -> drm236_pending_request
 * D6SET0 (020), DRM0 (021), and LPT0 (022) are real output counters.
 */
mfsdev_io_out:
        .block  1                      ; 00 CTY0 OUT
mach_kernel_sp:
        .block  1                      ; 01 CLK0 has no OUT
storage_state:
        .block  1                      ; 02 PTR0 has no OUT
        .block  1                      ; 03 PTP0 OUT
storage_iowd:
        .block  1                      ; 04 CR0 has no OUT
        .block  6                      ; 05..012 OUT counters
storage_count:
        .block  1                      ; 013 OCNSLS has no OUT
        .block  3                      ; 014..016 OUT counters
        .globl  drm236_pending_request
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_drm_writes
drm236_pending_request:
        .block  1                      ; 017 SLV0 has no OUT; one-request DRM FIFO
mfsdev_d6set_writes:
        .block  1                      ; 020 D6SET0 request writes
mfsdev_drm_writes:
        .block  1                      ; 021 DRM0 writes
        .block  1                      ; 022 LPT0 OUT


/**
 * Sparse storage-error counters for MonitorFS ids 014..021.
 *
 * The six words are DTC0, MTC0, DSK0, SLV0, D6SET0, and DRM0 in that order.
 * Read/write totals remain in the dense arrays above; only errors need this
 * additional storage because they cannot be derived from completed requests.
 */
        .globl  mfsdev_storage_errors
        .globl  mfsdev_drm_reads
        .globl  mfsdev_drm_writes
        .globl  mfsdev_d6set_reads
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_d6set_members

mfsdev_storage_errors:       .block 6


        .text
        .globl  storage_request_init
/**
 * @brief Initialize a three-word asynchronous block-I/O request descriptor.
 * @param AC1 Device/block address; its right 18-bit half becomes descriptor LH.
 * @param AC2 Core-buffer word address; its right half becomes descriptor RH.
 * @param AC4 Driver operation code stored in descriptor word 1.
 * @return Descriptor words at -3(AC17)..-1(AC17); completion starts at zero.
 *
 * The caller must reserve three words by advancing AC17 before entry. AC2 is
 * clobbered while HRL forms address,,buffer in one instruction; AC1 and AC4
 * retain their input values. The completion word is the event polled or slept
 * on by DSK/DRM request paths. The helper is permanent because both resident
 * storage drivers share this descriptor ABI.
 */
storage_request_init:
        hrl     2,1
        movem   2,-3(017)
        movem   4,-2(017)
        setzm   -1(017)
        popj    017,
