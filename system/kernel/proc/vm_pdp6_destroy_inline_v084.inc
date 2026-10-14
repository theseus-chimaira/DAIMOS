/* Staged replacement for the vm_space_destroy leaf only. */
        .text
        .globl proc_packed_destroy_v084
vm_space_destroy_inline_v084:
        push    17,1                    ; descriptor
        push    17,2                    ; owner
        ; Staged storage-dispatch gate: PACKED retirement is MM-owned.
        ; Do not destroy a PACKED/invalid process through the old disk-swap
        ; cleanup path.  A packed image needs its registered MM extent
        ; released first; no such image may be published until that path is
        ; installed.  Reject before freeing any resident MM allocation.
        move    1,(17)
        pushj   17,proc_packed_destroy_v084
        jumpl   1,vm_space_destroy_fail
        jumpn   1,vm_space_destroy_packed_done
        move    1,(17)                 ; slot
        movei   2,0                    ; RESIDENT
        pushj   17,proc_storage_expect_v30
        jumpe   1,vm_space_destroy_mode_ok
        move    1,(17)
        movei   2,1                    ; DISK
        pushj   17,proc_storage_expect_v30
        jumpn   1,vm_space_destroy_fail
        move    2,-1(17)              ; DISK cannot have a resident VM base
        hrrz    3,1(2)
        jumpn   3,vm_space_destroy_fail
vm_space_destroy_mode_ok:
        move    1,-1(17)              ; restore descriptor for MM teardown
        hrrz    3,1(1)
        jumpe   3,vm_space_destroy_detach
        move    1,3
        movei   2,1                    ; MM_TYPE_PROCESS
        move    3,(17)
        pushj   17,mm_free
        jumpn   1,vm_space_destroy_fail
vm_space_destroy_detach:
        move    1,(17)
        pushj   17,proc_swap_detach
        move    2,-1(17)
        setom   1(2)                   ; VM_SPACE_NONE
        setz    1,
        jrst    vm_space_destroy_done
vm_space_destroy_packed_done:
        move    2,-1(17)
        setom   1(2)                   ; retired packed VM
        setz    1,
        jrst    vm_space_destroy_done
vm_space_destroy_fail:
        seto    1,
vm_space_destroy_done:
        sub     17,kconst_2_2
        popj    17,
