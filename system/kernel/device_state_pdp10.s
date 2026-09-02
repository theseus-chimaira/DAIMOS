; device_state_pdp10.s -- unconditional device runtime state.
;
; Physical MRES drivers update these counters directly, so the state belongs to
; KCORE rather than to the optional RAMFS address range.
        .bss
        .globl  devicefs_present
devicefs_present:
        .block  1
        .globl  devicefs_io_in
devicefs_io_in:
        .block  012                     ; DEVICEFS_IO_IN_COUNT
        .globl  devicefs_io_out
devicefs_io_out:
        .block  012                     ; DEVICEFS_IO_OUT_COUNT
