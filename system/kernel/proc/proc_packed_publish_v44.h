/* Staged registry publication after a prepared, transition-held eviction. */
#ifndef DAIMOS_PROC_PACKED_PUBLISH_V44_H
#define DAIMOS_PROC_PACKED_PUBLISH_V44_H
#include "proc_packed_release_v43.h"

/* Must run before the eviction transaction releases TRANSITION. The
 * caller supplies a descriptor for an already allocated packed MM extent.
 * No state or registry word changes on validation/capacity failure.
 */
int proc_packed_publish_v44(unsigned int slot,
    const struct proc_packed_v1 *prepared);
#endif
