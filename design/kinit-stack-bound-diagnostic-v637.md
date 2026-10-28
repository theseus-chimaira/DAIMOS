# KINIT 32K stack-bound diagnostic v637

The retained v493 native 32K TSFS+AUXSTORE build did not reach media
formatting or the simulator. The linked KINIT image ended at octal
`077236`, while 32K core and the configured octal `0600` stack reserve
require an end no higher than `077200`: an overflow of `036` octal,
or 30 decimal words. This is a genuine image-layout failure, not an
AUXSTORE validation result. No stack reserve, image layout, kernel
feature or KCORE resident text has been changed here.

The image builder now prints the linked end, stack-safe end, overflow,
core size, and configured reserve before retaining its original fatal
diagnostic. Testkit `daimos-kinit-stack-bound-v637.sh` checks the exact
boundary rejection using isolated synthetic maps, requiring no media.

An actual 32K TSFS+AUXSTORE native PASS remains open. Correcting the
image overrun requires a separate authorized KINIT footprint/layout
change or other genuinely safe source-level solution; lowering the
reserve is not an acceptable repair. Keep the v493 failure evidence.
