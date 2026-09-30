/**
 * @file kinit_late_end.s
 * @brief End boundary of the protected late-KINIT text range.
 *
 * The linker places this marker after every instruction which must remain
 * physically untouched while kinit_late_start() finishes bootstrap.  The
 * interval [__kinit_late_begin, __kinit_late_end) is returned to MM only when
 * no further allocation can overwrite the still-executing late KINIT path.
 *
 * Keep this object last in the protected late-KINIT group: moving executable
 * late-bootstrap code after the marker would make that code reclaimable too
 * early.
 */

        .text
        .globl __kinit_late_end
__kinit_late_end:
