; Permanent KCORE entry points for the pinned process swap service.
; Before installation, memory-pressure reclamation safely fails.
        .text
        .globl proc_swap_reclaim
        .globl proc_swap_service_one
        .globl kret_neg1
proc_swap_reclaim:
        jrst kret_neg1
proc_swap_service_one:
        jrst kret_neg1
        .globl proc_swap_storage_invariant_v30
proc_swap_storage_invariant_v30:
        halt .
