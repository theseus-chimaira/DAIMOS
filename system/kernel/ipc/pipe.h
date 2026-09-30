#ifndef DAIMON_PIPE_H
#define DAIMON_PIPE_H

#include "file.h"

/*
 * DAIMOS IPC transports native machine words only.  A pipe/FIFO never
 * interprets text, bytes, records, or device tokens; those transformations
 * belong to userland formats or device adapters.  The small eight-word ring
 * is deliberately independent of any encoding (48 SIXBIT characters fit in
 * it when a text application chooses ordinary packed SIXBIT words).
 *
 * The object layout is private to pipe_pdp10.s:
 *   0 active-FIFO link
 *   1 backing FIFO vnode (zero for anonymous pipes)
 *   2 ring state
 *   3 reader,,writer references
 *   4 reader event
 *   5 writer event
 *   6..15 eight opaque 36-bit payload words
 */
#define PIPE_PROVIDER       7U
#define PIPE_KIND_STREAM    1U
#define PIPE_BUFFER_WORDS   8U
#define PIPE_BUF            PIPE_BUFFER_WORDS
#define PIPE_OBJECT_WORDS  14U

/** Create one anonymous word FIFO and return read-fd,,write-fd, or -1. */
kword_t pipe_create(void);
/** Open or join the active word FIFO associated with a filesystem FIFO vnode. */
vnode_t pipe_fifo_open(vnode_t fifo_node, kword_t node_meta);
/** Detach a filesystem FIFO vnode without changing references held by opens. */
void pipe_fifo_detach(vnode_t fifo_node);
/** Return nonzero if an active FIFO belongs to the supplied mount id. */
int pipe_fifo_mount_busy(unsigned int mount_id);
/** Read up to nwords opaque words; zero is EOF and -1 is an error. */
int pipe_read_words(vnode_t node, kword_t *buf, unsigned int nwords);
/** Write up to nwords opaque words; returns count or -1/EPIPE. */
int pipe_write_words(vnode_t node, const kword_t *buf, unsigned int nwords);
/** Add references represented by one packed file-descriptor node word. */
void pipe_add_ref(kword_t node_meta);
/** Add inherited pipe/FIFO references for a complete process file table. */
void pipe_add_refs(struct file *table);
/** Drop descriptor references and free an anonymous/inactive object at zero. */
int pipe_close_ref(vnode_t node, kword_t node_meta);

#endif
