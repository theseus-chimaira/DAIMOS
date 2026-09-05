# D6FS mutation split investigation TODO v1

Status: deferred investigation only. Do not implement until the resident-memory,
correctness, and overlay-lifetime tradeoffs have been measured.

## Goal

Determine whether D6FS can reduce permanent resident memory by separating the
always-needed read/core path from code used only to mutate filesystem state.
This is related to a kernel-overlay design, but this TODO intentionally does not
choose an overlay mechanism.

## Candidate mutation-side code

Measure the code and dependencies associated with:

- allocation and free-map mutation;
- extent growth, shrink, and rollback;
- block zeroing required by allocation/growth;
- FCB updates and writeback;
- create, mkdir, and symlink creation;
- unlink and rename;
- truncate and chmod;
- write paths that allocate or extend files;
- recovery/error paths used only by the above operations.

Do not assume that `write_words` belongs wholly on either side. Split its call
graph first and identify the read/write-within-existing-extents path separately
from allocation/growth if that distinction produces a real resident saving.

## Candidate always-resident core

Start by measuring the minimum call graph needed for:

- lookup and path traversal;
- readdir;
- stat;
- parent and parent-name operations;
- read_words and block mapping;
- runtime FCB validation/decoding required by reads;
- the runtime block cache;
- mounted-filesystem state needed for ordinary access;
- the minimum sync/unmount support that must always be callable safely.

This list is a measurement starting point, not a required design boundary.

## Questions that must be answered before implementation

1. What is the exact linked-word saving after including all dispatch, loader,
   relocation, state, and error-handling overhead introduced by the split?
2. What owns the mutation code/storage and what are its precise lifetime rules?
3. Can a mutation path ever be entered recursively or while another filesystem
   mutation is active?
4. Can PI/interrupt activity observe or call code/data while the overlay is
   changing, and what exclusion is required?
5. What happens with more than one writable D6FS mount?
6. Can read-side operations execute safely while mutation code is active?
7. Which routines/data are genuinely shared by read and mutation paths, and
   would splitting them duplicate code or state?
8. How are allocation rollback, write failure, and crash-consistency guarantees
   preserved if loading or switching the mutation component fails?
9. Does writable-mount latency or repeated overlay switching become significant?
10. Is the memory required by the mechanism itself smaller than the code it
    displaces in all supported configurations?
11. Can the design remain one coherent optional/cold mutation component rather
    than degenerating into many small overlays and new dispatch machinery?
12. Does the approach interact safely with MRES installation/uninstallation and
    with any future kernel-overlay mechanism?

## Measurement work

- Produce a linker-symbol call-graph census for `d6fs_provider.c` and
  `d6fs_pdp10.s`, divided into read/core, mutation-only, and shared symbols.
- Measure words in each category from actual linked PDP-10 objects; do not infer
  savings from C source size.
- Identify every piece of writable global/static state used by mutation code and
  whether it is already shared with the resident core.
- Prototype the boundary only in a disposable measurement branch after the
  census is complete.
- Compare permanent resident words, peak resident words during mutation, code
  size, and mutation latency against the unsplit kernel.
- Run D6FS corruption/rollback/recovery tests as acceptance tests for any future
  prototype.

## Non-goals

- Do not add a general pager or swapping subsystem solely for D6FS.
- Do not split individual filesystem operations into many independently loaded
  fragments.
- Do not weaken FCB/extent validation to make the split easier.
- Do not remove writable D6FS functionality to meet the size target.

## Current rationale for deferral

The current D6FS MRES has a large mutation-side call graph, so the theoretical
saving is interesting. The drawbacks are also substantial: new lifetime and
reentrancy rules, failure modes, switching overhead, and possible duplication
of shared read/write machinery. The safe next step is therefore measurement and
call-graph classification, not implementation.
