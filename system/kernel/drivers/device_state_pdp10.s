; device_state_pdp10.s -- unconditional MonitorFS device view runtime state.
;
; Accounting arrays are indexed by MonitorFS device view device id.  Endpoint slots which
; cannot exist are reused for independent scalar KCORE state.  Drivers still
; use direct AOS addresses and MonitorFS device view still indexes counters directly; the
; alias slots are unreachable through the endpoint capability masks.
        .bss
        .globl  mfsdev_names
mfsdev_names:
        .block  023                    ; frozen detected-device SIXBIT names

        .globl  mfsdev_io_in
        .globl  mach_user_sp
        .globl  file_table
        .globl  vfs_namespace_root
        .globl  vfs_mount_ro
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
        .block  1                      ; 011 TTY0 has no IN
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
        .block  1                      ; 022 LPT0 has no IN

        .globl  mfsdev_io_out
        .globl  mach_kernel_sp
        .globl  storage_state
        .globl  storage_iowd
        .globl  storage_count
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
        .globl  drm236_idle_event
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_drm_writes
drm236_idle_event:
        .block  1                      ; 017 SLV0 has no OUT
mfsdev_d6set_writes:
        .block  1                      ; 020 D6SET0 request writes
mfsdev_drm_writes:
        .block  1                      ; 021 DRM0 writes
        .block  1                      ; 022 LPT0 OUT


; Sparse extended MonitorFS device view accounting.  The legacy io_in/io_out arrays above
; remain the per-device completed READS/WRITES counters.  Only statistics
; which cannot be derived from those counters consume additional KCORE words.
        .globl  mfsdev_storage_errors
        .globl  mfsdev_mtc_words_read
        .globl  mfsdev_mtc_words_written
        .globl  mfsdev_drm_reads
        .globl  mfsdev_drm_writes
        .globl  mfsdev_d6set_reads
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_d6set_blocks_read
        .globl  mfsdev_d6set_blocks_written
        .globl  mfsdev_log_reads
        .globl  mfsdev_log_writes
        .globl  mfsdev_log_blocks_read
        .globl  mfsdev_log_blocks_written
        .globl  mfsdev_log_errors
        .globl  mfsdev_d6set_members

; Indexed by MonitorFS device view id - DTC0 (014): DTC0, MTC0, DSK0, SLV0, D6SET0,
; DRM0.  Appending DRM0 preserves all existing offsets.
mfsdev_storage_errors:       .block 6

mfsdev_mtc_words_read:       .block 1
mfsdev_mtc_words_written:    .block 1
mfsdev_d6set_blocks_read:    .block 1
mfsdev_d6set_blocks_written: .block 1

mfsdev_log_reads:            .block 1
mfsdev_log_writes:           .block 1
mfsdev_log_blocks_read:      .block 1
mfsdev_log_blocks_written:   .block 1
mfsdev_log_errors:           .block 1


        .text
        .globl  storage_request_init
; Reserve three words in the caller before entering.  Fill the standard
; address,,buffer / operation / completion descriptor used by block drivers.
storage_request_init:
        hrlz    5,1
        hrr     5,2
        movem   5,-3(017)
        movem   4,-2(017)
        setzm   -1(017)
        popj    017,
