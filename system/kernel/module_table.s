; module_table.s -- MINIT/MRES pairs for built-in modules.
        .text
        .globl cty_minit
        .globl clk_minit
        .globl ptr_minit
        .globl ptp_minit
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
        .globl slv_minit
        .globl cty_mres_package
        .globl clk_mres_package
        .globl io7_mres_package
        .globl dcs_mres_package
        .globl ge_mres_package
        .globl dpy_mres_package
        .globl tty_mres_package
        .globl wcnsls_mres_package
        .globl ocnsls_mres_package
        .globl storage_mres_package
        .globl slv_mres_package
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end
__kinit_image_start:
__minit_table_begin:
        .word cty_minit,,cty_mres_package
        .word clk_minit,,clk_mres_package
        .word ptr_minit,,io7_mres_package
        .word ptp_minit,,io7_mres_package
        .word cr_minit,,io7_mres_package
        .word cp_minit,,io7_mres_package
        .word dcs_minit,,dcs_mres_package
        .word ge_minit,,ge_mres_package
        .word dpy_minit,,dpy_mres_package
        .word tty_minit,,tty_mres_package
        .word wcnsls_minit,,wcnsls_mres_package
        .word ocnsls_minit,,ocnsls_mres_package
        .word dtc_minit,,storage_mres_package
        .word mtc_minit,,storage_mres_package
        .word dsk_minit,,storage_mres_package
        .word slv_minit,,slv_mres_package
__minit_table_end:
