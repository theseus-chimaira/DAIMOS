; module_table.s -- MINIT/MRES pairs for built-in modules.
        .text
        .globl cty_minit
        .globl clk_minit
        .globl ptr_minit
        .globl ptp_minit
        .globl lpt_minit
        .globl cr_minit
        .globl cp_minit
        .globl dcs_minit
        .globl ge_minit
        .globl dpy_minit
        .globl tty_minit
        .globl wcnsls_minit
        .globl ocnsls_minit
        .globl dtc_minit
        .globl mtc_minit
        .globl dsk_minit
        .globl drm236_minit
        .globl root_select_minit
        .globl storage_minit
        .globl slv_minit
        .globl memfs_minit
        .globl dtfs_minit
        .globl blockset_minit
.if KINIT_BADMAP
        .globl badmap_minit
.endif
        .globl d6fs_minit
        .globl mfsdev_minit
        .globl cty_mres_package
        .globl clk_mres_package
.if KINIT_FULL
        .globl ptr_mres_package
        .globl ptp_mres_package
        .globl cr_mres_package
        .globl cp_mres_package
.endif
        .globl dcs_mres_package
        .globl ge_mres_package
.if KINIT_FULL
        .globl dpy_mres_package
.endif
        .globl tty_mres_package
.if KINIT_FULL
        .globl wcnsls_mres_package
        .globl ocnsls_mres_package
        .globl tape_mres_package
.endif
        .globl dsk_mres_package
.if KINIT_FULL
        .globl drm236_mres_package
        .globl slv_mres_package
        .globl memfs_mres_package
        .globl dtfs_mres_package
.endif
        .globl blockset_mres_package
.if KINIT_BADMAP
        .globl badmap_mres_package
.endif
        .globl d6fs_mres_package
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end

__kinit_image_start:

; The three Type-136 storage devices share one MINIT implementation.  Tiny
; entry stubs provide the device kind without duplicating probe/install code.
dtc_minit:
        setz 1,
        move 2,[0446443000000]          ; SIXBIT /DTC   /
        jrst storage_minit
mtc_minit:
        movei 1,1
        move 2,[0556443000000]          ; SIXBIT /MTC   /
        jrst storage_minit
dsk_minit:
        movei 1,2
        move 2,[0446353222720]          ; SIXBIT /DSK270/
        jrst storage_minit

__minit_table_begin:
        .word cty_minit,,cty_mres_package
        .word clk_minit,,clk_mres_package
.if KINIT_FULL
        .word ptr_minit,,ptr_mres_package
        .word ptp_minit,,ptp_mres_package
        .word lpt_minit,,0
        .word cr_minit,,cr_mres_package
        .word cp_minit,,cp_mres_package
.endif
        .word dcs_minit,,dcs_mres_package
        .word ge_minit,,ge_mres_package
.if KINIT_FULL
        .word dpy_minit,,dpy_mres_package
.endif
        .word tty_minit,,tty_mres_package
.if KINIT_FULL
        .word wcnsls_minit,,wcnsls_mres_package
        .word ocnsls_minit,,ocnsls_mres_package
        .word dtc_minit,,tape_mres_package
        .word mtc_minit,,tape_mres_package
.endif
        .word dsk_minit,,dsk_mres_package
.if KINIT_FULL
        .word drm236_minit,,drm236_mres_package
.endif
        .word root_select_minit,,0
        .word blockset_minit,,blockset_mres_package
.if KINIT_BADMAP
        .word badmap_minit,,badmap_mres_package
.endif
.if KINIT_FULL
        .word memfs_minit,,memfs_mres_package
        .word dtfs_minit,,dtfs_mres_package
.endif
.if KINIT_FULL
        .word slv_minit,,slv_mres_package
.endif
        .word d6fs_minit,,d6fs_mres_package
        .word mfsdev_minit,,0
__minit_table_end:
