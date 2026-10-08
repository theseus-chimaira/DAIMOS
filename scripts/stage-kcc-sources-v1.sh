#!/bin/sh
# Convert the host KCC sources to native C-SIX and a native MAKEFILE.
set -eu
if [ "$#" -ne 5 ]; then
        echo 'usage: stage-kcc-sources-v1.sh REPO OUT CSIX S6TEXT MAKEFILE' >&2
        exit 2
fi
repo=$1
out=$2
csix=$3
s6text=$4
makefile=$5
mkdir -p "$out/runtime" "$out/self/include/sys"
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
awk '{ line=$0; if (substr(line,1,1)=="\t") line="> " substr(line,2); \
       print toupper(line); }' "$makefile" > "$out/MAKEFILE.native.txt"
"$s6text" --encode "$out/MAKEFILE.native.txt" "$out/MAKEFILE"
rm -f "$out/MAKEFILE.native.txt"
