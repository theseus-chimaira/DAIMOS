/* memcpy.c - copy non-overlapping byte ranges. */

void *
memcpy(void *dst, const void *src, unsigned int n)
{
    unsigned char *d;
    const unsigned char *s;

    d = (unsigned char *)dst;
    s = (const unsigned char *)src;
    while (n != 0U) {
        *d++ = *s++;
        --n;
    }
    return dst;
}
