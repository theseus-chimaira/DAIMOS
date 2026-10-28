/* Gregorian UTC conversion outside KCORE, without dynamic allocation. */
#include "time1858.h"
#define DAY_SECONDS 86400UL
#define TIME1858_MAX 0777777777777UL

static unsigned int leap(unsigned int y)
{
    return y % 4U == 0U && (y % 100U != 0U || y % 400U == 0U);
}
static unsigned int month_days(unsigned int y, unsigned int m)
{
    static const unsigned char md[12] =
        {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1U || m > 12U) return 0U;
    return (unsigned int)md[m-1U] + (m == 2U && leap(y));
}
/* Valid up to the complete 36-bit unsigned range, without a 64-bit type. */
int time1858_encode(const struct time1858_utc *d, kword_t *out)
{
    unsigned int y, m;
    kword_t days, value;
    if (!d || !out || d->year < 1858U || d->year > 4036U ||
        d->month < 1U || d->month > 12U ||
        d->day < 1U || d->day > month_days(d->year,d->month) ||
        d->hour > 23U || d->minute > 59U || d->second > 59U ||
        (d->year == 1858U && (d->month < 11U ||
         (d->month == 11U && d->day < 17U)))) return -1;
    days=0UL;
    for (y=1858U;y<d->year;++y)
        days += (kword_t)(365U + leap(y));
    for (m=1U;m<d->month;++m)
        days += (kword_t)month_days(d->year,m);
    days += d->day-1U;
    days -= 320U; /* days from 1858-01-01 to 1858-11-17 */
    if (days > (TIME1858_MAX - (kword_t)d->hour*3600UL -
        (kword_t)d->minute*60UL - d->second) / DAY_SECONDS)
        return -1;
    value=days * DAY_SECONDS + (kword_t)d->hour*3600UL +
        (kword_t)d->minute*60UL + d->second;
    if (value > TIME1858_MAX) return -1;
    *out=value;
    return 0;
}
int time1858_decode(kword_t value, struct time1858_utc *d)
{
    kword_t days, rem;
    unsigned int y, m, n;
    if (!d || value > TIME1858_MAX) return -1;
    days=value / DAY_SECONDS + 320UL;
    rem=value % DAY_SECONDS;
    for (y=1858U;y<=4036U;++y) {
        n=365U+leap(y);
        if (days < n) break;
        days-=n;
    }
    if (y>4036U) return -1;
    for (m=1U;m<=12U;++m) {
        n=month_days(y,m);
        if (days < n) break;
        days-=n;
    }
    if (m>12U) return -1;
    d->year=y; d->month=m; d->day=(unsigned int)days+1U;
    d->hour=(unsigned int)(rem/3600UL); rem%=3600UL;
    d->minute=(unsigned int)(rem/60UL); d->second=(unsigned int)(rem%60UL);
    return 0;
}
