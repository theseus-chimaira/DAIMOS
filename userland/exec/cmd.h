#ifndef DAIMOS_CMD_H
#define DAIMOS_CMD_H

#include "u.h"

#define CMD_PROGRAM_ECHO       1
#define CMD_PROGRAM_CAT        2
#define CMD_PROGRAM_LS         3
#define CMD_PROGRAM_MKDIR      4
#define CMD_PROGRAM_RM         5
#define CMD_PROGRAM_PWD        6
#define CMD_PROGRAM_STAT       7
#define CMD_PROGRAM_TOUCH      8
#define CMD_PROGRAM_DATE       9
#define CMD_PROGRAM_RMDIR     10
#define CMD_PROGRAM_CP        11
#define CMD_PROGRAM_CHMOD     12
#define CMD_PROGRAM_CHOWN     13
#define CMD_PROGRAM_MKFS_DTFS 14
#define CMD_PROGRAM_FSCK_DTFS 15
#define CMD_PROGRAM_MOUNT     16
#define CMD_PROGRAM_MOUNT_DTFS 17
#define CMD_PROGRAM_UNMOUNT   18
#define CMD_PROGRAM_MV        19
#define CMD_PROGRAM_HEXDUMP   20
#define CMD_PROGRAM_PS        21
#define CMD_PROGRAM_DEVS      22
#define CMD_PROGRAM_MODS      23
#define CMD_PROGRAM_MOUNTS    24
#define CMD_PROGRAM_FREE      25
#define CMD_PROGRAM_MEMSTAT   26
#define CMD_PROGRAM_SYSCTL    27
#define CMD_PROGRAM_DF        28
#define CMD_PROGRAM_TTYOUT    29
#define CMD_PROGRAM_HALT      30

#define CMD_ENTRY_JOIN1(a, b) a##b
#define CMD_ENTRY_JOIN(a, b) CMD_ENTRY_JOIN1(a, b)
#define CMD_PROGRAM_ENTRY(token) CMD_ENTRY_JOIN(cmd_program_, token)

#ifndef DAIMOS_CMD_TOKEN
#define DAIMOS_CMD_TOKEN CHECK
#endif

int CMD_PROGRAM_ENTRY(DAIMOS_CMD_TOKEN)(int argc, kword_t **argv,
    struct u_io *io);

#endif
