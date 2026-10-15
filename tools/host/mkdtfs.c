/* mkdtfs.c -- initialize a new, empty native writable DTFS DECtape image.
 *
 * Keep existing media immutable during image rebuilds.  The native directory
 * uses the exact format written by the PDP-6 MKFS.DTFS command.
 */
#include "tsfs-format.h"

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

    if (argc != 3 || argv[1][0] != '-' || argv[1][1] != 'o' ||
        argv[1][2] != '\0') {
        fprintf(stderr, "usage: mkdtfs -o new-native-dtfs-image\n");
        return 2;
    }
    path = argv[2];
    fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        perror(path);
        return 1;
    }
    for (i = 0; i < TSFS_BLOCK_COUNT * TSFS_BLOCK_WORDS; ++i) {
        unsigned int position = i - DTFS_DIRECTORY_BLOCK * TSFS_BLOCK_WORDS;

        word = 0;
        if (i == DTFS_DIRECTORY_BLOCK * TSFS_BLOCK_WORDS)
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
