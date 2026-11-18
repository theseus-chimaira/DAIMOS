/*
 * DAIMOS formatted output for the console and character buffers.
 *
 * No heap buffer or mutable static data area is required. The PDP-6 syscall
 * veneer in syscall.s accepts (fd, character) in AC1/AC2 and returns
 * zero on success.  stdout is the process's descriptor 1.
 *
 * One formatting engine serves printf/vprintf and assembly sprintf/vsprintf;
 * the output sink alone chooses the monitor or the caller's buffer.
 * Supported conversions: d, i, u, o, x, X, p, c, s, and %.
 * Integer length modifiers h and l, flags -+ #0, field width and
 * precision are handled.  Fixed %.0f is rounded by PDP-6-safe integer
 * assembly using the portable two-word D-format representation.
 * Other floating-point precisions currently return -1 explicitly.
 */
#include "dsys.h"
#include <stdarg.h>

extern int __daimos_fround0_v33(double, unsigned long *, int *);

struct format_sink {
        char *buffer;           /* NULL: WRITECHAR; otherwise append chars */
        int count;
};

static int
put_format_char(int ch, struct format_sink *sink)
{
        if (sink->buffer != 0)
                *sink->buffer++ = (char)ch;
        else if (dsys_writechar(1, ch) != 0)
                return -1;
        ++sink->count;
        return 0;
}

static int
put_padding(int ch, int amount, struct format_sink *sink)
{
        while (amount-- > 0)
                if (put_format_char(ch, sink) != 0)
                        return -1;
        return 0;
}

static int
format_digits(unsigned long value, unsigned int base, int uppercase,
    char *reversed)
{
        static const char lower[] = "0123456789abcdef";
        static const char upper[] = "0123456789ABCDEF";
        const char *digits;
        int n;

        digits = uppercase ? upper : lower;
        n = 0;
        do {
                reversed[n++] = digits[(unsigned int)(value % base)];
                value /= base;
        } while (value != 0UL);
        return n;
}

static int
format_number(unsigned long value, int negative, int base, int upper,
    int left, int zero, int plus, int space, int alternate, int width,
    int precision, struct format_sink *sink)
{
        char reversed[24];
        char sign;
        int n;
        int prefix;
        int zeros;
        int padding;
        int i;

        n = format_digits(value, (unsigned int)base, upper, reversed);
        if (precision == 0 && value == 0UL)
                n = 0;
        sign = negative ? '-' : plus ? '+' : space ? ' ' : 0;
        prefix = alternate && base == 16 && value != 0UL ? 2 : 0;
        if (alternate && base == 8 &&
            (n == 0 || reversed[n - 1] != '0') &&
            precision <= n)
                precision = n + 1;
        zeros = precision > n ? precision - n : 0;
        padding = width - n - zeros - prefix - (sign != 0);
        if (padding < 0)
                padding = 0;

        if (!left && !(zero && precision < 0) &&
            put_padding(' ', padding, sink) != 0)
                return -1;
        if (sign != 0 && put_format_char(sign, sink) != 0)
                return -1;
        if (prefix != 0 &&
            (put_format_char('0', sink) != 0 ||
             put_format_char(upper ? 'X' : 'x', sink) != 0))
                return -1;
        if (!left && zero && precision < 0 &&
            put_padding('0', padding, sink) != 0)
                return -1;
        if (put_padding('0', zeros, sink) != 0)
                return -1;
        for (i = n - 1; i >= 0; --i)
                if (put_format_char(reversed[i], sink) != 0)
                        return -1;
        if (left && put_padding(' ', padding, sink) != 0)
                return -1;
        return 0;
}

static int
format_render(char *format, va_list args, struct format_sink *sink)
{
        int left;
        int zero;
        int plus;
        int space;
        int alternate;
        int width;
        int precision;
        int long_value;
        int ch;
        int len;
        int i;
        int negative;
        int base;
        int upper;
    unsigned long magnitude;
    long signed_value;
    char *string;
    double fixed_value;

        if (format == 0)
                return -1;
        while ((ch = (unsigned char)*format++) != 0) {
                if (ch != '%') {
                        if (put_format_char(ch, sink) != 0)
                                return -1;
                        continue;
                }
                left = zero = plus = space = alternate = 0;
                for (;;) {
                        ch = (unsigned char)*format;
                        if (ch == '-')
                                left = 1;
                        else if (ch == '0')
                                zero = 1;
                        else if (ch == '+')
                                plus = 1;
                        else if (ch == ' ')
                                space = 1;
                        else if (ch == '#')
                                alternate = 1;
                        else
                                break;
                        ++format;
                }
                width = 0;
                if (*format == '*') {
                        width = va_arg(args, int);
                        ++format;
                        if (width < 0) {
                                if (-width == width)
                                        return -1;
                                width = -width;
                                left = 1;
                        }
                } else {
                        while (*format >= '0' && *format <= '9') {
                                if (width > 100000)
                                        return -1;
                                width = width * 10 + *format++ - '0';
                        }
                }
                precision = -1;
                if (*format == '.') {
                        ++format;
                        precision = 0;
                        if (*format == '*') {
                                precision = va_arg(args, int);
                                ++format;
                        } else {
                                while (*format >= '0' && *format <= '9') {
                                        if (precision > 100000)
                                                return -1;
                                        precision = precision * 10 +
                                            *format++ - '0';
                                }
                        }
                }
                long_value = 0;
                if (*format == 'h') {
                        ++format;
                } else if (*format == 'l') {
                        ++format;
                        long_value = 1;
                }
                ch = (unsigned char)*format;
                if (ch == 0 || ch == 'l' || ch == 'h')
                        return -1;
                ++format;

                if (ch == '%') {
                        if (put_format_char('%', sink) != 0)
                                return -1;
                        continue;
                }
                if (ch == 'c') {
                        int value = va_arg(args, int);
                        if (!left &&
                            put_padding(' ', width - 1, sink) != 0)
                                return -1;
                        if (put_format_char(value, sink) != 0 ||
                            (left && put_padding(' ', width - 1,
                                sink) != 0))
                                return -1;
                        continue;
                }
                if (ch == 's') {
                        string = va_arg(args, char *);
                        if (string == 0)
                                string = "(null)";
                        len = 0;
                        while (string[len] != 0 &&
                            (precision < 0 || len < precision))
                                ++len;
                        if (!left &&
                            put_padding(' ', width - len, sink) != 0)
                                return -1;
                        for (i = 0; i < len; ++i)
                                if (put_format_char(
                                    (unsigned char)string[i], sink) != 0)
                                        return -1;
                        if (left &&
                            put_padding(' ', width - len, sink) != 0)
                                return -1;
                        continue;
                }
                if (ch == 'f' || ch == 'F') {
                        fixed_value = va_arg(args, double);
                        if (precision != 0)
                                return -1;
                        if (__daimos_fround0_v33(fixed_value, &magnitude,
                            &negative) != 0)
                                return -1;
                        if (format_number(magnitude, negative, 10, 0,
                            left, zero, plus, space, 0, width, -1,
                            sink) != 0)
                                return -1;
                        continue;
                }

                negative = 0;
                base = 10;
                upper = 0;
                if (ch == 'd' || ch == 'i') {
                        signed_value = long_value ? va_arg(args, long) :
                            (long)va_arg(args, int);
                        negative = signed_value < 0;
                        magnitude = negative ? 0UL -
                            (unsigned long)signed_value :
                            (unsigned long)signed_value;
                } else if (ch == 'u' || ch == 'x' || ch == 'X' ||
                           ch == 'o' || ch == 'p') {
                        if (ch == 'p') {
                                magnitude = (unsigned long)va_arg(args, void *);
                                alternate = 1;
                                base = 16;
                        } else {
                                magnitude = long_value ?
                                    va_arg(args, unsigned long) :
                                    (unsigned long)va_arg(args, unsigned int);
                                if (ch == 'o')
                                        base = 8;
                                else if (ch == 'x' || ch == 'X')
                                        base = 16;
                                upper = ch == 'X';
                        }
                        plus = space = 0;
                } else {
                        return -1;
                }
                if (format_number(magnitude, negative, base, upper,
                    left, zero, plus, space, alternate, width,
                    precision, sink) != 0)
                        return -1;
        }
        return sink->count;
}

int
vprintf(char *format, va_list args)
{
        struct format_sink sink;

        sink.buffer = 0;
        sink.count = 0;
        return format_render(format, args, &sink);
}

int
vsprintf(char *buffer, char *format, va_list args)
{
        struct format_sink sink;
        int result;

        if (buffer == 0)
                return -1;
        sink.buffer = buffer;
        sink.count = 0;
        result = format_render(format, args, &sink);
        *sink.buffer = 0;
        return result;
}

int
printf(char *format, ...)
{
        int written;
        va_list args;

        va_start(args, format);
        written = vprintf(format, args);
        va_end(args);
        return written;
}
