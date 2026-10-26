# Proposal B v452: masked-XOR RENAME namespace comparison

RENAME must reject distinct provider/mount namespaces. Compare the
exclusive-or of the two 36-bit vnode arguments against the mask
`0777700000000`. Any differing provider/mount bit produces failure;
kind and local index remain irrelevant. This replaces two separate
AND operations and a compare with XOR plus TDNE, saving two resident
instructions. No ABI, stack, provider, filesystem, or persistent data
change.

Full linked GCC PDP-6 lowmem from v450/v451: KCORE 8963 -> 8961,
KCORE+MRES 15510 -> 15508, PERMANENT_LAST 15557 -> 15555.

The namespace regression in daimos-testkit recognizes both prior
and new source forms, verifies 4096 provider/mount identities remain
collision-free, and checks XOR and masked-identity equality agree.
Native PDP-6 FILE, pathname walk and provider-lock tests PASS.
Full end-to-end RENAME transaction regression remains outstanding.
Strict 32K acceptance remains deferred.
