# BSTORE shared raw AUXSTORE region I/O, first feature increment v657

This is a **feature implementation**, not a KCORE size optimization.
The production AUXSTORE bootstrap now exports a region-relative
`auxstore_boot_region_read/write` interface for BACKSTORE, LOGSTORE,
and CACHESTORE. Requests are checked against the selected region's
advertised block count before translation to the hardware DSK/DRM
block. Missing media, null buffers, unknown regions and an offset at
or beyond the partition endpoint are rejected. The existing LOGSTORE
bootstrap read/write wrappers use this shared path.

The production BACKSTORE allocator, direct I/O, allocation bitmap,
recovery and removable-MRES contracts remain unchanged in this
increment. Their conversion/verification is subsequent feature work.
No linked-image size savings or completed BSTORE cutover are claimed.

Testkit `daimos-auxstore-region-v657.sh` compiles the production
translator and wrapper functions into a C99 host harness, covering
valid region translations, boundaries, missing media, invalid region
and null-buffer rejection. PASS. Existing AUXSTORE choice v492 and
D6FS overlap v507 tests also PASS. Native hardware/boot regression
is still required for the complete migration.
