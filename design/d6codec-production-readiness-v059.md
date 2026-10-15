# D6 codec production readiness v059

Status: staged native decoder correction only. No production cutover.

The active system uses the D6LZ36 resumable decoder installed at 000060.
The v053 grouped-token codec decoder was complete-buffer-only and rejected
odd kind-2 halfword match distances, despite the broader format proposal.
It must not replace the live decoder or be used to produce boot images yet.

## First staged correction

`system/kernel/core/d6codec_decode_v059.s` adds an odd-distance halfword
copy path to the staged v053 decoder. It writes the high output half first
and then reads the second source half, preserving overlapping distance-one
semantics. No permanent data, heap buffer or scratch BSS is introduced.
Native DAS compilation: PASS; 115 text words, zero data and BSS. This
is not a native functional test or evidence of safe decoding.

## Native regression checkpoint v060 (2026-10-15)

`daimos-testkit/tests/validation/d6codec-native-v060/run-v060.sh` now
tests odd-distance halfword copies of distances one (overlapping) and
three (across two literal source words). Both pass on the PDP-6 simulator.
The runner also rebuilds and executes all 21 historical v048 decoding
fixtures against the v059 entry point, including malformed descriptor,
overrun, invalid kind, truncation, empty stream and even-halfword tests:
23/23 PASS. The existing v055 kernel ABI/golden-corpus regression also
passes (using the older v053 decoder).

The initial distance-one test failed due to an incorrectly emitted
decimal assembler descriptor; adding a leading octal zero fixed the
fixture without changing the decoder. No v059 implementation failure
was established by this test. Native v060 tests are **not** an end-to-end
streaming, full-boundary or kernel-ABI acceptance test; remaining checks
below still gate production use.

## Mandatory validation before replacing anything

## Native VFS refill adapter v062 (2026-10-15)

### v074 host halfword-match emission

The staged v072 host encoder now searches odd 18-bit halfword distances
up to 511 halfwords and emits kind-2 descriptors where they exceed an
ordinary word match. The decoder's overlapping high-half/low-half
semantics are preserved. `daimos-testkit/tests/validation/d6codec-host-native-v074`
generates a 350-word mixed workload with a constructed odd-distance
match, compresses it on the host and validates every output word using
the native PDP-6 v061 decoder: PASS. Inspection of the generated 196-word
stream confirms one kind-2 descriptor, plus zero-run and word matches.
The production D6LZ36 encoder/decoder remain unchanged. Further
adversarial, capacity and real-provider tests are required before cutover;
the final regression sweep excludes all 32K configurations.

### v073 host encoder to native decoder differential test

`daimos-testkit/tests/validation/d6codec-host-native-v073` generates a
350-word deterministic mixed payload in host C99, compresses it using the
staged v072 host encoder, emits exact octal 36-bit words for the PDP-6
assembler, and checks that the native v061 decoder reconstructs every
source word. PASS on PDP-6. Existing v071 host decoder and v072 host
encoder round-trip tests also pass. The source is generated at test time;
no golden compressed stream is hardcoded. The staged VFS adapter,
registered real providers, halfword-match encoder emission and production
format selection remain outside this test. After full integration the
user-requested full test run excludes all 32K configurations.

### v072 bounded staged host encoder

`tools/host/d6codec-group-v072.c` adds a caller-buffered C99 encoder for
the grouped 36-bit stream format, with zero-run and overlapping word-match
descriptors and literal fallback. It uses no heap or temporary arrays and
does not alter the active D6LZ36 producer. The testkit v072 suite passes
a 350-word mixed-input round trip and a sequence of short-input cases
against the independent v071 decoder. The six v071 decoder cases also pass.
Halfword-match emission, complete native compressed-stream parity, and
production host/userspace producer cutover remain open. On eventual
integration run all applicable tests except 32K configurations.

### v071 staged C99 host reference decoder

Added `tools/host/d6codec-group-v071.c` and `.h` as an independent
bounded, allocation-free reference decoder for the grouped format. It
supports literals, zero runs, ordinary overlapping word matches, aligned
halfword matches, and odd-distance halfword matches, rejecting invalid
descriptors, truncation, and trailing groups. The staged decoder has no
effect on `d6lz-codec.c` or production host/userland format selection.
The host testkit's `tests/host/d6codec-group-v071/run-v071.sh` passes six
initial valid/invalid fixtures under strict C99 compilation. Additional
golden-corpus parity, randomized adversarial coverage, native differential
testing and both host/userland *encoder* migrations remain mandatory.

After integration run the full applicable regression matrix, excluding
all 32K configurations. Production D6LZ36 remains unchanged.

### v070 producer-to-refill decoder regression

`daimos-testkit/tests/validation/d6codec-vfs-v070` now uses the native
v053 encoder to compress 90 distinct words, reads the encoded stream in
seven-word fragments through a vnode-style read callback, and validates
all output words from the v061/v062 grouped decoder. PDP-6 native PASS.
The callback is not a registered production VFS provider; real-provider
integration and nested stack measurement remain open.

After atomic integration of the new format and all producers/consumers,
run the complete applicable DAIMOS and daimos-testkit test suites, including
boot, FS, EXEC, swap, compression, malformed data, and regression coverage,
**excluding 32K configurations as requested**. Do not present the current
staged probe or the 32K development smoke results as post-integration
acceptance tests.

### v069 linked-codec boot smoke

Built a fresh GCC 32K disk-root image with `D6CODEC_LINK_PROBE_V068=1`,
then ran PDP-6 SIMH through the D6FS load and `INIT V1` milestones.
The command was stopped by its 25-second timeout after those milestones;
its exit status 124 does not mean the boot failed. Boot log includes
`D6FS LOADED` and `INIT V1`, without an observed simulator HALT.
This confirms the *inactive* linked codec objects do not prevent this
legacy-format disk-root boot. It does not exercise grouped decompression
through actual VFS providers, nor prove their deepest kernel-stack use.
The runtime cutover remains blocked pending a test-only invocation with
new-format producer data and a full provider-stack measurement.

### v068 opt-in full KCORE link probe

The boot Makefile now accepts `D6CODEC_LINK_PROBE_V068=1` to place the
staged v061 group decoder and v062/v063 VFS adapter into the complete
GCC 32K disk-root KCORE link **before** `kcore_end`. The default is off;
the old `000060` decoder and all D6LZ36 call sites are unchanged.
The probe image built and packaged successfully with the added object
symbols in `kcore.map`. The size audit reports the exclusive KCORE end
at `024024` octal, compared with the earlier v141 inactive baseline
`023451` octal (235 decimal words difference). This is a provisional
link-size comparison across checkpoints, not a measured codec replacement
saving. No native execution with actual registered filesystem providers
has been performed; their nested stack and cache-miss paths still gate
activation.

### v067 staged VFS adapter register preservation

The native PDP-6 regression at
`daimos-testkit/tests/validation/d6codec-vfs-v067` checks AC10..AC16
after each of three adapter calls: successful short-read decoding,
premature EOF, and truncated compressed input. All paths preserve the
callee-saved registers (PASS). This uses a test-only VFS stub, not the
real D6FS, DTFS, or TSFS provider. No codec source changes were needed.

Real-provider integration remains blocked on a dedicated test-only
kernel link/call path and nested stack measurements under provider
serialization and cache-miss conditions. Legacy Stage-1 and production
D6LZ36 remain unchanged.

### v066 full-window group carry and provider-dispatch inspection

The native PDP-6 test in `daimos-testkit/tests/validation/d6codec-vfs-v066`
decodes three 30-literal groups (93 compressed, 90 decoded words), forcing
the 64-word VFS window to hold two complete groups and the beginning of
the third. The adapter shifts the unconsumed prefix into the window,
refills it, and verifies all 90 reconstructed words: PASS. No changes to
the staged decoder/adapter were necessary for this test.

`system/kernel/fs/vfs.s:vfs_read_words` dispatches to the pipe, MFS or
registered FS provider; `system/kernel/fs/fs_mres.s:fs_provider_reg_call`
serializes D6FS/DTFS/TSFS and may enter `proc_wait_event` while waiting.
The existing v064 synthetic stack measurement is therefore insufficient
to establish maximum cache-miss or provider-lock-wait depth. A real
provider-integrated stack canary run remains a production activation gate.

### v065 output geometry and real-provider dispatch audit

The actual VFS entry is `system/kernel/fs/vfs.s:vfs_read_words`, which
dispatches to pipe/MFS or through `fs_provider_reg_call`. Therefore a
one-word test stub cannot establish the maximum call depth of the real
D6FS/DTFS/TSFS provider or its cache-miss path. The process kernel stack
allowance is `0316` octal (206 decimal) words in
`system/kernel/proc/proc_pdp6.s`. The v064 simulated nested 113-word peak
leaves 93 words, **not** a measured guarantee of sufficient headroom.

New `daimos-testkit/tests/validation/d6codec-vfs-v065` passes on native
PDP-6: a null destination and an output extent crossing the 18-bit address
limit are rejected **before any provider call**. The real-provider
cache-miss, failure-path, and total nested stack measurements remain
mandatory before production activation. No KCORE path was changed.

### v064 native stack canary checkpoint

`daimos-testkit/tests/validation/d6codec-vfs-v064` uses a 512-word
canary-filled PDP-6 stack and the existing short-read, premature EOF and
truncation fixture. The result is 0127 octal (87 decimal) stack words
from the test stack origin. A VFS stub that explicitly reserves 030
octal (24 decimal) additional words and makes a nested call reaches 0161
octal (113 decimal) words. Both native tests pass. The difference also
includes the nested test call overhead; these measurements are not
real-provider worst-case stack bounds.

Before activation, instrument actual VFS/TSFS/D6FS nested call stacks,
including cache-miss and failure paths, and compare the peak against the
configured per-process kernel stack and its safety reserve. No active
KCORE code or decoder ABI changed in this checkpoint.

### v063 provider-count and address-bound audit

The inactive VFS adapter now rejects output spans extending beyond the
18-bit PDP-6 physical address range and checks every VFS return count
against the actual requested capacity before treating the refill as valid.
An overreporting VFS provider could previously raise `CV_USED` beyond the
64-word stack window; that is a correctness bug, not a permitted fast path.
`daimos-testkit/tests/validation/d6codec-vfs-v063` passes on PDP-6 using a
stub which returns 65 words for a one-word actual copy. The earlier v062
native short-read/truncation fixture continues to pass. One additional
transient stack word holds requested capacity across the read; no permanent
storage or additional decoder buffer is allocated.

The adapter remains inactive and unaccepted for real VFS integration.

`system/kernel/core/d6codec_vfs_v062.s` is a separate, inactive native
adapter for the v061 grouped decoder. It preserves AC10..AC16, holds a
64-word VFS refill window on the kernel stack, counts descriptors from
the control bitmap, and only invokes v061 once a complete group is present.
Unconsumed words are shifted down in the same window. VFS may return short
reads; the adapter tracks file offset, unread input, output cursor, and
exact completion. There is no permanent codec BSS or heap allocation.

Native DAS: 106 text words, 0 data/BSS (separate from v061's 122 words).
`daimos-testkit/tests/validation/d6codec-vfs-v062` passes on PDP-6 with
a one-word-per-read VFS stub, including an odd-halfword overlapping match
split across reads, excess advertised compressed length (premature EOF),
and insufficient declared input (truncated final group).

This test does NOT establish the worst-case stack high-water, real VFS
integration, fixed address 000060 ABI, all failure cases, bootloader
operation, or net linked KCORE footprint. The new adapter is not linked
into production and the legacy implementation remains untouched.

## Group-boundary streaming prototype v061 (2026-10-15)

`system/kernel/core/d6codec_decode_group_v061.s` implements a staged
group-boundary resumable decoder without a static buffer or persistent
workspace. Every refill must supply one or more **complete** groups.
AC1/AC2 identify the compressed window; AC3 is the retained output cursor;
AC4 is the absolute exclusive output end; AC5 is the fixed output origin.
AC1 returns 0 for exact completion, +1 when an entire group was consumed
and further input is required, and -1 on malformed input. This is a new
experimental ABI, **not** a replacement for fixed address 000060.

Native DAS object size: 122 text words, zero static data/BSS. Each group
requires at most 46 compressed words (control + 15 paired descriptors +
30 literals), so the existing 64-word VFS stack refill window can hold a
complete group with headroom for carry; the VFS adapter must implement
incomplete-group compaction and must not hand an incomplete group to this
engine. The new core does not allocate or copy its own window.

`daimos-testkit/tests/validation/d6codec-native-v061/run-v061.sh`:
3/3 native PDP-6 fixtures PASS (two-group resume with overlapping odd
halfword copy; reject truncated descriptor group; reject trailing input
after exact output completion). This is **not** full ABI, refill, stage-1,
address-wrap, or stress acceptance; production D6LZ36 remains unchanged.

1. Put native tests in daimos-testkit for distances 1, 3, maximum supported,
   overlap, destination/source boundaries, truncated and malformed groups,
   input/output alias rejection and exact consumption. Compare with an
   independent host reference model. Fix all discovered bugs at source.
2. Implement the grouped-stream decoder as a resumable fixed-000060 engine,
   retaining caller state and using the existing 64-word VFS refill window.
   Group size is at most 31 words; compact incomplete groups in the window.
3. Validate compiler calling conventions, saved ACs, 18-bit address bounds,
   encoder capacity and full worst-case stack high-water with native tests.
4. Update ALL five Stage-1 loaders, kernel VFS/EXEC/PURE/TSFS consumers,
   host image and filesystem producers, host validation tools and userspace
   tools in a single new-format-only migration. Rebuild all encoded images.
5. Run exact-format corpus tests, 32K GCC KCORE/KINIT size/boot tests,
   corruption/restart tests and packed-process failure injection; retain
   disk-swap fallback until its own acceptance is complete.

The detailed producer/consumer inventory and cross-subsystem acceptance
criteria remain in `system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md` and
`TODO.MD` (v057 plan). Do not enable a mixed-format runtime or claim KCORE
savings from codec replacement before actual linked measurements.
