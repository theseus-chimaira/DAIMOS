#include "tsfs.h"

/*
 * The PDP-6 TSFS provider is implemented in tsfs_pdp10.s.  TSFS metadata is
 * deliberately simple enough that the assembly implementation is both much
 * smaller than KCC output and faster on the target.  Media-set discovery,
 * checksums, and structural validation remain in the transient userspace
 * MOUNT.TSFS helper; the resident provider validates only addresses it will
 * dereference.
 */
