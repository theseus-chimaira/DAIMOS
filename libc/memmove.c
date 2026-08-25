/* memmove.c - copy byte ranges that may overlap. */

void *
memmove(void *dst, const void *src, unsigned int n)
{
    unsigned char *d;
    const unsigned char *s;

    d = (unsigned char *)dst;
    s = (const unsigned char *)src;
    if (d == s || n == 0U)
        return dst;

    if (d < s) {
        while (n != 0U) {
            *d++ = *s++;
            --n;
        }
    } else {
        d += n;
        s += n;
        while (n != 0U) {
            *--d = *--s;
            --n;
        }
    }
    return dst;
}
