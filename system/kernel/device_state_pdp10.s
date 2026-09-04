; device_state_pdp10.s -- unconditional DEVICEFS runtime state.
;
; Accounting arrays are indexed by DEVICEFS device id.  This keeps every
; detected device's counters directly reachable by DEVICEFS while allowing
; each driver to update its own slot without runtime registration machinery.
        .bss
        .globl  devicefs_present
devicefs_present:
        .block  1
        .globl  devicefs_io_in
devicefs_io_in:
        .block  021                    ; DEVICEFS_DEV_COUNT (17 decimal)
        .globl  devicefs_io_out
devicefs_io_out:
        .block  021
