#ifndef DAIMOS_USER_LOGEVENT_H
#define DAIMOS_USER_LOGEVENT_H

#include "dsys.h"

#define ULOG_SEV_DEBUG    0U
#define ULOG_SEV_INFO     1U
#define ULOG_SEV_NOTICE   2U
#define ULOG_SEV_WARNING  3U
#define ULOG_SEV_ERROR    4U
#define ULOG_SEV_CRITICAL 5U

#define ULOG_SRC_INIT     1U
#define ULOG_SRC_LOGIN    2U

#define ULOG_INIT_READY        1UL
#define ULOG_INIT_RUN_FAIL     2UL
#define ULOG_INIT_RESPAWN_FAIL 3UL

#define ULOG_LOGIN_FAIL        1UL
#define ULOG_LOGIN_OK          2UL
#define ULOG_LOGIN_SESSION_FAIL 3UL
#define ULOG_LOGIN_EXEC_FAIL   4UL

int ulog_event(unsigned int severity, unsigned int source,
    kword_t code, kword_t value);

#endif
