# D6 inline decoder v066 - native safety checkpoint

The separately staged v066 fixed-address decoder preserves the inline-token
streaming ABI from v065. It adds an early 18-bit one-past-end check for both
input window and output region, verifies output cursor >= original base,
returns success for zero remaining output, and rejects literal writes when
remaining output is zero. It uses no static data or stack, and contains no
relocation-dependent literal pool (installed at fixed 000060).

The daimos-testkit native PDP-6 tests pass: v066 fixed-address resumable
stream decoding, source/output boundary rejection, malformed match descriptors
and unchanged legacy VFS adapter with two-word provider refills. Tests include
zero lengths, wrong kinds, backward references before origin and invalid zero
run distance. Passing these tests is not proof of exhaustive malformed-input
coverage or aliasing safety.

Native standalone map image span: 001064..001214 (octal 0130 = 88 decimal
36-bit words). With the unchanged 56-word VFS adapter, combined text is 144
words, +48 compared with the original 96-word D6LZ36 pair. v065 used 74+56
=130 (+34). Guarding therefore costs 14 decoder words.

No complete KCORE or post-p10fold measurement, producer parity, Stage-1 switch,
or native boot acceptance was performed. This variant stays inactive until
zero net resident growth and the 8K KCORE link limit can be demonstrated.
