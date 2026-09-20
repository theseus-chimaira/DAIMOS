#include "tsfs_scan.h"

int
main(int argc, kword_t **argv)
{
        struct tsfs_scan_result r;
        unsigned int i;

        (void)argc;
        (void)argv;
        if (tsfs_scan(0U, &r) != 0) {
                (void)u_puts(2, "TSFSPROBE: NO COMPLETE SET ON DTC0");
                (void)u_crlf(2);
                return 1;
        }
        if (u_puts(1, "TSFS MEMBERS ") != 0 ||
            u_put_uint(1, r.members) != 0 || u_crlf(1) != 0)
                return 1;
        for (i = 0U; i < r.members; ++i) {
                if (u_puts(1, "MEMBER ") != 0 || u_put_uint(1, i) != 0 ||
                    u_puts(1, " DTC") != 0 || u_put_uint(1, r.unit[i]) != 0 ||
                    u_puts(1, " BLOCKS ") != 0 ||
                    u_put_uint(1, r.blocks[i]) != 0 || u_crlf(1) != 0)
                        return 1;
        }
        return 0;
}
