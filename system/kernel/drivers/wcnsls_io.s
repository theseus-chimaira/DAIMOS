/**
 * @file wcnsls_io.s
 * @brief Resident PDP-6 Spacewar console and color-scope primitives.
 *
 * Device 0420 needs no interrupt handler or BSS. The MRES merely exposes raw
 * DATAI, CONO, and DATAO operations after KINIT has established device presence.
 * MonitorFS counts input reads and plotted points; control-word writes are not
 * treated as user-visible output operations.
 */
        .globl mfsdev_io_in
        .globl mfsdev_io_out
        .text
        .globl wcnsls_read
        .globl wcnsls_cono
        .globl wcnsls_plot
/**
 * @brief Read all four active-low Spacewar switch banks.
 * @return AC1 = raw DATAI 0420 value; no other AC is clobbered.
 */
wcnsls_read:
        datai 0420,1
        aos mfsdev_io_in+012
        popj 017,
/**
 * @brief Write one raw WCNSLS control/color word.
 * @param AC1 CONO value; AC1 is preserved by the I/O instruction.
 */
wcnsls_cono:
        cono 0420,0(1)
        popj 017,
/**
 * @brief Plot one packed 9-bit X/Y point on the optional color scope.
 * @param AC1 Packed WCNSLS_COORD() word.
 *
 * One MonitorFS output operation is counted for each DATAO point.
 */
wcnsls_plot:
        datao 0420,1
        aos mfsdev_io_out+012
        popj 017,
