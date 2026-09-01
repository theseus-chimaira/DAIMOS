#include "kboot_v1.h"

/* Permanent bootstrap/runtime state.  One-shot boot orchestration lives in
 * KINIT so it disappears after entering the first user process. */
kword_t kcore_resident_end_v1;
kword_t kcore_cty_putchar_v1;
kword_t kcore_cty_getchar_v1;
