#include "dsys.h"
#include "u.h"
#include "dtfs_probe.h"

int
main(int argc, kword_t **argv)
{
        int deep;
        int type;
        int unit;
        unsigned int i;
        static const char *const names[8] = {
                "/DEV/DTC0", "/DEV/DTC1", "/DEV/DTC2", "/DEV/DTC3",
                "/DEV/DTC4", "/DEV/DTC5", "/DEV/DTC6", "/DEV/DTC7"
        };

        if (argc != 2 && argc != 3)
                return 1;
        unit = -1;
        for (i = 0U; i < 8U; ++i)
                if (u_s6_eq(argv[1], names[i])) {
                        unit = (int)i;
                        break;
                }
        if (unit < 0 ||
            (argc == 3 && !u_s6_eq(argv[2], "DEEP")))
                return 1;
        deep = argc == 3;
        type = dtfs_probe_unit((unsigned int)unit, deep);
        if (type < 0)
                return 1;
        return type;
}
