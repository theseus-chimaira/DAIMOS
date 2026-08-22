; module_io.s -- disposable MINIT-only raw device probes.
        .text
        .globl minit_cty_coni
        .globl minit_cty_cono
        .globl minit_clk_coni
        .globl minit_clk_cono
        .globl minit_ptr_coni
        .globl minit_ptr_cono
        .globl minit_ptp_coni
        .globl minit_ptp_cono
        .globl minit_cr_coni
        .globl minit_cr_cono
        .globl minit_cp_coni
        .globl minit_cp_cono
        .globl minit_wcnsls_cono
        .globl minit_slv_coni
        .globl minit_slv_cono

minit_cty_coni:
        coni 0120,1
        popj 17,
minit_cty_cono:
        cono 0120,0(1)
        popj 17,
minit_clk_coni:
        coni 0000,1
        popj 17,
minit_clk_cono:
        cono 0000,0(1)
        popj 17,
minit_ptr_coni:
        coni 0104,1
        popj 17,
minit_ptr_cono:
        cono 0104,0(1)
        popj 17,
minit_ptp_coni:
        coni 0100,1
        popj 17,
minit_ptp_cono:
        cono 0100,0(1)
        popj 17,
minit_cr_coni:
        coni 0150,1
        popj 17,
minit_cr_cono:
        cono 0150,0(1)
        popj 17,
minit_cp_coni:
        coni 0110,1
        popj 17,
minit_cp_cono:
        cono 0110,0(1)
        popj 17,
minit_wcnsls_cono:
        cono 0420,0(1)
        popj 17,
minit_slv_coni:
        coni 0020,1
        popj 17,
minit_slv_cono:
        cono 0020,0(1)
        popj 17,
