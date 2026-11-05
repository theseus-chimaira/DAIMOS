# A04/F02 full P0-D matrix acceptance v1202 (2026-11-05)

## Frozen candidate and execution

- DAIMOS source revision: `5db15ecfe687950c82b492d08476c4816faf77b8`.
- Testkit revision: `122c61ffa81310c3fd6c038f8f7e5db2b5114e09`.
- PDP-6 256K KCC DSK/D6FS, installed `$HOME/cross` simulator/tools.
- Full uninterrupted command: `P0D_FULL=1 sh tests/validation/daimos-p0d-release-matrix-v1096.sh`.
- Exit code **0**, final line `v1096: P0-D component release-stage matrix PASS`.
- Log: `$TMPDIR/daimos-a04-full-matrix-v1202.log`, SHA256
  `5a57f3c2cd966aa5441e0a601118a88804af9ed96ada98dbe286b767732784b6`.
- Evidence archive: `$TMPDIR/daimos-a04-final-v1202.tar.gz`, SHA256
  `254f68a9aca6d9727ab31b86a6fdaf424db80b6bd5957ad17a444be221ea49d4`.

## Verified results

- All 18 source-linked/native P0-D owner-release component stages PASS.
- v1083 direct fatal-release scope PASS.
- v1087 transitive BRK/APR/EXEC to FILE/VFS/HUP/MM/SWAP/WAIT
  no-new-allocation graph PASS, with checked release/ownership edges.
- All 17 `P0D_FULL=1` booted guest invocations PASS in one script:
  v1125, v1124, v997, v1103, v1117, v1118, v1104, v1105,
  v1119, v1121, v1122, v1120 disabled and enabled, v1106, v1107,
  v1077 and v1101.
- Real physical MM_BUSY, MM wrong-owner, swap bitmap refusal,
  physical swap-out/in, TTY/HUP, PIPE/DPY and VM/u-area retries
  preserve owners, exact final releases and WAIT/accounting.
- A separate full-log structural audit confirms exactly 18 stage
  markers, exactly 17 distinct booted invocation markers, both
  v1120 policy checks, and the unique terminal PASS marker.

## Acceptance scope

A04 and F02 are **CLOSED** for the frozen 256K writable-process
requester-only OOM candidate: the full source-linked release graph
and booted P0-D matrix passed on the exact source revision above.
This does **not** establish E13 independent simulator/media coverage,
F07 combined dynamic peak-memory budgets, or F11 final release
integration, nor authorize F12/F13 OOM activation. The separately
tracked 16K permanent residency goal remains unmet. No production
kernel code, OOM switch, ABI, or permanent RAM allocation changed.
