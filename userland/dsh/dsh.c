#include "dsh.h"

static struct dsh_line dsh_line;

static void
dsh_line_clear(void)
{
        unsigned int i;

        dsh_line.len = 0U;
        for (i = 0U; i < DSH_LINE_MAX_WORDS; ++i)
                dsh_line.words[i] = 0;
}

static int
dsh_line_append(int ch)
{
        unsigned int wi;
        unsigned int sh;

        if (ch < 040 || ch > 0137 || dsh_line.len >= DSH_LINE_MAX_CHARS)
                return -1;
        wi = dsh_line.len / 6U;
        sh = 30U - (dsh_line.len % 6U) * 6U;
        dsh_line.words[wi] |= ((kword_t)(ch - 040) & 077UL) << sh;
        ++dsh_line.len;
        return 0;
}

static int
dsh_getline(void)
{
        int ch;

        dsh_line_clear();
        for (;;) {
                ch = dsys_readchar(0);
                if (ch == -2)
                        continue;
                if (ch < 0)
                        return -1;
                if (ch == '\r' || ch == '\n') {
                        (void)u_crlf(1);
                        return 0;
                }
                if (ch == 010 || ch == 0177) {
                        if (dsh_line.len != 0U) {
                                --dsh_line.len;
                                (void)u_puts(1, "\b \b");
                        }
                        continue;
                }
                if (ch < 040 || ch > 0137)
                        continue;
                if (ch >= 'a' && ch <= 'z')
                        ch -= 'a' - 'A';
                if (dsh_line_append(ch) == 0)
                        (void)u_putc(1, ch);
        }
}

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct dsh_state state;
        int interactive;
        int rc;

        dsh_state_init(&state, argc, argv, envp);
        interactive = dsys_isatty(0) > 0;
        if (interactive &&
            dsys_procctl(SYS_PROCCTL_TTY_SETMODE, SYS_TTY_MODE_RAW) !=
            (int)SYS_TTY_MODE_RAW)
                return 1;
        if (interactive) {
                (void)u_puts(1, "DSH V2");
                (void)u_crlf(1);
        }
        while (!state.exit_requested) {
                if (interactive && u_puts(1, "# ") != 0)
                        break;
                if (dsh_getline() != 0)
                        break;
                rc = dsh_execute_line(&state, &dsh_line);
                state.status = (unsigned int)(rc < 0 ? DSH_ERROR : rc);
        }
        if (interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_COOKED);
        return state.exit_requested ? (int)state.exit_status :
            (int)state.status;
}
