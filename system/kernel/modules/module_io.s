/**
 * @file module_io.s
 * @brief Disposable PDP-6 MINIT raw-device, PI, and boot-reader primitives.
 *
 * These routines are linked into KINIT, not permanent KCORE. They provide the
 * direct CONI/CONO/DATAI/DATAO probes needed before resident MRES drivers are
 * installed, initialize the fixed PDP-6 PI vectors, and allocate/seed the D6FS
 * boot reader. All code here is reclaimed after KINIT handoff.
 */
        .text
        .globl dtc_minit
        .globl mtc_minit
        .globl dsk_minit
        .globl storage_minit

; The three Type-136 storage devices share one C MINIT implementation.  Keep
; these tiny entry stubs with live KINIT code, not in module_table.s: the
; linker-generated MINIT table is copied to the KINIT stack and its source
; prefix is deliberately reclaimed before MINIT processing finishes.  Table
; entries therefore must not point back into that reclaimed prefix.
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

        .globl minit_pi_low_init
        .globl minit_pi_hw_clear
        .globl minit_pi_hw_set
        .globl minit_pi_request
        .globl pdp10_pi_level1
        .globl pdp10_pi_level2
        .globl pdp10_pi_level3
        .globl pdp10_pi_level4
        .globl pdp10_pi_level5
        .globl pdp10_pi_level6
        .globl pdp10_pi_level7
        .globl minit_cty_coni
        .globl minit_cty_cono
        .globl minit_clk_coni
        .globl minit_clk_cono
        .globl minit_ptr_coni
        .globl minit_ptr_cono
        .globl minit_ptp_coni
        .globl minit_ptp_cono
        .globl minit_lpt_coni
        .globl minit_lpt_cono
        .globl minit_cr_coni
        .globl minit_cr_cono
        .globl minit_cp_coni
        .globl minit_cp_cono
        .globl minit_dcs_coni
        .globl minit_dcs_cono
        .globl minit_gtyi_coni
        .globl minit_gtyi_cono
        .globl minit_gtyo_coni
        .globl minit_gtyo_cono
        .globl minit_dpy_coni
        .globl minit_dpy_cono
        .globl minit_wcnsls_cono
        .globl minit_wcnsls_datai
        .globl minit_wcnsls_plot
        .globl minit_storage_probe
        .globl minit_drm236_probe
        .globl minit_slv_coni
        .globl minit_slv_cono
        .globl d6fs_reader_bootstrap_call
        .globl d6fs_active_reader
        .globl d6fs_reader_slots
        .globl mm_alloc

; Allocate the boot-root D6FS reader from managed core.  The reader is
; mount-owned dynamic state; this helper is reclaimed with KINIT.
; AC1=packed backing callbacks.  d6fs_active_reader is the fixed KCORE
; active-reader word and also serves as mm_alloc's basep.
d6fs_reader_bootstrap_call:
        push 017,1                       ; backing.ops
        push 017,[d6fs_active_reader]  ; fifth arg: basep
        movei 1,021
        movei 2,3
        movei 3,010
        setz 4,
        pushj 017,mm_alloc
        jumpn 1,d6fs_reader_bootstrap_done
        move 5,d6fs_active_reader
        move 6,5
        movei 7,021
d6fs_reader_bootstrap_zero:
        setzm 0(6)
        addi 6,1
        sojg 7,d6fs_reader_bootstrap_zero
        setzm 017(5)                    ; backing.opaque
        move 4,-1(017)                  ; saved backing.ops
        movem 4,016(5)
        movem 5,d6fs_reader_slots        ; root is VFS mount id 1
d6fs_reader_bootstrap_done:
        sub 017,[2,,2]
        popj 017,

; Install the fixed PDP-6 PI vectors and clear their private AC save cells.
; KINIT has already copied the Stage1 040/041 handoff into KCORE, so PI no
; longer needs to preserve those low-core words itself.
minit_pi_low_init:
        setzm 000020
        move 1,[000020,,000021]
        blt 1,000037
        move 1,[jsr pdp10_pi_level1]
        movem 1,000042
        setzm 000043
        move 1,[jsr pdp10_pi_level2]
        movem 1,000044
        setzm 000045
        ; PI3 is the Type-136 block-data channel.  BLKI/BLKO is installed
        ; in 046 by the resident storage service; on the final word it falls
        ; through to this JSR in 047 for normal saved-AC completion handling.
        setzm 000046
        move 1,[jsr pdp10_pi_level3]
        movem 1,000047
        move 1,[jsr pdp10_pi_level4]
        movem 1,000050
        setzm 000051
        move 1,[jsr pdp10_pi_level5]
        movem 1,000052
        setzm 000053
        move 1,[jsr pdp10_pi_level6]
        movem 1,000054
        setzm 000055
        move 1,[jsr pdp10_pi_level7]
        movem 1,000056
        setzm 000057
        popj 017,

minit_pi_hw_clear:
        cono 0004,010000
        popj 017,

; AC1 contains the complete level-enable mask.  Enable PI globally and add
; these levels without disturbing already enabled levels.
minit_pi_hw_set:
        andi 1,0177
        iori 1,002200
        cono 0004,0(1)
        popj 017,

; AC1 contains one or more PI level mask bits to request in software.
minit_pi_request:
        andi 1,0177
        iori 1,004000
        cono 0004,0(1)
        popj 017,

minit_cty_coni:
        coni 0120,1
        popj 017,
minit_cty_cono:
        cono 0120,0(1)
        popj 017,
minit_clk_coni:
        coni 0000,1
        popj 017,
minit_clk_cono:
        cono 0000,0(1)
        popj 017,
minit_ptr_coni:
        coni 0104,1
        popj 017,
minit_ptr_cono:
        cono 0104,0(1)
        popj 017,
minit_ptp_coni:
        coni 0100,1
        popj 017,
minit_ptp_cono:
        cono 0100,0(1)
        popj 017,
minit_lpt_coni:
        coni 0124,1
        popj 017,
minit_lpt_cono:
        cono 0124,0(1)
        popj 017,
minit_cr_coni:
        coni 0150,1
        popj 017,
minit_cr_cono:
        cono 0150,0(1)
        popj 017,
minit_cp_coni:
        coni 0110,1
        popj 017,
minit_cp_cono:
        cono 0110,0(1)
        popj 017,
minit_dcs_coni:
        coni 0300,1
        popj 017,
minit_dcs_cono:
        cono 0300,0(1)
        popj 017,
minit_gtyi_coni:
        coni 0070,1
        popj 017,
minit_gtyi_cono:
        cono 0070,0(1)
        popj 017,
minit_gtyo_coni:
        coni 0750,1
        popj 017,
minit_gtyo_cono:
        cono 0750,0(1)
        popj 017,
minit_dpy_coni:
        coni 0130,1
        popj 017,
minit_dpy_cono:
        cono 0130,0(1)
        popj 017,
minit_wcnsls_cono:
        cono 0420,0(1)
        popj 017,
minit_wcnsls_datai:
        datai 0420,1
        popj 017,
minit_wcnsls_plot:
        datao 0420,1
        popj 017,
; Probe the three Type-136 storage controllers through one compact entry.
; AC1 kind: 0 DTC, 1 MTC, 2 DSK.  Return status in AC1 with DSK normalized
; from its left half so all three callers test the same PI-level bits.
minit_storage_probe:
        jumpe 1,minit_storage_probe_dtc
        caie 1,1
        jrst minit_storage_probe_dsk
        cono 0220,5
        coni 0220,1
        cono 0220,0
        popj 017,
minit_storage_probe_dtc:
        cono 0210,5
        coni 0210,1
        cono 0210,0
        popj 017,
minit_storage_probe_dsk:
        cono 0270,5
        coni 0270,1
        cono 0270,0
        hlrz 1,1
        popj 017,
; Probe the Type 167 I/O Processor independently of Type-136 storage.
; Use PI level 7 only as a recognizable probe value, then leave DP disabled.
minit_drm236_probe:
        cono 0010,7
        coni 0010,1
        cono 0010,0
        popj 017,
minit_slv_coni:
        coni 0020,1
        popj 017,
minit_slv_cono:
        cono 0020,0(1)
        popj 017,
