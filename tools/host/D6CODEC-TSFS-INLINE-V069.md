# TSFS inline-token image tools v069

Staged TSFS format 1.3 host tools use the v068 inline codec and reserve
extent flag 2 for compressed inline-token data. The old on-media D6LZ36
format (TSFS 1.1) and the v061 grouped prototype (1.2) are not accepted
by the new checker. Both tools are excluded from the production TOOLS list.

Testkit `tests/host/d6codec-tsfs-v069/run-v069.sh` validates an inline
compressed extent and checks bidirectional rejection with TSFS 1.1 media.
Native TSFS runtime decode, boot acceptance, and full 8K KCORE linking are
still pending. Resident optimization remains deferred, but the 8K
requirement remains mandatory before production activation.
