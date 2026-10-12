# DSH no-overlay compiler size comparison v20

Scope: corrected shared-heap DSH experimental branch, PDP-6 target,
same source, different supported userland compiler selections.

| Compiler | DXR image (octal) | BSS (octal) | Sum (octal) |
| --- | ---: | ---: | ---: |
| KCC | 037000 | 013742 | 052742 |
| GCC | 032177 | 013742 | 046141 |

GCC saves 04601 octal image words (2433 decimal words) without reducing
any shell limit or introducing overlays.  The GCC build meets the
production DXR image limit of 036000 octal.  These measurements are
linker output, not resident APR-frame or peak BRK measurements.

The dedicated seven-case variable heap target SIMH test passed under GCC
using COMPILER=gcc and DSH_IMAGE_LIMIT=036000.  Full shell conformance
also passed 115/115 under GCC with the production image gate.  Peak
resident-memory results must be recorded separately.  Do not
assume a mixed GCC/KCC executable is supported; select the compiler
consistently using the existing COMPILER build interface.

Policy decision: a 32K DAIMOS configuration requires COMPILER=gcc.
KCC remains supported for larger-memory builds.  The smaller executable
does not by itself establish a successful 32K boot or root DSH EXEC:
INIT residency, allocation granularity, fragmentation, and process
working-set high-water marks remain independent acceptance gates.

Follow-up: compare full-image boot and process memory allocations,
measure sustained command and maximum-variable high-water marks, and
keep the KCC build supported for larger-memory targets even though its
image still exceeds the production 32K-oriented size limit.
