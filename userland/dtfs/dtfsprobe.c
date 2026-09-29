#include "dsys.h"
#include "u.h"
#include "dtfs_probe.h"

int
main(int argc, kword_t **argv)
{
        int deep;
        int type;

        if ((argc != 2 && argc != 3) || !u_s6_eq(argv[1], "/DEV/DTC0") ||
            (argc == 3 && !u_s6_eq(argv[2], "DEEP")))
                return 1;
        deep = argc == 3;
        type = dtfs_probe_unit(0U, deep);
        if (type < 0)
                return 1;
        return type;
}
