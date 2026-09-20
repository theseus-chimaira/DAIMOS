#include "tsfs_scan.h"

static int
tsfs_seed_unit(const kword_t *path, unsigned int *unitp)
{
        unsigned int unit;
        static const char *const names[8] = {
                "/DEVICE/DTC0", "/DEVICE/DTC1", "/DEVICE/DTC2", "/DEVICE/DTC3",
                "/DEVICE/DTC4", "/DEVICE/DTC5", "/DEVICE/DTC6", "/DEVICE/DTC7"
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
                (void)u_puts(2, "USAGE: MOUNT.TSFS /DEVICE/DTCN TARGET");
                (void)u_crlf(2);
                return 1;
        }
        if (tsfs_scan(seed, &scan) != 0 ||
            tsfs_build_mount_handoff(&scan, handoff) != 0) {
                (void)u_puts(2, "MOUNT.TSFS: NO COMPLETE SET");
                (void)u_crlf(2);
                return 1;
        }
        if (dsys_tsfs_mount(handoff, argv[2], SYS_MOUNT_RDONLY) != 0) {
                (void)u_puts(2, "MOUNT.TSFS: MOUNT FAILED");
                (void)u_crlf(2);
                return 1;
        }
        return 0;
}
