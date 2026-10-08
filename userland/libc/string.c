void *
memset(void *dst, int value, unsigned int count)
{
        unsigned char *p;

        p = (unsigned char *)dst;
        while (count-- != 0U)
                *p++ = (unsigned char)value;
        return dst;
}

int
memcmp(void *a, void *b, unsigned int count)
{
        unsigned char *pa;
        unsigned char *pb;

        pa = (unsigned char *)a;
        pb = (unsigned char *)b;
        while (count-- != 0U) {
                if (*pa != *pb)
                        return *pa < *pb ? -1 : 1;
                ++pa;
                ++pb;
        }
        return 0;
}

unsigned int
strlen(char *s)
{
        char *p;

        p = s;
        while (*p != 0)
                ++p;
        return (unsigned int)(p - s);
}

int
strcmp(char *a, char *b)
{
        while (*a != 0 && *a == *b) {
                ++a;
                ++b;
        }
        return (unsigned char)*a - (unsigned char)*b;
}

int
strncmp(char *a, char *b, unsigned int count)
{
        while (count-- != 0U) {
                if (*a != *b)
                        return (unsigned char)*a - (unsigned char)*b;
                if (*a == 0)
                        return 0;
                ++a;
                ++b;
        }
        return 0;
}

char *
strcpy(char *dst, char *src)
{
        char *ret;

        ret = dst;
        while ((*dst++ = *src++) != 0)
                ;
        return ret;
}

char *
strncpy(char *dst, char *src, unsigned int count)
{
        char *ret;

        ret = dst;
        while (count != 0U && *src != 0) {
                *dst++ = *src++;
                --count;
        }
        while (count-- != 0U)
                *dst++ = 0;
        return ret;
}

char *
strcat(char *dst, char *src)
{
        char *ret;

        ret = dst;
        while (*dst != 0)
                ++dst;
        while ((*dst++ = *src++) != 0)
                ;
        return ret;
}

char *
strncat(char *dst, char *src, unsigned int count)
{
        char *ret;

        ret = dst;
        while (*dst != 0)
                ++dst;
        while (count-- != 0U && *src != 0)
                *dst++ = *src++;
        *dst = 0;
        return ret;
}

char *
strchr(char *s, int ch)
{
        for (;;) {
                if ((unsigned char)*s == (unsigned char)ch)
                        return s;
                if (*s++ == 0)
                        return 0;
        }
}

char *
strrchr(char *s, int ch)
{
        char *last;

        last = 0;
        for (;;) {
                if ((unsigned char)*s == (unsigned char)ch)
                        last = s;
                if (*s++ == 0)
                        return last;
        }
}

char *
strtok(char *s, char *delim)
{
        static char *next;
        char *start;

        if (s != 0)
                next = s;
        if (next == 0)
                return 0;
        while (*next != 0 && strchr(delim, *next) != 0)
                ++next;
        if (*next == 0) {
                next = 0;
                return 0;
        }
        start = next;
        while (*next != 0 && strchr(delim, *next) == 0)
                ++next;
        if (*next != 0)
                *next++ = 0;
        else
                next = 0;
        return start;
}

/* Copy exactly count C characters. Source and destination must not overlap.
 * C characters are 9-bit on the native PDP-6 ABI, not host octets. */
void *
memcpy(void *dst, const void *src, unsigned int count)
{
        unsigned char *d;
        const unsigned char *s;

        d = (unsigned char *)dst;
        s = (const unsigned char *)src;
        while (count-- != 0U)
                *d++ = *s++;
        return dst;
}
