#include "d6fs.h"


/* Dirent decode/validation lives in d6fs_pdp10.s; FCB validation stays in C until the compact assembly version is revalidated. */

/* d6fs_reader_init is implemented in d6fs_pdp10.s. */

/* d6fs_reader_fcb is implemented in d6fs_pdp10.s. */

/* d6fs_reader_read_words is implemented in d6fs_pdp10.s. */

/* d6fs_reader_commit_cache is implemented in d6fs_pdp10.s. */

/* d6fs_reader_write_block is implemented in d6fs_pdp10.s. */

/* d6fs_reader_zero_block is implemented in d6fs_pdp10.s. */

/* d6fs_reader_write_words is implemented in d6fs_pdp10.s. */

/* d6fs_reader_put_fcb is implemented in d6fs_pdp10.s. */

/* Compact PDP-10 bitmap helpers implemented in d6fs_pdp10.s. */
extern int d6fs_freemap_state(struct d6fs_reader *, kword_t);

/* d6fs_free_run is implemented in d6fs_pdp10.s. */





/* d6fs_dirent_decode_valid is implemented in d6fs_pdp10.s. */

/* d6fs_file_block and d6fs_reader_read_words are implemented in d6fs_pdp10.s. */
