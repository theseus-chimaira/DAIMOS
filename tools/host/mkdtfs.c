/* mkdtfs.c -- initialize a new, empty native writable DTFS DECtape image.
 *
 * Keep existing media immutable during image rebuilds.  The native directory
 * uses the exact format written by the PDP-6 MKFS.DTFS command.
 */
#include "tsfs-format.h"
#include "../../system/kernel/fs/dtfs_identity_v1520.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DTFS_DIRECTORY_BLOCK 0144U
#define DTFS_DIRECTORY_MAGIC_WORD 0177U
#define DTFS_NATIVE_MAGIC UINT64_C(0446446632021)
#define DTFS_OWNER_RESERVED 036U
#define DTFS_OWNER_NATIVE_TAG 035U

/* Fail closed if the OS cannot supply the full 72-bit new-volume token.
 * Neither a deterministic clock nor a truncated rand() sequence can
 * safely distinguish independently formatted removable media. */
static int
new_volume_id(uint64_t words[DTFS_ID_USED_WORDS_V1520])
{
    unsigned char random_bytes[9];
    size_t count;
    ssize_t n;
    int random_fd;
    unsigned int i;
    uint64_t high;
    uint64_t low;

    random_fd = open("/dev/urandom", O_RDONLY);
    if (random_fd < 0)
        return -1;
    count = 0;
    while (count < sizeof(random_bytes)) {
        n = read(random_fd, random_bytes + count,
            sizeof(random_bytes) - count);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0) {
            close(random_fd);
            return -1;
        }
        count += (size_t)n;
    }
    if (close(random_fd) != 0)
        return -1;

    high = 0;
    for (i = 0U; i < 4U; ++i)
        high = (high << 8) | random_bytes[i];
    high = (high << 4) | (random_bytes[4] >> 4);
    low = random_bytes[4] & 15U;
    for (i = 5U; i < 9U; ++i)
        low = (low << 8) | random_bytes[i];
    /* Treat even extraordinarily unlikely all-zero output as failure. */
    if (high == 0 || low == 0)
        return -1;
    words[DTFS_ID_MAGIC_WORD_V1520] = DTFS_ID_MAGIC_V1520;
    words[DTFS_ID_HIGH_WORD_V1520] = high;
    words[DTFS_ID_LOW_WORD_V1520] = low;
    words[DTFS_ID_CHECK_WORD_V1520] =
        (~(DTFS_ID_MAGIC_V1520 ^ high ^ low)) &
        DTFS_ID_WORD_MASK_V1520;
    return 0;
}

static int
write_half(int fd, uint32_t half)
{
    unsigned char b[4];
    size_t written;

    b[0] = (unsigned char)half;
    b[1] = (unsigned char)(half >> 8);
    b[2] = (unsigned char)(half >> 16);
    b[3] = (unsigned char)(half >> 24);
    written = 0;
    while (written < sizeof(b)) {
        ssize_t n = write(fd, b + written, sizeof(b) - written);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return -1;
        written += (size_t)n;
    }
    return 0;
}

int
main(int argc, char **argv)
{
    int fd;
    unsigned int i;
    uint64_t word;
    const char *path;
    uint64_t volume_id[DTFS_ID_USED_WORDS_V1520];

    if (argc != 3 || argv[1][0] != '-' || argv[1][1] != 'o' ||
        argv[1][2] != '\0') {
        fprintf(stderr, "usage: mkdtfs -o new-native-dtfs-image\n");
        return 2;
    }
    path = argv[2];
    if (new_volume_id(volume_id) != 0) {
        fprintf(stderr, "mkdtfs: 72-bit volume identity unavailable\n");
        return 1;
    }
    fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        perror(path);
        return 1;
    }
    for (i = 0; i < TSFS_BLOCK_COUNT * TSFS_BLOCK_WORDS; ++i) {
        unsigned int position = i - DTFS_DIRECTORY_BLOCK * TSFS_BLOCK_WORDS;

        word = 0;
        if (i < DTFS_ID_USED_WORDS_V1520)
            word = volume_id[i];
        else if (i == DTFS_DIRECTORY_BLOCK * TSFS_BLOCK_WORDS)
            word = (uint64_t)DTFS_OWNER_RESERVED << 31;
        else if (position == 14U)
            word = (uint64_t)DTFS_OWNER_RESERVED << 21;
        else if (position == 82U)
            word = ((uint64_t)DTFS_OWNER_NATIVE_TAG << 11) |
                ((uint64_t)DTFS_OWNER_NATIVE_TAG << 6) |
                ((uint64_t)DTFS_OWNER_NATIVE_TAG << 1);
        else if (position == DTFS_DIRECTORY_MAGIC_WORD)
            word = DTFS_NATIVE_MAGIC;
        if (write_half(fd, (uint32_t)((word >> 18) & TSFS_HALF_MASK)) != 0 ||
            write_half(fd, (uint32_t)(word & TSFS_HALF_MASK)) != 0) {
            perror(path);
            close(fd);
            unlink(path); /* Only the newly created, incomplete file. */
            return 1;
        }
    }
    if (close(fd) != 0) {
        perror(path);
        unlink(path);
        return 1;
    }
    return 0;
}
