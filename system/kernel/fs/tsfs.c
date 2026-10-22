#include "tsfs.h"

/*
 * The PDP-6 TSFS provider is implemented in tsfs_runtime.s.  TSFS metadata is
 * deliberately simple enough that the assembly implementation is both much
 * smaller than KCC output and faster on the target.  Boot-root discovery, checksums and structural validation are handled
 * by KINIT before the resident read-only provider is invoked.
 */
