; device_state_pdp10.s -- unconditional DEVICEFS runtime state.
;
; Accounting arrays are indexed by DEVICEFS device id.  This keeps every
; detected device's counters directly reachable by DEVICEFS while allowing
; each driver to update its own slot without runtime registration machinery.
        .bss
        .globl  devicefs_names
devicefs_names:
        .block  021                    ; frozen detected-device SIXBIT names
        .globl  devicefs_io_in
devicefs_io_in:
        .block  017                    ; counters exist only for ids 0..14
        .globl  devicefs_io_out
devicefs_io_out:
        .block  017
