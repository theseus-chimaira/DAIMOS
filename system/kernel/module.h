#ifndef DAIMON_MODULE_H
#define DAIMON_MODULE_H

#include "kinit.h"

/* Two 18-bit MINIT entry addresses are packed into each table word. */
extern kword_t __minit_table_begin;
extern kword_t __minit_table_end;

void module_run_minits(void);

#endif
