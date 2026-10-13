/* Staged guard for vm_space_destroy(), shared by exit and creation abort.
 * Not linked into active VM until KINIT provides a persistent registry.
 */
#ifndef DAIMOS_PROC_PACKED_TEARDOWN_V39_H
#define DAIMOS_PROC_PACKED_TEARDOWN_V39_H
#include "proc_packed_owner_v37.h"

/* Called with lifecycle exclusion and TRANSITION set. Return MM_OK when
 * standard resident/disk cleanup may proceed. For PACKED, release the
 * registered extent first. An error MUST stop destruction and slot reuse.
 */
int proc_packed_teardown_guard_v39(struct proc_packed_layout_v36 *layout,
    unsigned int slot);
#endif
