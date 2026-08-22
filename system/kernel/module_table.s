; module_table.s -- packed MINIT entry table for built-in modules.
        .text
        .globl cty_minit
        .globl clk_minit
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end
__kinit_image_start:
__minit_table_begin:
        .word cty_minit,,clk_minit
__minit_table_end:
