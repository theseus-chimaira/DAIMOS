# KINIT stack-watermark exclusive endpoint v639

The v638 32K lowmem image ends at octal `077200` with an octal `0600`
stack reserve. The exclusive reserve endpoint is octal `0100000`, one
word beyond the installed 32K core. The optional watermark fill and
scan previously treated that endpoint as inclusive, risking an access
to nonexistent memory even though ordinary noninstrumented boots pass.

Both routines now subtract one after calculating the endpoint, keeping
their last access inside the reserved area. No reserve is reduced and
no permanent KCORE text is changed.

The 48K GCC lowmem tape-root image built with watermarking enabled
links at octal `077252`. Testkit `daimos-kinit-watermark-bound-v639`
passes a native PDP-6 SIMH boot, reporting `MEM 48 K` and `INIT V1`.
The extra instrumentation is tested at 48K; this does not constitute a
measured 32K failure-path stack high-water or kernel-only boot gate.
