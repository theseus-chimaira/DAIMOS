# Proposal B v422: complete root-login matrix accepted

Status: full PDP-6 simulator test PASS on disk, drum and tape roots.

Executed existing daimos-testkit target
`MAKEFLAGS=COMPILER=gcc make test-daimos-root-login-lifecycle`
against committed v417 full-six-bit FILE descriptors. Testkit commit
`5d22c0c` permits up to 15 minutes per tape-root interactive response
while disk and drum retain their existing five-minute response limit.

Results from the actual three-root simulator matrix:

* DSK/D6FS: PASS.
* DRM/D6FS: PASS.
* DTC/TSFS: PASS, including formerly delayed `PWD`.

Each root covers DCS0 transport, invalid LOGIN rejection, ROOT login,
DSH launch, PWD and root home, EXIT/LOGIN respawn, subsequent ROOT
login and ECHO response. The final harness output was:

    root-login-tape: PASS (invalid login, ROOT, HOME, logout/respawn, relogin)
    root-login-lifecycle: PASS (DSK/D6FS, DRM/D6FS, DTC/TSFS)

The longer tape-root timeout resolved the test's previous incomplete
observation; this does not establish a CPU/runtime performance improvement.
No kernel change was required. The current accepted linked-image
measurement remains KCORE 8999, KCORE+MRES 15547,
PERMANENT_LAST 15594 (110 words saved relative to pre-v417).
Strict 32K acceptance and unrelated folding work remain deferred.
