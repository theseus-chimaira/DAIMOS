; module_table.s -- MINIT/MRES pairs for built-in modules.
        .text
        .globl cty_minit
        .globl clk_minit
        .globl ptr_minit
        .globl ptp_minit
        .globl cr_minit
        .globl cp_minit
        .globl dcs_minit
        .globl wcnsls_minit
        .globl ocnsls_minit
        .globl slv_minit
        .globl cty_mres_package
        .globl clk_mres_package
        .globl ptr_mres_package
        .globl ptp_mres_package
        .globl cr_mres_package
        .globl cp_mres_package
        .globl dcs_mres_package
        .globl wcnsls_mres_package
        .globl ocnsls_mres_package
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end
__kinit_image_start:
__minit_table_begin:
        .word cty_minit,,cty_mres_package
        .word clk_minit,,clk_mres_package
        .word ptr_minit,,ptr_mres_package
        .word ptp_minit,,ptp_mres_package
        .word cr_minit,,cr_mres_package
        .word cp_minit,,cp_mres_package
        .word dcs_minit,,dcs_mres_package
        .word wcnsls_minit,,wcnsls_mres_package
        .word ocnsls_minit,,ocnsls_mres_package
        .word slv_minit,,0
__minit_table_end:
