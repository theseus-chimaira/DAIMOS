#include "tsfs_scan.h"

static int
tsfs_seed_unit(const kword_t *path, unsigned int *unitp)
{
        unsigned int unit;
        static const char *const names[8] = {
                "/DEV/DTC0", "/DEV/DTC1", "/DEV/DTC2", "/DEV/DTC3",
                "/DEV/DTC4", "/DEV/DTC5", "/DEV/DTC6", "/DEV/DTC7"
        };

        if (path == 0 || unitp == 0)
                return -1;
        for (unit = 0U; unit < 8U; ++unit) {
                if (u_s6_eq(path, names[unit])) {
                        *unitp = unit;
                        return 0;
                }
        }
        return -1;
}

int
main(int argc, kword_t **argv)
{
        struct tsfs_scan_result scan;
        kword_t handoff[SYS_TSFS_MOUNT_WORDS];
        unsigned int seed;

        if (argc != 3 || tsfs_seed_unit(argv[1], &seed) != 0) {
                (void)u_puts(2, "USAGE: MOUNT.TSFS /DEV/DTCN TARGET");
                (void)u_crlf(2);
                return 1;
        }
        if (tsfs_scan(seed, &scan) != 0 ||
            tsfs_build_mount_handoff(&scan, handoff) != 0) {
                (void)u_puts(2, "MOUNT.TSFS: NO COMPLETE SET");
                (void)u_crlf(2);
                return 1;
        }
        if (dsys_tsfs_mount(&handoff[SYS_TSFS_FILE_LOC], argv[2],
            SYS_MOUNT_RDONLY) != 0) {
                (void)u_puts(2, "MOUNT.TSFS: MOUNT FAILED");
                (void)u_crlf(2);
                return 1;
        }
        return 0;
}
