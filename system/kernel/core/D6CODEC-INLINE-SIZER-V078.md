# Native inline size-only encoder and KPACK callbacks v078

The staged v078 PDP-6 encoder extends the native v071 inline stream writer
with count-only mode (AC3=0). It retains a virtual output cursor, performs
the same token-selection passes and capacity accounting, but suppresses
all compressed output writes. It uses no temporary compressed buffer and
no permanent codec workspace. The regular AC3!=0 encoding path emits
identical inline tokens. The v078 KPACK adapter exports three C-callable
assembly operations: measure input words, encode with exact caller capacity,
and decode an exact compressed payload into expected output words.
AC10..AC16 are preserved across all three operations.

Native PDP-6 testkit v078 performs measure->encode->decode roundtrips for
mixed literal/zero/match data and 128 consecutive zeros (expected packed
length 3), and checks preserved registers. This is a component-level
validation; an actual KCORE link and scheduler MM transaction are still
needed. In particular the existing v077 C transaction is staged and must
be linked with these callbacks under lifecycle exclusion before activation.

The 8K resident endpoint remains a mandatory acceptance condition.
Decoder and encoder size optimization is deferred by explicit request.
