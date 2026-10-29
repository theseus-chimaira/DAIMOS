# Native D6FS root INIT rejection v653

Native PDP-6 SIMH full-profile GCC disk-root KINIT rejects two invalid
`/SYSTEM/INIT` situations after normal D6FS initialization: an absent
directory entry (zero child reference), and a nonregular INIT FCB
(directory instead of regular-file type). Each emits `?RT`, HALTs,
and does not start INIT V1.

Testkit `daimos-kinit-nonregular-init-v653.sh` exercises both modes
(`missing` and `nonregular`): verify a CLEAN original D6FS diskset;
copy four disk members; locate SYSTEM and INIT from on-media metadata;
mutate only one directory entry or FCB in the disposable copies; and
verify that the original diskset remains CLEAN. No production code
changes. The tested image was the clean 96K GCC instrumented v649
build; these are root-validation tests, not repeated stack tests.
