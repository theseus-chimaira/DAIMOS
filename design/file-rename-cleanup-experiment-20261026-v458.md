# Proposal B v458: rejected RENAME cleanup duplication

Attempted to replace the shared FILE RENAME success/failure cleanup
with separate inline return sequences to remove the failure branch.
The full linked PDP-6 GCC lowmem image increased from the accepted
KCORE 8961 to 8963 words (`__kcore_low_end` 021461 -> 021463).

The experiment was reverted completely. There is no accepted kernel
change, semantic change or saving. It confirms that preserving the
single RENAME cleanup tail is better than duplicating the stack
unwind. Baseline KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555 remain from v452; strict 32K deferred.
