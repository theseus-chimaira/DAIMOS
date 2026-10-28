# KINIT explicit disk-selector failure watermark v648

Testkit `daimos-kinit-disk-selector-failure-v648.sh` runs an
instrumented GCC 48K lowmem tape-root image with console switch
selector `1` (explicit disk root) rather than its configured tape
selector `2`. Native PDP-6 SIMH reaches the fatal HALT and records
`kinit_stack_highwater = 0164` octal (116 decimal words) against the
unchanged `0600` octal (384-word) reserve. Regression PASS.

This is additional selector coverage, not proof of a distinct D6FS
mount or allocator failure path. The ordinary lowmem tape-root build
does not install the full-profile D6FS root mount path. The broader
KINIT error-path stack watermark checklist remains open. No production
code or resident memory changed.
