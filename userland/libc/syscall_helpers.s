; syscall_helpers.s -- DAIMOS libc syscall helpers which are not
; one-instruction native ABI veneers.  These must remain callable and are
; deliberately excluded from dlink --daimos-uuo-relax.

        .text

        .globl dsys_mkdir

dsys_mkdir:
        movei 2,0777
        uuo 051,0(1)
        popj 17,

