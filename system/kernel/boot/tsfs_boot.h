#ifndef DAIMON_TSFS_BOOT_H
#define DAIMON_TSFS_BOOT_H

#include "kcore.h"

int tsfs_boot_select(unsigned int ordinal);
int tsfs_boot_mount_root(void);
/* Physical DTC units in the validated selected TSFS set; zero if absent. */
unsigned int tsfs_boot_selected_unit_mask_v22(void);

#endif
