; device_state_pdp10.s -- unconditional device runtime state.
;
; Only CTY accounting is fixed because DEVICEFS exposes CTY input/output
; directly from KCORE.  Optional devices keep their accounting words in their
; owning MRES so absent hardware consumes no resident KCORE state.
        .bss
        .globl  devicefs_present
devicefs_present:
        .block  1
        .globl  devicefs_io_in
devicefs_io_in:
        .block  1
        .globl  devicefs_io_out
devicefs_io_out:
        .block  1
