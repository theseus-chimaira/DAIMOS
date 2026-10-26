# Proposal B v447: compact CREATE/SYMLINK provider argument

Accepted one-word improvement to the v446 shared provider dispatch.
The provider request E argument is unused for SYMLINK (operation 010)
and must be zero there. A conditional instruction directly skips the
zeroing for other operations; the prior unconditional branch is removed.
The CREATE operation retains its existing E argument unchanged.

Full linked PDP-6 GCC lowmem: KCORE 8973 -> **8972**;
KCORE+MRES 15520 -> 15519; PERMANENT_LAST 15567 -> 15566.
No data allocation or extra process stack word is introduced.

Updated `daimos-testkit` ABI guard PASS, as did DTFS CREATE with
native/TENEX/ITS rollback, FILE/PIPE direct-mount, UMASK and native
provider-lock checks. This does not constitute a full native SYMLINK
end-to-end runtime regression; retain that outstanding item.

No other FILE/VFS ownership interface was changed. Strict 32K
acceptance remains deferred.
