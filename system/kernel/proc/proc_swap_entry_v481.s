; Permanent KCORE entry points for the pinned process swap service.
; Before installation, memory-pressure reclamation safely fails.
        .text
        .globl proc_swap_reclaim
        .globl proc_swap_service_one
        .globl proc_swap_term_v1666
        .globl kret_neg1
proc_swap_reclaim:
        jrst kret_neg1
proc_swap_service_one:
        jrst kret_neg1
; Default refusal: no installed memory service must never free
; an unknown DISK or PACKED owner. The RESIDENT common path does
; not call this trampoline.
proc_swap_term_v1666:
        jrst kret_neg1
        .globl proc_swap_storage_invariant_v30
proc_swap_storage_invariant_v30:
        halt .
