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
int u_text_gets6(struct u_text_reader *r, kword_t *buf, unsigned int words);
void u_text_close(struct u_text_reader *r);

/*
 * Small allocation-free configuration helpers.  Both modify LINE in place
 * and return pointers into it.  Applications retain ownership of field
 * meaning and validation.
 *
 * u_text_fields(): colon-delimited records; empty fields are significant.
 * u_text_key():    KEY = VALUE VALUE ... records; an empty RHS is valid.
 *
 * Return the number of fields/values, 0 for blank/comment lines, or -1 for a
 * malformed record or insufficient caller pointer space.
 */
int u_text_fields(char *line, char **field, unsigned int max_fields);
int u_text_key(char *line, char **key, char **value, unsigned int max_values);

#endif
