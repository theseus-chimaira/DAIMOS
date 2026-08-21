#include "kinit.h"

#ifndef DAIMON_VERSION_MAJOR
#error DAIMON_VERSION_MAJOR must come from VERSION
#endif
#ifndef DAIMON_VERSION_MINOR
#error DAIMON_VERSION_MINOR must come from VERSION
#endif
#if DAIMON_VERSION_MAJOR > 9 || DAIMON_VERSION_MINOR > 9
#error KINIT V0.1 banner format supports one decimal digit per component
#endif

void
kinit_diag_banner(void)
{
        kword_t version;

        version = KINIT_S6_W6(' ', 'V',
            '0' + DAIMON_VERSION_MAJOR, '.',
            '0' + DAIMON_VERSION_MINOR, ' ');
        kinit_poll_put6(KINIT_S6_W6('D','A','I','M','O','N'));
        kinit_poll_put6(version);
        kinit_poll_put6(KINIT_S6_W6(' ',' ',' ',' ',' ',' '));
}

void
kinit_diag_failure_poll(void)
{
        kinit_poll_put6(KINIT_S6_W6('?','K','I','N','I','T'));
}

void
kinit_diag_finished(void)
{
        kinit_poll_put6(KINIT_S6_W6('K','I','N','I','T',' '));
        kinit_poll_put6(KINIT_S6_W6('F','I','N','I','S','H'));
        kinit_poll_put6(KINIT_S6_W6('E','D',' ',' ',' ',' '));
}
