# Native PDP-6 packed allocation rollback v081

The daimos-testkit v081 regression extends the native v080 MM transaction
fixture with a deliberately rejected packed extent allocation. The true
v077 transaction must return failure without retaining any pin on the
resident PROCESS extent, publishing a packed registry entry, changing
storage mode or freeing the resident allocation. The same process is then
successfully evicted and restored using the true native inline codec.
All four combinations of dense/sparse descriptor layout and zero/mixed
writable contents pass on the PDP-6 simulator.

This verifies a key failure-and-retry path of the previously corrected v076
source-pin cleanup on native PDP-6. The fixture still provides mock MM
arenas, not the production allocator, and no scheduler integration or
complete boot is claimed. The 8K resident limit is deferred by request.
