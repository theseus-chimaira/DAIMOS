#!/bin/sh
# Strict GCC 32K KCORE link-end gate; normal development links stay available.
set -eu
if [ "$#" -ne 1 ]; then
    echo "usage: $0 KCORE_MAP" >&2
    exit 2
fi
map=$1
end=$(awk '$1 == "__kcore_low_end" { print $2; count++ } END { if (count != 1) exit 1 }' "$map") || {
    echo "KCORE 32K: missing or duplicate __kcore_low_end in $map" >&2
    exit 2
}
case "$end" in
    *[!0-7]*|'') echo "KCORE 32K: invalid octal end: $end" >&2; exit 2 ;;
esac
words=$((0$end))
limit=$((020000))
if [ "$words" -ge "$limit" ]; then
    echo "KCORE 32K FAIL: end $end >= 020000 (must be strictly below)" >&2
    exit 1
fi
echo "KCORE 32K PASS: end $end < 020000"
