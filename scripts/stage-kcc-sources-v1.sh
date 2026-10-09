#!/bin/sh
# Convert the host KCC sources to native C-SIX and a native MAKEFILE.
set -eu
if [ "$#" -ne 7 ]; then
        echo 'usage: stage-kcc-sources-v1.sh REPO OUT CSIX S6TEXT GENERATOR DAIMOS NATIVE' >&2
        exit 2
fi
repo=$1
out=$2
csix=$3
s6text=$4
generator=$5
daimos=$6
native=$7
if [ ! -f "$repo/cc.c" ] || [ ! -f "$repo/cc.h" ]; then
        echo "KCC source tree missing: $repo" >&2
        exit 1
fi
mkdir -p "$out/runtime" "$out/self/include/sys" "$out/ABI" "$out/BOOT"
for src in "$repo"/*.[chs] "$repo"/runtime/*.[cs] \
        "$repo"/self/include/*.h "$repo"/self/include/sys/*.h; do
        [ -f "$src" ] || continue
        rel=${src#"$repo"/}
        # Historical KCC uses form-feed as a page separator.  It is C
        # whitespace, but C-SIX cannot represent it; a space preserves
        # tokenization and the original physical line numbers.
        case "$src" in
        *.s)
                # DAS consumes ordinary SIXBIT S6REC, not C-SIX escapes.
                tr '\014' ' ' < "$src" | expand -t 8 | \
                    tr '[:lower:]' '[:upper:]' > "$out/$rel.ascii"
                "$s6text" --encode "$out/$rel.ascii" "$out/$rel"
                ;;
        *)
                tr '\014' ' ' < "$src" > "$out/$rel.ascii"
                "$csix" -e "$out/$rel.ascii" "$out/$rel"
                ;;
        esac
        rm -f "$out/$rel.ascii"
done
python3 "$generator" "$repo" "$out/native-make"
for src in "$out"/native-make/*; do
        name=${src##*/}
        [ "$name" = MONOLITH ] && continue
        tr '[:lower:]' '[:upper:]' < "$src" > "$out/$name.ascii"
        "$s6text" --encode "$out/$name.ascii" "$out/$name"
        rm -f "$out/$name.ascii"
done
for spec in userland/libc/u.h userland/libc/dsys.h \
        system/kernel/proc/syscall.h system/kernel/fs/file.h \
        system/kernel/fs/vfs.h system/kernel/core/kcore.h \
        system/kernel/storage/storage.h tools/host/pdp10-sixbit.h; do
        src="$daimos/$spec"
        name=${src##*/}
        tr '\014' ' ' < "$src" > "$out/ABI/$name.ascii"
        "$csix" -e "$out/ABI/$name.ascii" "$out/ABI/$name"
        rm -f "$out/ABI/$name.ascii"
done
for spec in runtime/crt0.dobj:CRT0.DOBJ \
        runtime/daimos-bootstrap.dobj:BOOT.DOBJ \
        daimos-libc/libc/syscall.dobj:SYS.DOBJ \
        runtime/syscall-helpers.dobj:HELP.DOBJ \
        daimos-libc/libc/libc.a:LIBC.DARC; do
        source=${spec%%:*}
        target=${spec#*:}
        [ -s "$native/$source" ] || {
                echo "missing native KCC library: $native/$source" >&2
                exit 1
        }
        cp "$native/$source" "$out/BOOT/$target"
done
