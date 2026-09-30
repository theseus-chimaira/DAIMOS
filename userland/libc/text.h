#ifndef DAIMOS_USER_TEXT_H
#define DAIMOS_USER_TEXT_H

#include "u.h"

#define U_TEXT_EOF (-1)
#define U_TEXT_ERROR (-2)

#define U_TEXT_READ_WORDS 16U

struct u_text_reader {
        int fd;
        unsigned int pos;
        unsigned int used;
        kword_t words[U_TEXT_READ_WORDS];
};

int u_text_open(struct u_text_reader *r, const char *path);
int u_text_open_fd(struct u_text_reader *r, int fd);
int u_text_getline(struct u_text_reader *r, char *buf, unsigned int size);
void u_text_close(struct u_text_reader *r);

#endif
