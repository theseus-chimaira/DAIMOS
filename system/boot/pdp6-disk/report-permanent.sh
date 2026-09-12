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

# Boot MRES keeps only the linked text+data+BSS image.  The MRES package
# header, export table, and relocation bitmap live in disposable KINIT and are
# not part of permanent memory.  dlink lays input sections back-to-back with
# no alignment holes, so summing the source DOBJ sections is the installed
# extent size exactly.
mres_objects()
{
        case "$1" in
        cty)     echo 'cty_io' ;;
        clk)     echo 'clk_io' ;;
        ptr)     echo 'ptr_io' ;;
        ptp)     echo 'ptp_io' ;;
        cr)      echo 'cr_io' ;;
        cp)      echo 'cp_io' ;;
        dcs)     echo 'dcs_io' ;;
        ge)      echo 'ge_io' ;;
        dpy)     echo 'dpy_io' ;;
        tty)     echo 'tty_io' ;;
        wcnsls)  echo 'wcnsls_io' ;;
        ocnsls)  echo 'ocnsls_io' ;;
        dsk)     echo 'dsk_io' ;;
        tape)    echo 'tape_io' ;;
        slv)     echo 'slv_io' ;;
        memfs)   echo 'memfs_pdp10' ;;
        dtfs)    echo 'dtfs dtfs_pdp10' ;;
        diskset) echo 'diskset_dispatch' ;;
        d6fs)    echo 'd6fs d6fs_provider d6fs_validate_pdp10 d6fs_pdp10' ;;
        *) return 1 ;;
        esac
}

object_words()
{
        "$objdump" -h "$1" | awk '
            NR == 1 {
                t = d = b = 0
                for (i = 1; i <= NF; ++i) {
                        split($i, a, "=")
                        if (a[1] == "text") t = a[2]
                        else if (a[1] == "data") d = a[2]
                        else if (a[1] == "bss") b = a[2]
                }
                print t + d + b
                exit
            }'
}

total=$kcore
for name in cty clk ptr ptp cr cp dcs ge dpy tty wcnsls ocnsls dsk tape slv \
    memfs dtfs diskset d6fs; do
        package="$build/$name-mres.dobj"
        [ -f "$package" ] || { echo "missing MRES package: $package" >&2; exit 1; }
        words=0
        for obj in $(mres_objects "$name"); do
                file="$build/$obj.dobj"
                [ -f "$file" ] || { echo "missing MRES input: $file" >&2; exit 1; }
                n=$(object_words "$file")
                case "$n" in
                ''|*[!0-9]*) echo "cannot size $file" >&2; exit 1 ;;
                esac
                words=$((words + n))
        done
        printf 'MRES %-8s %06o %6d\n' "$name" "$words" "$words"
        total=$((total + words))
done
printf 'KCORE+MRES      %06o %6d\n' "$total" "$total"
last=$((060 + total - 1))
printf 'PERMANENT_LAST  %06o %6d\n' "$last" "$last"
