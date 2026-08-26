#include "exec_v1.h"
#include "file_v1.h"
#include "mach_user_v1.h"
#include "vfs_v1.h"

#define EXEC_V1_HALF_MASK 0777777UL
#define EXEC_V1_WORD_MASK 0777777777777UL
#define EXEC_V1_DXR_MAGIC \
    ((VFS_V1_SIX6('D','X','R',' ',' ',' ') >> 18) & EXEC_V1_HALF_MASK)

static unsigned int
exec_v1_lh(kword_t word)
{
        return (unsigned int)((word >> 18) & EXEC_V1_HALF_MASK);
}

static unsigned int
exec_v1_rh(kword_t word)
{
        return (unsigned int)(word & EXEC_V1_HALF_MASK);
}

static int
exec_v1_read_exact(unsigned int owner, int fd, kword_t *buf,
    unsigned int words)
{
        int n;

        n = file_v1_read_words(owner, fd, buf, words);
        return n == (int)words ? 0 : -1;
}

static int
exec_v1_relocate(kword_t *image, unsigned int image_words, kword_t base,
    unsigned int owner, int fd)
{
        unsigned int done;
        unsigned int i;
        unsigned int n;
        kword_t bits;

        done = 0U;
        while (done < image_words) {
                if (exec_v1_read_exact(owner, fd, &bits, 1U) != 0)
                        return -1;
                n = image_words - done;
                if (n > 36U)
                        n = 36U;
                for (i = 0U; i < n; ++i) {
                        if ((bits & ((kword_t)1UL << (35U - i))) == 0)
                                continue;
                        image[done + i] =
                            (image[done + i] & ~((kword_t)EXEC_V1_HALF_MASK)) |
                            ((image[done + i] + base) & EXEC_V1_HALF_MASK);
                }
                done += n;
        }
        return 0;
}

static void
exec_v1_patch_syscalls(kword_t *image, unsigned int image_words)
{
        unsigned int i;
        kword_t target;

        target = EXEC_V1_PDP10_JRST |
            ((kword_t)(unsigned long)mach_syscall_trampoline_v1 &
            EXEC_V1_HALF_MASK);
        target &= EXEC_V1_WORD_MASK;
        for (i = 0U; i < image_words; ++i) {
                if ((image[i] & EXEC_V1_WORD_MASK) == EXEC_V1_SYSCALL_MARKER)
                        image[i] = target;
        }
}

int
exec_v1_load_init(struct proc_v1 *p, unsigned int owner,
    const kword_t *path, kword_t base, kword_t limit)
{
        kword_t hdr[EXEC_V1_DXR_HDR_WORDS];
        kword_t *mem;
        kword_t process_words;
        unsigned int entry;
        unsigned int image_words;
        unsigned int bss_words;
        unsigned int i;
        int fd;
        int rc;

        if (p == 0 || path == 0 || base == 0 || base >= limit)
                return -1;
        fd = file_v1_open(owner, path, FILE_V1_O_READ);
        if (fd < 0)
                return -1;
        rc = exec_v1_read_exact(owner, fd, hdr, EXEC_V1_DXR_HDR_WORDS);
        if (rc != 0)
                goto out;
        if (exec_v1_lh(hdr[0]) != (unsigned int)EXEC_V1_DXR_MAGIC) {
                rc = -1;
                goto out;
        }
        entry = exec_v1_rh(hdr[0]);
        image_words = exec_v1_lh(hdr[1]);
        bss_words = exec_v1_rh(hdr[1]);
        if (image_words == 0U || image_words > EXEC_V1_DXR_MAX_IMAGE_WORDS ||
            bss_words > EXEC_V1_DXR_MAX_BSS_WORDS || entry >= image_words) {
                rc = -1;
                goto out;
        }
        process_words = (kword_t)image_words + (kword_t)bss_words +
            (kword_t)EXEC_V1_DXR_STACK_WORDS;
        if (process_words > EXEC_V1_HALF_MASK || base + process_words > limit) {
                rc = -1;
                goto out;
        }
        mem = (kword_t *)(unsigned long)base;
        if (exec_v1_read_exact(owner, fd, mem, image_words) != 0 ||
            exec_v1_relocate(mem, image_words, base, owner, fd) != 0) {
                rc = -1;
                goto out;
        }
        for (i = 0U; i < bss_words + EXEC_V1_DXR_STACK_WORDS; ++i)
                mem[image_words + i] = 0;
        exec_v1_patch_syscalls(mem, image_words);
        proc_v1_set_memory(p, base, process_words);
        /* PDP-10 PUSH/PUSHJ grow the pushdown list upward.  AC17 must
         * therefore begin immediately below the reserved stack, not at
         * its upper limit.  The first push then lands in the first stack
         * word and all stack-local syscall buffers remain inside the
         * process memory bounds. */
        proc_v1_set_entry(p, (kword_t)entry);
        proc_v1_set_state(p, PROC_V1_SRUN);
        rc = 0;
out:
        if (file_v1_close(owner, fd) != 0 && rc == 0)
                rc = -1;
        return rc;
}
