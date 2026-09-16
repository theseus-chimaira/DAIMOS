#ifndef DAIMOS_USER_TEXT_H
#define DAIMOS_USER_TEXT_H

#include "u.h"

#define U_TEXT_EOF (-1)
#define U_TEXT_ERROR (-2)

struct u_text_reader {
        int fd;
        kword_t chars_left;
        kword_t word;
        unsigned int pos;
};

int u_text_open(struct u_text_reader *r, const char *path);
int u_text_getline(struct u_text_reader *r, char *buf, unsigned int size);
void u_text_close(struct u_text_reader *r);

#endif
