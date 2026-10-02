#include "text.h"

static int
config_show(const char *name, const char *path)
{
        struct u_text_reader r;
        char line[192];
        int rc;

        if (u_puts(1, name) != 0 || u_crlf(1) != 0)
                return 1;
        if (u_text_open(&r, path) != 0)
                return 1;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0)
                if (u_puts(1, "  ") != 0 || u_puts(1, line) != 0 ||
                    u_crlf(1) != 0) {
                        u_text_close(&r);
                        return 1;
                }
        u_text_close(&r);
        return rc == U_TEXT_EOF ? 0 : 1;
}

int
main(int argc, kword_t **argv)
{
        (void)argv;
        if (argc != 1) {
                (void)u_puts(2,
                    "CONFIGCTL: LIVE V1 STAGING WAS RETIRED; EDIT /CONFIG FILES");
                (void)u_crlf(2);
                return 2;
        }
        if (config_show("SYSTEM", "/CONFIG/SYSTEM") != 0 ||
            config_show("FSTAB", "/CONFIG/FSTAB") != 0 ||
            config_show("INITTAB", "/CONFIG/INITTAB") != 0)
                return 1;
        return 0;
}
