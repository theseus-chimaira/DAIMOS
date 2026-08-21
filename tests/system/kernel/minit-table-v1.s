; Packed halfword MINIT entry table.  No count or sentinel is stored.
        .text
        .globl test_minit_v1
        .globl test_minit2_v1
        .globl __kinit_image_start
        .globl __minit_table_begin
        .globl __minit_table_end
__kinit_image_start:
__minit_table_begin:
        .word test_minit_v1,,test_minit2_v1
__minit_table_end:
