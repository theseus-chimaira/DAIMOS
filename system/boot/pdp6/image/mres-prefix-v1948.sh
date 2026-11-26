#!/bin/sh
# Calculate the disposable KINIT prefix needed for contiguous MRES install.
# The embedded KCORE slot retains its exact initialized size; any extra
# zero words precede it and are reclaimed with the MINIT table and slot.
# Count linked resident words, not the package's relocation/export data.
set -eu
objdump=$1
kmap=$2
table=$3
mm_header=$4
shift 4
sym()
{
        while read name value rest; do
                if [ "$name" = "$1" ]; then
                        printf '%s\n' "$value"
                        return
                fi
        done < "$kmap"
        return 1
}
low_end=$(sym __kcore_low_end)
init_end=$(sym __kcore_low_init_end)
capacity=$(awk '$1=="#define" && $2=="MM_MAX_EXTENTS" {print $3}' "$mm_header")
case "$capacity" in ''|*[!0-9]*) exit 1 ;; esac
table_words=
for field in $("$objdump" -h "$table"); do
        case "$field" in text=*) table_words=${field#text=} ;; esac
done
case "$table_words" in ''|*[!0-9]*) exit 1 ;; esac
common=0
swap=0
pure=0
dtfs=0
d6fs=0
block=0
for package do
        resident=
        for field in $("$objdump" -h "$package"); do
                case "$field" in resident=*) resident=${field#resident=} ;; esac
        done
        case "$resident" in ''|*[!0-9]*) echo "invalid MRES size: $package" >&2; exit 1 ;; esac
        case "${package##*/}" in
        swapv481-mres.dobj) swap=$resident ;;
        purev1612-mres.dobj) pure=$resident ;;
        dtfs-mres.dobj) dtfs=$resident ;;
        d6fs-mres.dobj) d6fs=$resident ;;
        blockset-mres.dobj|badmap-mres.dobj) block=$((block+resident)) ;;
        *) common=$((common+resident)) ;;
        esac
done
# Swap and PURE are mutually exclusive. DTFS/TSFS and D6FS are root
# alternatives; BLOCKSET/BADMAP are only installed for block roots.
# Optional device and LOGSTORE packages are conservatively all counted.
tape_mem=$swap
if [ "$pure" -gt "$tape_mem" ]; then tape_mem=$pure; fi
tape_total=$((common+tape_mem+dtfs))
disk_total=$((common+swap+d6fs+block))
resident=$disk_total
if [ "$tape_total" -gt "$resident" ]; then resident=$tape_total; fi
# The simulator may discover more RAM than the configured image minimum,
# so account for the maximum runtime MM table, not only the build's core.
need=$((0$low_end+2*capacity+resident))
prefix_end=$((030000+table_words+0$init_end-060))
pad=0
if [ "$need" -gt "$prefix_end" ]; then pad=$((need-prefix_end)); fi
printf 'MRES prefix: bound %06o, unpadded end %06o, pad %06o (%d words)\n' "$need" "$prefix_end" "$pad" "$pad" >&2
printf '%o\n' "$pad"
