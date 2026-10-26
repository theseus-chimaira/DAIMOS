# Proposal B v432: remove duplicate MEMFS missing-service test

Accepted resident KCORE reduction. The MEMFS direct-dispatch path
already loads the installed service address, which defaults to
`fs_mres_no_service`. That trampoline returns -1 through the normal
call ABI, so testing for it and jumping there explicitly is redundant.
Dispatching the loaded target unconditionally removes two KCORE words
and does not affect the DTFS/D6FS/TSFS serialized provider path.

Full post-linked PDP-6 GCC lowmem measurement:

                       v431     v432     saved
KCORE                  8992     8990         2
KCORE+MRES            15539    15537         2
PERMANENT_LAST       15586    15584         2

Native daimos-testkit provider lock/dispatch v327, static contract
v325 and shared-workspace interleaving v428 PASS. Extended native
v327 to exercise an **unbound MEMFS service**, verifying -1 return,
no wait and an unchanged free provider lock. No allocations or API
changes. Strict 32K acceptance remains deferred.
