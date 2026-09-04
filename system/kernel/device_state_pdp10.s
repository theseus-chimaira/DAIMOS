; device_state_pdp10.s -- unconditional DEVICEFS runtime state.
;
; Accounting arrays are indexed by DEVICEFS device id.  Endpoint slots which
; cannot exist are reused for independent scalar KCORE state.  Drivers still
; use direct AOS addresses and DEVICEFS still indexes counters directly; the
; alias slots are unreachable through the endpoint capability masks.
        .bss
        .globl  devicefs_names
devicefs_names:
        .block  021                    ; frozen detected-device SIXBIT names

        .globl  devicefs_io_in
        .globl  proc_wait_channel
        .globl  mach_user_sp
        .globl  file_cwd
        .globl  vfs_namespace_root
        .globl  vfs_mount_ro
devicefs_io_in:
        .block  1                      ; 00 CTY0 IN
proc_wait_channel:
        .block  1                      ; 01 CLK0 has no IN
        .block  1                      ; 02 PTR0 IN
mach_user_sp:
        .block  1                      ; 03 PTP0 has no IN
        .block  1                      ; 04 CR0 IN
file_cwd:
        .block  1                      ; 05 CP0 has no IN
        .block  2                      ; 06 DCS0, 07 GE0 IN
vfs_namespace_root:
        .block  1                      ; 010 DPY0 has no IN
vfs_mount_ro:
        .block  1                      ; 011 TTY0 has no IN
        .block  5                      ; 012..016 remaining IN counters

        .globl  devicefs_io_out
        .globl  mach_kernel_sp
        .globl  storage_state
        .globl  storage_iowd
        .globl  storage_count
devicefs_io_out:
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
