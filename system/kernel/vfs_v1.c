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

static kword_t
vfs_v1_pid_digit(unsigned int digit, unsigned int pos)
{
        return ((kword_t)(020U + digit) & 077UL) << (30U - pos * 6U);
}

int
vfs_v1_name_set_pid(struct vfs_v1_name *name, unsigned int value)
{
        unsigned int chars;
        unsigned int digit;
        kword_t word;

        if (name == 0 || value > 0377U)
                return -1;
        word = 0;
        chars = 1U;
        if (value >= 100U) {
                digit = 0U;
                while (value >= 100U) {
                        value -= 100U;
                        ++digit;
                }
                word |= vfs_v1_pid_digit(digit, 0U);
                chars = 3U;
        } else if (value >= 10U) {
                chars = 2U;
        }
        if (chars >= 2U) {
                digit = 0U;
                while (value >= 10U) {
                        value -= 10U;
                        ++digit;
                }
                word |= vfs_v1_pid_digit(digit, chars - 2U);
        }
        word |= vfs_v1_pid_digit(value, chars - 1U);
        name->chars = chars;
        name->words[0] = word;
        name->words[1] = 0;
        name->words[2] = 0;
        name->words[3] = 0;
        return 0;
}

int
vfs_v1_name_get_pid(const struct vfs_v1_name *name, unsigned int *valuep)
{
        unsigned int chars;
        unsigned int i;
        unsigned int code;
        unsigned int value;

        if (name == 0 || valuep == 0)
                return -1;
        chars = name->chars;
        if (chars == 0U || chars > 3U)
                return -1;
        value = 0U;
        for (i = 0U; i < chars; ++i) {
                code = (unsigned int)((name->words[0] >> (30U - i * 6U)) & 077UL);
                if (code < 020U || code > 031U)
                        return -1;
                value = value * 10U + code - 020U;
        }
        if (value > 0377U)
                return -1;
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


static const kword_t vfs_v1_pow10[] = {
        10000000000UL, 1000000000UL, 100000000UL, 10000000UL,
        1000000UL, 100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL
};

int
vfs_v1_decimal_readchar(kword_t value, kword_t off, unsigned int *chp)
{
        unsigned int digit;
        unsigned int digits;
        unsigned int first;
        unsigned int pos;
        kword_t power;

        if (chp == 0)
                return -1;
        first = 0U;
        while (first < 10U && value < vfs_v1_pow10[first])
                ++first;
        digits = 11U - first;
        if (off < (kword_t)digits) {
                digit = 0U;
                for (pos = 0U; pos <= (unsigned int)off; ++pos) {
                        power = vfs_v1_pow10[first + pos];
                        digit = 0U;
                        while (value >= power) {
                                value -= power;
                                ++digit;
                        }
                }
                *chp = '0' + digit;
                return 1;
        }
        if (off == (kword_t)digits) {
                *chp = '\r';
                return 1;
        }
        if (off == (kword_t)digits + 1U) {
                *chp = '\n';
                return 1;
        }
        return 0;
}
