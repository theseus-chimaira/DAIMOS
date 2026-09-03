#include "fs_mres.h"
#include "dtfs.h"

int
dtfs_mres_bind(unsigned int read_addr, unsigned int write_addr)
{
        dtfs_dtc_read_addr = read_addr;
        dtfs_dtc_write_addr = write_addr;
        return 0;
}
