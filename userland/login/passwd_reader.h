/* Account-file lookup only: password verification remains in LOGIN. */
#ifndef DAIMOS_LOGIN_PASSWD_READER_H
#define DAIMOS_LOGIN_PASSWD_READER_H
#include "u.h"
#define LOGIN_PASS_MAX 63U
struct login_account {
        unsigned int uid;
        unsigned int gid;
        char password[LOGIN_PASS_MAX + 1U];
        kword_t home[U_PATH_WORDS];
        kword_t shell[U_PATH_WORDS];
};
int login_find_account(const char *name, struct login_account *account);
#endif
