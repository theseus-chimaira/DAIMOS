# KCORE P0-A v1543: physical DRM236-root fail-stop accepted

Date: 2026-11-13. Feature-completion acceptance only.

The existing v1538 KCORE selected-root service gate applies to D6FS
regardless of the physical controller used for its backing descriptor.
The normal KINIT `root_select.c` and `blockset_boot.c` already support
`KINIT_ROOT_DRM`, DRM236 direct ranges and multi-member BLOCKSET roots.
No new kernel status bits, device register inference, resident state,
drivers, syscall ABI or root-failure recovery mechanism are needed.

The v1543 testkit builds a complete KCC 256K PDP-6 image with
`ROOT=drum BOOT=drm`, producing four real D6FS Type-236 images
from the normal `mkdsk` / `mkd6fs` recipes. SIMH switch octal 3
selects the DRM root. After normal PID1 reads succeed, the host
DETACHes **DR0..DR3** through the physical simulator monitor and
resumes the still-running guest. A fresh read of `/SYSTEM/INIT`
must require root data; the machine HALTs at **octal `003743`**,
the root-specific instruction at current v1515 entry `003740+3`.
The distinct BACKSTORE HALT is `003747`. The test rejects userland
error returns and all other HALT PCs.

The earlier v1540 test with the same PID1 validates selected
`ROOT=disk BOOT=dsk` and physical DSK0..DSK3 detachment. Both
disk and drum root hardware configurations PASS; no kernel code
was changed or duplicated for v1543. Detailed test instructions
are in daimos-testkit
`tests/validation/daimos-drm-root-outage-v1543.md`.

Remaining P0-A: real noncritical secondary DSK270/DRM236 I/O
failure while the selected root remains healthy, and mixed
root/BACKSTORE shared DEVICE/VOLUME failure qualification. A
device detached while unused is not evidence of failed secondary
mounted I/O, and the source-linked native tests cannot alone
replace those physical acceptance cases. Strict 32K/optimization
gate HOLD; VERSION 0.5.
