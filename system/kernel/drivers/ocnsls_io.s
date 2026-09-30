/**
 * @file ocnsls_io.s
 * @brief Tiny resident PDP-6 old Spacewar console-switch reader.
 *
 * Device 0724 has no interrupt path, writable state, or useful presence probe.
 * The installed MRES therefore consists only of DATAI, MonitorFS accounting,
 * and return. A zero word is both the normal no-switches state and the harmless
 * result of a null implementation, so no per-device BSS is required.
 */
        .globl mfsdev_io_in
        .text
        .globl ocnsls_read
/**
 * @brief Return the raw two-player Spacewar switch word.
 * @return AC1 = DATAI 0724; no other AC is clobbered.
 *
 * AC17 is the normal return stack. One completed read increments the OCNSLS
 * MonitorFS input counter before returning.
 */
ocnsls_read:
        datai 0724,1
        aos mfsdev_io_in+013
        popj 017,
