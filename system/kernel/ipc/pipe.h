#ifndef DAIMON_PIPE_H
#define DAIMON_PIPE_H

#include "file.h"

#define PIPE_PROVIDER       7U
#define PIPE_KIND_STREAM    1U
#define PIPE_KIND_WORD      2U
#define PIPE_BUFFER_WORDS  26U
#define PIPE_BUFFER_CHARS 128U
#define PIPE_BUFFER_NATIVE_WORDS PIPE_BUFFER_WORDS
#define PIPE_BUF          128U
#define PIPE_OBJECT_WORDS  32U

kword_t pipe_create(void);
kword_t pipe_create_words(void);
vnode_t pipe_fifo_open(vnode_t fifo_node, kword_t node_meta);
void pipe_fifo_detach(vnode_t fifo_node);
int pipe_fifo_mount_busy(unsigned int mount_id);
int pipe_readchar(vnode_t node);
int pipe_writechar(vnode_t node, unsigned int ch);
int pipe_read_words(vnode_t node, kword_t *buf, unsigned int nwords);
int pipe_write_words(vnode_t node, const kword_t *buf, unsigned int nwords);
void pipe_add_ref(kword_t node_meta);
void pipe_add_refs(struct file *table);
int pipe_close_ref(vnode_t node, kword_t node_meta);

#endif
