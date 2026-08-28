#ifndef DAIMON_SCHED_V1_H
#define DAIMON_SCHED_V1_H

#include "proc_v1.h"

#define SCHED_V1_NO_RUNNABLE  (-1)
#define SCHED_V1_ENTERED      1

typedef void (*sched_v1_enter_fn)(kword_t base, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);

int sched_v1_run_once(sched_v1_enter_fn enterfn);

#endif
