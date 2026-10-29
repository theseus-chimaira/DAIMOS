# D6FS read-only native block inspection v676

The native syscall extension 053 supplies `dsys_d6fs_read_block(logical,
buffer)` for a future userspace D6FSCK. It reads exactly one 128-word block
through the existing root BLOCKSET mapping. The syscall requires root
credentials, a mounted D6FS namespace root, and the root mount policy RO.
The ordinary mapped-user-buffer hold and bounds check are reused; the new
path does not expose raw block writes or a new permanent data buffer.

This is a checker prerequisite, not recovery authorization. Filesystem repair
must use a separate exclusive interface coordinated with the VFS and cache;
the RW remount must independently revalidate the repaired media. The
production KINIT mount remains unchanged, and there is no native D6FSCK
implementation in this increment.

Validation: testkit `daimos-d6fs-rawread-v676.sh` source checks PASS;
PDP-6 assembler object build of the modified syscall dispatcher PASS.
Native syscall execution, 32K full boot, media fault injection, and complete
recovery acceptance remain outstanding.
