; module-table-test.s -- production modules plus test-only device checkpoint.
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
        .globl device_test_guard_minit
        .globl device_test_nested_minit
        .globl device_test_minit
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
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end
__kinit_image_start:
__minit_table_begin:
        ; Install a test-only KCORE guard before any device can interrupt.
        .word device_test_guard_minit,,0
        .word cty_minit,,cty_mres_package
        .word clk_minit,,clk_mres_package
        .word ptr_minit,,io7_mres_package
        .word ptp_minit,,io7_mres_package
        .word cr_minit,,io7_mres_package
        .word cp_minit,,io7_mres_package
        ; Exercise nesting while the seventh handler slot is still free.
        .word device_test_nested_minit,,0
        .word dcs_minit,,dcs_mres_package
        .word ge_minit,,ge_mres_package
        .word dpy_minit,,dpy_mres_package
        .word tty_minit,,tty_mres_package
        .word wcnsls_minit,,wcnsls_mres_package
        .word ocnsls_minit,,ocnsls_mres_package
        .word dtc_minit,,storage_mres_package
        .word mtc_minit,,storage_mres_package
        .word dsk_minit,,storage_mres_package
        .word slv_minit,,0
        .word device_test_minit,,0
__minit_table_end:

; Force a level-4 interrupt while a test level-7 handler is active.  The
; compact PI runtime must preserve the outer level's AC1..AC3 across nesting.
        .globl device_test_pi7_handler
        .globl device_test_pi7_handler_address
        .globl device_test_pi7_done
        .globl pdp10_pi_handler_return

device_test_pi7_handler:
        cono 0004,004010
        setom device_test_pi7_done
        jrst pdp10_pi_handler_return

        .data
device_test_pi7_handler_address:
        .word device_test_pi7_handler
device_test_pi7_done:
        .word 0
