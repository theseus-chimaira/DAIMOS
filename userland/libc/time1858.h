/* Civil UTC conversion for DAIMOS unsigned 36-bit epoch seconds.
 * Epoch: 1858-11-17 00:00:00 UTC (MJD day zero).
 * Zero is reserved for an unset system clock at the syscall boundary.
 */
#ifndef DAIMOS_TIME1858_H
#define DAIMOS_TIME1858_H
#include "dsys.h"
struct time1858_utc {
    unsigned int year, month, day, hour, minute, second;
};
int time1858_encode(const struct time1858_utc *date, kword_t *seconds);
int time1858_decode(kword_t seconds, struct time1858_utc *date);
#define TIME1858_UNIX_OFFSET 3506716800UL
#endif
