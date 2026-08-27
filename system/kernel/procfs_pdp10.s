; procfs_pdp10.s -- compact PROCFS validation/search primitives.
        .text

        .globl procfs_v1_is_root
procfs_v1_is_root:
        hlrz    2,1
        move    3,2
        lsh     2,-014
        andi    2,077
        caie    2,3
        jrst    procfs_false
        andi    3,07777
        caie    3,1
        jrst    procfs_false
        movei   1,1
        popj    17,

        .globl procfs_v1_slot_live
procfs_v1_slot_live:
        cail    1,0100
        jrst    procfs_null
        move    3,1
        lsh     3,1
        move    4,proc_v1_table(3)
        move    5,4
        lsh     5,-017
        andi    5,07
        jumpe   5,procfs_null
        jumpe   2,procfs_slot_ptr
        andi    4,0377
        movem   4,(2)
procfs_slot_ptr:
        movei   1,proc_v1_table(3)
        popj    17,

        .globl procfs_v1_is_proc
procfs_v1_is_proc:
        hlrz    3,1
        move    4,3
        lsh     3,-014
        andi    3,077
        caie    3,3
        jrst    procfs_false
        andi    4,07777
        caie    4,2
        jrst    procfs_false
        hrrz    3,1
        cail    3,0100
        jrst    procfs_false
        move    4,3
        lsh     4,1
        move    5,proc_v1_table(4)
        lsh     5,-017
        andi    5,07
        jumpe   5,procfs_false
        jumpe   2,procfs_true
        movem   3,(2)
procfs_true:
        movei   1,1
        popj    17,

        .globl procfs_v1_is_file
procfs_v1_is_file:
        hlrz    4,1
        move    5,4
        lsh     4,-014
        andi    4,077
        caie    4,3
        jrst    procfs_false
        andi    5,07777
        cail    5,3
        cail    5,7
        jrst    procfs_false
        subi    5,1
        hrrz    4,1
        cail    4,0100
        jrst    procfs_false
        move    6,4
        lsh     6,1
        move    7,proc_v1_table(6)
        lsh     7,-017
        andi    7,07
        jumpe   7,procfs_false
        jumpe   2,procfs_file_field
        movem   4,(2)
procfs_file_field:
        jumpe   3,procfs_true
        movem   5,(3)
        jrst    procfs_true

        .globl procfs_v1_find_pid
procfs_v1_find_pid:
        jumpe   2,procfs_minus1
        move    6,1
        move    7,2
        movei   3,0
procfs_find_loop:
        move    4,3
        lsh     4,1
        move    5,proc_v1_table(4)
        move    0,5
        lsh     0,-017
        andi    0,07
        jumpe   0,procfs_find_next
        andi    5,0377
        came    5,6
        jrst    procfs_find_next
        movem   3,(7)
        movei   1,0
        popj    17,
procfs_find_next:
        addi    3,1
        cail    3,0100
        jrst    procfs_minus1
        jrst    procfs_find_loop

procfs_false:
        movei   1,0
        popj    17,
procfs_null:
        movei   1,0
        popj    17,
procfs_minus1:
        seto    1,
        popj    17,
