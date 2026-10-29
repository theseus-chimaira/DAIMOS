# LOG boot I/O uses REGION directly (v661)

The earlier v657 transition left `auxstore_boot_log_read/write`
wrappers between `logstore_boot_read/write` and the bounded region
interface. There were no other callers. The reclaimable LOG boot code
now selects `AUXSTORE_REGION_LOGSTORE` explicitly and invokes
`auxstore_boot_region_read/write` directly. The redundant wrappers
and declarations are removed, rather than adding another runtime
abstraction. The legacy BLOCKSET-backed root LOG fallback remains.

LOG owns the persistent record format, sequence, append and recovery
policy. REGION only checks the block range and routes physical I/O;
LOG is not a cache and does not share the BACKING allocation bitmap.
This change neither alters the AUXST1 descriptor nor changes the
permanent MRES LOG service ABI.

The testkit v661 source routing regression and v657 AUXSTORE region
validation pass. Native LOG recovery/fault-injection remains separate.
