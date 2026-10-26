# Proposal B v437 — OPEN scratch lifetime proof boundary

Status: lifetime test PASS; source audit only; KCORE unchanged.

The resident `file_open` entry saves AC10–AC13 and reserves an
additional 016 (octal, 14 decimal) words. The node pointer lives
throughout lookup/create, stat and descriptor initialization. The
create-only phase uses parent vnode plus a five-word name. On both
existing and created file paths `vfs_stat` then writes a seven-word
result; its type/size/ownership fields are consumed during access
checking, truncate, FIFO opening and append-offset publication.

The caller `exec_native_syscall:%L67` holds the user VM mapping before
invoking FILE OPEN and releases it on normal return. FIFO failure
clears both descriptor words before returning. Combining these phases
must keep every live value valid across potentially sleeping provider
calls. A frame overlap cannot be inferred solely from separate source
branches: the node and stat remain live at descriptor publication.

`daimos-testkit/tests/validation/daimos-open-lifetime-v437.py` PASS
guards the current core boundaries. It is static validation, not a
concurrent OPEN workload. No production change: linked KCORE remains
8989 words (accepted v436 baseline). A future replacement must remove
original syscall/FILE branches rather than adding a second veneer;
measure the post-fold linked image and run all OPEN, FIFO, mapping and
process lifecycle tests before accepting any size saving.
