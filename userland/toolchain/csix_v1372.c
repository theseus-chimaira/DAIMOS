/*
 * Native CSIX v1372: packed ASCII7 source to canonical C-SIX/S6REC.
 *
 * This uses the reversible source-storage rules documented in
 * pdp10-doc/KERNEL/SELFHOSTING.MD. The established host csix -e and
 * -d operations provide the reference implementation. Native KCC
 * produces five 7-bit characters per 36-bit file word; CSIX reads
 * that representation and writes editable native S6REC text.
 *
 * Do not use C-SIX as an assembler source without decoding it first:
 * resident DAS expects ordinary uppercase SIXBIT/S6REC assembly.
 *
 * C89, bounded storage, no additional full-file source buffer.
 */
#include "u.h"

#define CSIX_READ_WORDS 16U

static kword_t csix_read_buffer[CSIX_READ_WORDS];
static unsigned int csix_read_used;
static unsigned int csix_read_pos;
static unsigned int csix_slot;
static unsigned int csix_lineno;
static unsigned int csix_column;
static kword_t csix_current;
static int csix_infd;
static int csix_outfd;
static int csix_padded;

static int
csix_error(const char *reason)
{
        (void)u_puts(2, "?CSIX ");
        (void)u_puts(2, reason);
        (void)u_puts(2, " LINE ");
        (void)u_put_uint(2, (kword_t)csix_lineno);
        (void)u_crlf(2);
        return -1;
}

/* Read packed 7-bit data without treating a 36-bit word as a 9-bit
 * character record. The low spare bit is ignored, as by native KCPP.
 * Once a zero (last-word padding) appears, nonzero data is invalid.
 */
static int
csix_getchar(void)
{
        unsigned int shift;
        int rc;
        int ch;

        for (;;) {
                if (csix_slot >= 5U) {
                        if (csix_read_pos == csix_read_used) {
                                rc = dsys_read_words(csix_infd,
                                    csix_read_buffer, CSIX_READ_WORDS);
                                if (rc == 0)
                                        return -1;
                                if (rc < 0 || rc > (int)CSIX_READ_WORDS)
                                        return -2;
                                csix_read_used = (unsigned int)rc;
                                csix_read_pos = 0U;
                        }
                        csix_current = csix_read_buffer[csix_read_pos++];
                        csix_slot = 0U;
                }
                shift = 29U - csix_slot * 7U;
                ++csix_slot;
                ch = (int)((csix_current >> shift) & 0177UL);
                if (ch == 0) {
                        csix_padded = 1;
                        continue;
                }
                if (csix_padded)
                        return -2;
                return ch;
        }
}

/* Reversible C-SIX escaping is syntax-blind: all source characters,
 * including quotes, character literals and identifiers, use the same
 * rules. Never silently uppercase characters in semantic text.
 */
static int
csix_put_encoded(unsigned int ch)
{
        int escape;

        if (ch >= 'a' && ch <= 'z')
                return u_putc(csix_outfd, (int)(ch - 'a' + 'A')) == 0 ?
                    0 : -1;

        escape = 0;
        if (ch >= 'A' && ch <= 'Z')
                escape = (int)ch;
        else {
                switch (ch) {
                case '@': escape = '@'; break;
                case '`': escape = '\''; break;
                case '{': escape = '<'; break;
                case '|': escape = '!'; break;
                case '}': escape = '>'; break;
                case '~': escape = '-'; break;
                default: break;
                }
        }
        if (escape != 0) {
                if (u_putc(csix_outfd, '@') != 0 ||
                    u_putc(csix_outfd, escape) != 0)
                        return -1;
                return 0;
        }
        if (ch < 040U || ch > 0137U)
                return -1;
        return u_putc(csix_outfd, (int)ch) == 0 ? 0 : -1;
}

static int
csix_encode(void)
{
        int ch;
        int next;
        int at_line_start;
        unsigned int nspaces;

        at_line_start = 1;
        csix_column = 0U;
        for (;;) {
                ch = csix_getchar();
                if (ch == -2)
                        return csix_error("INPUT");
                if (ch == -1)
                        break;
                if (ch == '\r') {
                        next = csix_getchar();
                        if (next != '\n')
                                return csix_error("BARE CR");
                        ch = '\n';
                }
                if (ch == '\n') {
                        if (u_putc(csix_outfd, '\n') != 0)
                                return csix_error("OUTPUT RECORD");
                        at_line_start = 1;
                        csix_column = 0U;
                        ++csix_lineno;
                        continue;
                }
                if (ch == '\t') {
                        nspaces = 8U - (csix_column & 7U);
                        while (nspaces-- != 0U) {
                                if (csix_put_encoded(' ') != 0)
                                        return csix_error("OUTPUT RECORD");
                                ++csix_column;
                        }
                } else {
                        if (csix_put_encoded((unsigned int)ch) != 0)
                                return csix_error("CHARACTER OR OUTPUT");
                        ++csix_column;
                }
                at_line_start = 0;
        }
        if (!at_line_start)
                return csix_error("MISSING FINAL NEWLINE");
        return 0;
}

int
main(int argc, kword_t **argv)
{
        struct vfs_stat stat_input;
        struct vfs_stat stat_output;
        int result;

        if (argc != 3) {
                (void)u_puts(2, "CSIX INPUT.ASCII7 OUTPUT.S6REC");
                (void)u_crlf(2);
                return 2;
        }
        if (dsys_stat(argv[1], &stat_input) != 0 ||
            stat_input.type != VFS_TYPE_REG ||
            dsys_stat(argv[2], &stat_output) == 0)
                return csix_error("INPUT OR OUTPUT EXISTS");

        csix_infd = dsys_open(argv[1], SYS_O_RDONLY);
        if (csix_infd < 0)
                return csix_error("INPUT");
        csix_outfd = dsys_open(argv[2], SYS_O_WRONLY | SYS_O_CREAT);
        if (csix_outfd < 0) {
                (void)dsys_close(csix_infd);
                return csix_error("OUTPUT");
        }
        csix_read_used = 0U;
        csix_read_pos = 0U;
        csix_slot = 5U;
        csix_lineno = 1U;
        csix_padded = 0;

        result = u_text_sink_attach(csix_outfd);
        if (result == 0)
                result = csix_encode();
        if (u_text_sink_detach() != 0)
                result = -1;
        if (dsys_close(csix_infd) != 0)
                result = -1;
        if (dsys_close(csix_outfd) != 0)
                result = -1;
        if (result != 0)
                (void)dsys_unlink(argv[2]);
        return result == 0 ? 0 : 1;
}
