#include "dsk270.h"

unsigned int dsk270_read_addr_v1;
unsigned int dsk270_write_addr_v1;

int dsk270_call(unsigned int address, kword_t raw_address, kword_t *block);

static int
dsk270_io(unsigned int address, unsigned int unit, kword_t sector,
    kword_t *buf)
{
        struct dsk270_addr addr;

        if (address == 0U || buf == 0 ||
            dsk270_make_addr(unit, sector, &addr) != 0)
                return -1;
        return dsk270_call(address, addr.raw, buf);
}

int
dsk270_read_sector(unsigned int unit, kword_t sector, kword_t *buf)
{
        return dsk270_io(dsk270_read_addr_v1, unit, sector, buf);
}

int
dsk270_write_sector(unsigned int unit, kword_t sector, const kword_t *buf)
{
        return dsk270_io(dsk270_write_addr_v1, unit, sector, (kword_t *)buf);
}
