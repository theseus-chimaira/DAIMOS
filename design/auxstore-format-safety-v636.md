# AUXSTORE host-image safety gate v636 (2026-10-28)

The `mkauxstore` host formatter retains the AUXST1 disk format and layout.
Formatting an existing image now requires `--confirm-destructive`.
`--dry-run` validates ranges and checks that block zero and the complete
future LOGSTORE/BACKSTORE/CACHESTORE tail are byte-for-byte empty without
writing. A nonempty target is rejected even with confirmation. The D6FS
partition is deliberately neither examined as a filesystem nor modified.
Format and inspect reject symbolic links and nonregular files. The
`--blank` creation option now exclusively creates a new file and never
truncates an existing image.

Testkit: `daimos-mkauxstore-safety-v636.sh` covers dry-run immutability,
missing confirmation, successful confirmed formatting, reformat rejection,
unknown data rejection, upper-bit descriptor corruption, symlink rejection,
exclusive creation, and D6FS data preservation. Existing v493/v494/v508
fixture format commands are updated to use explicit confirmation.

The host safety test passed. A fresh run of legacy native fixture v493
was blocked before formatting by `mkimage: KINIT image leaves less than
the configured stack reserve` in its 32K lowmem build. Its failing build
and media are retained by the updated fixture cleanup. This is not a
successful native AUXSTORE integration test; the KINIT image/stack issue
remains a separate release gate and must not be bypassed.

This is a host-image safeguard, NOT completion of the separate MKSWAP TODO.
An in-system provisioning interface still needs exact physical controller,
unit and region selection, non-destructive inspection, and confirmation
against actual device identity. The host formatter must not write raw
device nodes. There is no permanent KCORE or MRES change in v636.
