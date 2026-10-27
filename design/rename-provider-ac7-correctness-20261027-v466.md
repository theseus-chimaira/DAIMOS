# Proposal B v466: fix VFS RENAME provider AC7 after namespace XOR

Native mapped FILE/VFS RENAME integration with the **actual** extracted
`fs_provider_reg_call` gateway failed before entering the provider lock
wait. Instrumentation established both parent resolutions and permissions
completed, and the register gateway itself was entered, but the provider
was not called.

Root cause: the compact v452 RENAME namespace comparison used AC7 for
the XOR of the two vnodes. The subsequent tail call to
`fs_provider_reg_call` requires AC7 to contain the source provider ID.
The former provider extraction had been lost when the comparison was
consolidated, so the gateway rejected a valid RENAME request.

Fix: immediately after the namespace check, reload AC7 with
`ldb 7,[POINT 6,1,5]`. Provider ABI and namespace checks are now both
preserved. This mandatory correctness repair costs one linked code word.

The native testkit v466 regression links the real two-path mapping
helper, FILE RENAME, VFS RENAME and unmodified extracted FS_MRES
provider lock gateway. A test `proc_wait_event` callback forces a
lock wait, clobbers volatile registers and makes the provider ready.
PASS: callback/wakeup, provider opcode 012, live mapping lease and
balanced mapping release; pathname, permission, provider and pointer
error paths. Prior v455, v461 and v464 tests also PASS.

This is a deterministic synchronous mock of `proc_wait_event`, not
full scheduler context-switch validation. The full monitor-UUO entry
and genuine cross-process wait/resumption still require testing.

Full linked PDP-6 GCC lowmem: KCORE **8961 -> 8962**, KCORE+MRES
**15508 -> 15509**, PERMANENT_LAST **15555 -> 15556**.
Strict 32K acceptance remains deferred.
