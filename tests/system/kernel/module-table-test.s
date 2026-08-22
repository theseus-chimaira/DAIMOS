; module-table-test.s -- production MINITs plus test-only device checkpoint.
        .text
        .globl cty_minit
        .globl clk_minit
        .globl ptr_minit
        .globl ptp_minit
        .globl cr_minit
        .globl cp_minit
        .globl wcnsls_minit
        .globl ocnsls_minit
        .globl slv_minit
        .globl device_test_minit
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end
__kinit_image_start:
__minit_table_begin:
        .word cty_minit,,clk_minit
        .word ptr_minit,,ptp_minit
        .word cr_minit,,cp_minit
        .word wcnsls_minit,,ocnsls_minit
        .word slv_minit,,device_test_minit
__minit_table_end:
