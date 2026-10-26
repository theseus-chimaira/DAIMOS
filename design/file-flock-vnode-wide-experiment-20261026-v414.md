# Proposal B v414: rejected intermediate FLOCK-only conversion

Status: experiment REVERTED; no production code retained.

The user approved vnode-wide process-local FLOCK and FILE provider/
local-kind 0..7 for the full Proposal B migration. As an intermediate
experiment, the existing packed descriptor representation was kept
while assembly `file_lock` was changed to update all same-vnode FDs,
removing the original family-conflict scan. Successful regular-file
OPEN was modified to inherit an existing same-vnode lock mode using a
bounded descriptor scan. The existing daimos-testkit FILE target
assertions were provisionally changed to reflect new FLOCK semantics.

Built the complete linked GCC PDP-6 lowmem image with
`make -s permanent-size PDP10_PREFIX=$HOME/cross COMPILER=gcc
BOOT_PROFILE=lowmem`:

                         baseline      experiment      delta
  KCORE                     9109            9110         +1
  KCORE+MRES               15657           15658         +1
  PERMANENT_LAST           15704           15705         +1

This violates the memory-first goal and was reverted immediately,
including the provisional target assertion change. No runtime tests
were claimed for the reverted code. It is not a valid production
milestone; the intermediate OPEN lock inheritance scan offsets the
FLOCK conflict-check removal in the linked image.

The host-only v413 encoding test in daimos-testkit remains committed
and passed independently; the unrelated existing testkit working-tree
modifications were not touched.

Next: avoid stand-alone intermediate conversions with extra code;
the full descriptor conversion must remove mount-extension handling
and former family-token logic together, then measure the whole linked
image. Continue to preserve the existing baseline and reject all
changes raising total RAM.
