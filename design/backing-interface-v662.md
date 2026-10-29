# BACKING ownership and runtime vocabulary v662

The resident transfer bridge remains one DEVICE-or-VOLUME I/O path.
`backstore_read/write` retain exact entry addresses alongside the
historic `blockset_direct_tail_read/write` aliases. Its common
internal gateway is named `backing_region_io`.

Process swap boot and MonitorFS SWAP statistics now both use
`backing_region_blocks()` to query the logical BACKING capacity.
`blockset_tail_blocks` remains an ABI-compatible alias.

No additional dispatch layer, persistent state, allocation bitmap,
media-format change, or KCORE optimization was introduced. Tests in
daimos-testkit v662 verify production call sites and ABI aliases.
Native multi-member fault injection remains to be completed.
