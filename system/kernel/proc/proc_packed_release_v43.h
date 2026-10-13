/* Inactive compact packed teardown. MM, not a duplicated span, is authority.
 * Caller holds process lifecycle exclusion and TRANSITION.
 */
#ifndef DAIMOS_PROC_PACKED_RELEASE_V43_H
#define DAIMOS_PROC_PACKED_RELEASE_V43_H
#include "proc_packed_boot_v38.h"

int proc_packed_release_v43(unsigned int slot);
#endif
