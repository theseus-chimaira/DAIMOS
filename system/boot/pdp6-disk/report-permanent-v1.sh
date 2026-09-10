#!/bin/sh
set -eu

usage()
{
        echo "usage: $0 --kcore-map MAP --build DIR --objdump TOOL" >&2
        exit 2
}

kmap=
build=
objdump=
while [ $# -gt 0 ]; do
        case "$1" in
        --kcore-map) [ $# -ge 2 ] || usage; kmap=$2; shift 2 ;;
        --build) [ $# -ge 2 ] || usage; build=$2; shift 2 ;;
        --objdump) [ $# -ge 2 ] || usage; objdump=$2; shift 2 ;;
        *) usage ;;
        esac
done
[ -n "$kmap" ] && [ -n "$build" ] && [ -n "$objdump" ] || usage

end=$(awk '$1 == "__kcore_low_end" { print $2; exit }' "$kmap")
[ -n "$end" ] || { echo "missing __kcore_low_end" >&2; exit 1; }
# KCORE starts at executive location 060.  Linker-map addresses are octal.
kcore=$((0$end - 060))
printf 'KCORE           %06o %6d\n' "$kcore" "$kcore"

total=$kcore
for name in cty clk ptr ptp cr cp dcs ge dpy tty wcnsls ocnsls dsk tape slv \
    memfs dtfs diskset d6fs; do
        file="$build/$name-mres.dobj"
        [ -f "$file" ] || { echo "missing MRES package: $file" >&2; exit 1; }
        words=$($objdump -h "$file" | awk 'NR == 1 { for (i=1;i<=NF;i++) if ($i ~ /^data=/) { sub(/^data=/,"",$i); print $i; exit } }')
        [ -n "$words" ] || { echo "cannot size $file" >&2; exit 1; }
        # pdp10-objdump section sizes are decimal.
        printf 'MRES %-8s %06o %6d\n' "$name" "$words" "$words"
        total=$((total + words))
done
printf 'KCORE+MRES      %06o %6d\n' "$total" "$total"
