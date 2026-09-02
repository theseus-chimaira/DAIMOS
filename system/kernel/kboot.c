#include "kboot.h"

/* Permanent bootstrap/runtime state.  One-shot boot orchestration lives in
 * KINIT so it disappears after entering the first user process. */
kword_t kcore_resident_end;
kword_t kcore_cty_putchar;
kword_t kcore_cty_getchar;
