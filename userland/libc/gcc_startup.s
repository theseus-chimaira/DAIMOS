/**
 * @file gcc_startup.s
 * @brief PDP-10 GCC C-main initialization ABI entry.
 *
 * GCC inserts a call to __main at entry to C main.  DAIMOS has no C++
 * constructors, and its process state is initialized before _start.  This
 * empty ABI hook is linked only with GCC-built userspace executables.
 */
        .text
        .globl __main
__main:
        popj 17,
