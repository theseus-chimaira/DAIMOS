#!/bin/sh
# Report permanent KCORE+MRES occupancy against the configured PDP-6 address
# ceiling.  MRES package headers/relocation maps are disposable KINIT material;
# only each installed text+data+BSS extent contributes to permanent residency.
set -eu

usage()
{
        echo "usage: $0 --kcore-map MAP --build DIR --objdump TOOL [--omit-blockset] [--limit OCTAL]" >&2
        exit 2
}

kmap=
build=
objdump=
omit_blockset=0
limit_arg=040000
while [ $# -gt 0 ]; do
        case "$1" in
        --kcore-map) [ $# -ge 2 ] || usage; kmap=$2; shift 2 ;;
        --build) [ $# -ge 2 ] || usage; build=$2; shift 2 ;;
        --objdump) [ $# -ge 2 ] || usage; objdump=$2; shift 2 ;;
        --omit-blockset) omit_blockset=1; shift ;;
        --limit) [ $# -ge 2 ] || usage; limit_arg=$2; shift 2 ;;
        *) usage ;;
        esac
done
[ -n "$kmap" ] && [ -n "$build" ] && [ -n "$objdump" ] || usage
case "$limit_arg" in
''|*[!0-7]*) echo "invalid octal limit: $limit_arg" >&2; exit 2 ;;
esac
limit=$((0$limit_arg))

end=
while read sym_name sym_value rest; do
        if [ "$sym_name" = "__kcore_low_end" ]; then
                end=$sym_value
                break
        fi
done < "$kmap"
[ -n "$end" ] || { echo "missing __kcore_low_end" >&2; exit 1; }
# KCORE starts at executive location 060.  Linker-map addresses are octal.
kcore=$((0$end - 060))
printf 'KCORE           %06o %6d\n' "$kcore" "$kcore"

# Boot MRES keeps only the linked initialized+BSS image.  The package header,
# export table, and relocation bitmap live in disposable KINIT.  Measure the
# MRES1 package itself rather than summing its input DOBJs: dlink fold plans
# may remove duplicate input text before packaging.
mres_words()
{
        set -- $("$objdump" -h "$1")
        for field in "$@"; do
                case "$field" in
                resident=*)
                        printf '%s\n' "${field#resident=}"
                        return 0
                        ;;
                esac
        done
        return 1
}

total=$kcore
blockset_words=0
for name in cty clk ptr ptp lpt cr cp dcs ge dpy tty wcnsls ocnsls dsk tape slv \
    drm memfs dtfs blockset logstore badmap d6fs; do
        package="$build/$name-mres.dobj"
        [ -f "$package" ] || { echo "missing MRES package: $package" >&2; exit 1; }
        words=$(mres_words "$package") || {
                echo "cannot read MRES resident size: $package" >&2
                exit 1
        }
        case "$words" in
        ''|*[!0-9]*) echo "invalid MRES resident size: $package" >&2; exit 1 ;;
        esac
        resident=$words
        if [ "$name" = blockset ]; then
                blockset_words=$words
        fi
        if [ "$name" = blockset ] && [ "$omit_blockset" -eq 1 ]; then
                resident=0
                printf 'MRES %-8s %06o %6d\n' "$name" "$resident" "$resident"
                printf 'PKG  %-8s %06o %6d\n' "$name" "$words" "$words"
        else
                printf 'MRES %-8s %06o %6d\n' "$name" "$resident" "$resident"
        fi
        total=$((total + resident))
done
printf 'KCORE+MRES      %06o %6d\n' "$total" "$total"
last=$((060 + total - 1))
printf 'PERMANENT_LAST  %06o %6d\n' "$last" "$last"

check_limit()
{
        label=$1
        high=$2
        distance=$((limit - high))
        free=$((distance - 1))
        printf '%-16s limit=%06o distance=%d free=%d\n' "$label" "$limit" \
            "$distance" "$free"
        if [ "$high" -ge "$limit" ]; then
                printf '%s exceeds permanent-address limit %06o: last=%06o\n' \
                    "$label" "$limit" "$high" >&2
                return 1
        fi
}

status=0
check_limit 'SINGLE_ROOT' "$last" || status=1
if [ "$omit_blockset" -eq 1 ]; then
        multi_total=$((total + blockset_words))
        multi_last=$((060 + multi_total - 1))
        printf 'MULTI_ROOT       %06o %6d\n' "$multi_total" "$multi_total"
        printf 'MULTI_LAST       %06o %6d\n' "$multi_last" "$multi_last"
        check_limit 'MULTI_ROOT' "$multi_last" || status=1
fi
exit "$status"
