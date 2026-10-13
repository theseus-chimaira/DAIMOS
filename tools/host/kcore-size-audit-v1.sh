#!/bin/sh
# Summarize PDP-6 KCORE object sections from a completed GCC build.
# Object totals are pre-fold and must not be equated with resident savings.
set -eu
if [ "$#" -ne 1 ]; then
    echo "usage: $0 BOOT_BUILD_DIRECTORY" >&2
    exit 2
fi
build=$1
test -f "$build/kcore.map" || { echo "missing KCORE map" >&2; exit 2; }
command -v pdp10-objdump >/dev/null 2>&1 || exit 2
awk '$1 == "__kcore_low_end" { print "LINK_END_OCTAL=" $2; found++ }
    END { if (found != 1) exit 1 }' "$build/kcore.map"
printf 'OBJECT TEXT DATA BSS TOTAL_DECIMAL\n'
for obj in "$build"/*.dobj; do
    [ -f "$obj" ] || continue
    # Diagnostic focus: major known fixed-KCORE units (not MRES packages).
    case " $(basename "$obj" .dobj) " in
        ' proc_pdp6 '|' file_runtime '|' monitorfs_runtime '|' vfs '|\
        ' syscall_dispatch_pdp6 '|' mm '|' proc_swap '|' exec '|\
        ' backstore '|' dsk270 '|' drm236 '|' pclk ') ;;
        *) continue ;;
    esac
    sections=$(pdp10-objdump -h "$obj" | sed -n 's/^sections: text=\([0-9]*\) data=\([0-9]*\) bss=\([0-9]*\).*/\1 \2 \3/p')
    [ -n "$sections" ] || continue
    set -- $sections
    printf '%s %s %s %s %s\n' "$(basename "$obj" .dobj)" "$1" "$2" "$3" "$(( $1 + $2 + $3 ))"
done
