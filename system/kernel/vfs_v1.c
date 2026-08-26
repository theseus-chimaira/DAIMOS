#include "vfs_v1.h"

int
vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars)
{
        unsigned int i;

        if (name == 0 || chars > 6U)
                return -1;
        name->chars = chars;
        name->words[0] = word;
        for (i = 1U; i < VFS_V1_NAME_WORDS; ++i)
                name->words[i] = 0;
        return 0;
}

int
vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars)
{
        if (name == 0)
                return 0;
        return name->chars == chars && name->words[0] == word;
}

static void
vfs_v1_name_put_digit(struct vfs_v1_name *name, unsigned int pos,
    unsigned int digit)
{
        unsigned int wi;
        unsigned int shift;

        wi = pos / 6U;
        shift = 30U - (pos % 6U) * 6U;
        name->words[wi] |= ((kword_t)(020U + digit) & 077UL) << shift;
}

int
vfs_v1_name_set_uint(struct vfs_v1_name *name, unsigned int value)
{
        unsigned int divisor;
        unsigned int chars;
        unsigned int i;

        if (name == 0)
                return -1;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                name->words[i] = 0;
        divisor = 1U;
        chars = 1U;
        while (value / divisor >= 10U) {
                if (divisor > ((~0U) / 10U))
                        break;
                divisor *= 10U;
                ++chars;
        }
        if (chars > VFS_V1_NAME_MAX_CHARS)
                return -1;
        name->chars = chars;
        for (i = 0U; i < chars; ++i) {
                vfs_v1_name_put_digit(name, i, value / divisor % 10U);
                if (divisor > 1U)
                        divisor /= 10U;
        }
        return 0;
}

int
vfs_v1_name_get_uint(const struct vfs_v1_name *name, unsigned int *valuep)
{
        unsigned int i;
        unsigned int wi;
        unsigned int shift;
        unsigned int code;
        unsigned int value;
        unsigned int digit;

        if (name == 0 || valuep == 0 || name->chars == 0U ||
            name->chars > VFS_V1_NAME_MAX_CHARS)
                return -1;
        value = 0U;
        for (i = 0U; i < name->chars; ++i) {
                wi = i / 6U;
                shift = 30U - (i % 6U) * 6U;
                code = (unsigned int)((name->words[wi] >> shift) & 077UL);
                if (code < 020U || code > 031U)
                        return -1;
                digit = code - 020U;
                if (value > ((~0U) - digit) / 10U)
                        return -1;
                value = value * 10U + digit;
        }
        *valuep = value;
        return 0;
}

int
vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp)
{
        unsigned int shift;

        if (chp == 0 || nchars > 6U)
                return -1;
        if (off < (kword_t)nchars) {
                shift = 30U - (unsigned int)off * 6U;
                *chp = (unsigned int)(((word >> shift) & 077UL) + 040U);
                return 1;
        }
        if (off == (kword_t)nchars) {
                *chp = '\r';
                return 1;
        }
        if (off == (kword_t)nchars + 1U) {
                *chp = '\n';
                return 1;
        }
        return 0;
}
