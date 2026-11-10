/**
 * @file d6block_v1452.h
 * @brief PDP-6 kernel interface to the resident block LZ/RLE stream decoder.
 *
 * block LZ/RLE is the compact 36-bit word-stream format shared by compressed boot
 * images, compressed executable images, and TSFS data extents.  The PDP-6
 * kernel exposes only the VFS streaming frontend here; token decoding itself
 * is implemented by the fixed low-core decoder shared with Stage1.
 *
 * The resident decoder is split between the fixed low-core resumable token
 * engine and a VFS frontend.  It allocates no permanent
 * decode buffer; the frontend uses only a small temporary stack refill window
 * while the already-produced destination words form the LZ history.
 *
 * Format details belong to the decoder implementation; kernel C callers need
 * only the packed destination ABI documented on d6codec_decode_vfs_v1452().
 */
#ifndef DAIMON_D6BLOCK_H
#define DAIMON_D6BLOCK_H

#include "kcore.h"

/**
 * @brief Stream a block LZ/RLE payload from VFS directly into PDP-6 memory.
 * @param node VFS vnode containing the compressed payload.
 * @param file_offset Word offset of the first compressed block LZ/RLE word.
 * @param src_words Exact compressed payload length in 36-bit words, or zero
 *        only for already-validated immutable backing where decode may stop
 *        as soon as the requested output image is complete.
 * @param dst_words_addr Packed output descriptor: bits 18..35 are the exact
 *        output word count and bits 0..17 are the destination word address.
 * @return 0 when exactly @p src_words are consumed and exactly the requested
 *         output word count is produced; -1 on VFS failure, malformed input,
 *         premature input exhaustion, trailing compressed words, or invalid
 *         destination geometry.
 *
 * The PDP-6 frontend refills a small stack-resident source window and
 * resumes the fixed low-core token engine across VFS reads.  No heap or
 * persistent per-decode storage is allocated.  The destination must remain
 * valid for the entire call because previously produced words are also the
 * match history.
 */
int d6codec_decode_vfs_v1452(kword_t node, kword_t file_offset,
    unsigned int src_words, kword_t dst_words_addr);

#endif
