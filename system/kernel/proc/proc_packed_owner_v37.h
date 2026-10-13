/* Shared packed lifecycle for either v36 descriptor layout. Inactive until
 * KINIT allocates and publishes the selected layout and process paths use it.
 */
#ifndef DAIMOS_PROC_PACKED_OWNER_V37_H
#define DAIMOS_PROC_PACKED_OWNER_V37_H
#include "proc_packed_layout_v36.h"

int proc_packed_owner_lookup_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot, struct proc_packed_v1 *descriptor);
int proc_packed_owner_publish_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot, const struct proc_packed_v1 *prepared);
/* Called with lifecycle exclusion and PROC_TRANSITION held. On failure the
 * MM allocation, storage state and layout must remain unchanged. */
int proc_packed_owner_discard_v37(struct proc_packed_layout_v36 *layout,
    unsigned int slot);
#endif
