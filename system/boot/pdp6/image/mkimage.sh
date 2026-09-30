#!/bin/sh
# Install the fixed KCORE stream in the linked KINIT load slot.
set -eu

fail() { echo "mkimage: $*" >&2; exit 1; }
map= input= kcore= kcore_map= output= core_words=0100000 stack_words=02000
while [ "$#" -gt 0 ]; do
        case $1 in
        --map) map=$2; shift 2 ;;
        --input) input=$2; shift 2 ;;
        --kcore) kcore=$2; shift 2 ;;
        --kcore-map) kcore_map=$2; shift 2 ;;
        --output) output=$2; shift 2 ;;
        --core-words) core_words=$2; shift 2 ;;
        --stack-words) stack_words=$2; shift 2 ;;
        *) fail "unknown argument: $1" ;;
        esac
done
for path in "$map" "$input" "$kcore" "$kcore_map"; do
        [ -f "$path" ] || fail "missing input: $path"
done
[ -n "$output" ] || fail "--output is required"

sym()
{
        file=$1
        name=$2
        value=$(awk -v name="$name" '$1 == name { print $2; exit }' "$file")
        [ -n "$value" ] || fail "missing link symbol: $name"
        case $value in *[!0-7]*) fail "invalid octal link symbol $name=$value" ;; esac
        printf '%s\n' "$value"
}

IMAGE_BASE=$((030000))
KCORE_BASE=$((060))
case $core_words in
*[!0-7]*) fail "invalid octal core size: $core_words" ;;
esac
case $stack_words in
*[!0-7]*) fail "invalid octal stack reserve: $stack_words" ;;
esac
KINIT_CORE_TOP=$((0$core_words))
KINIT_MIN_STACK_WORDS=$((0$stack_words))
HALF_MASK=$((0777777))
DAIMON_MAGIC=$((0444151555756))
image_start=$((0$(sym "$map" __kinit_image_start)))
image_end=$((0$(sym "$map" __kinit_image_end)))
load_begin=$((0$(sym "$map" __kcore_load_begin)))
load_end=$((0$(sym "$map" __kcore_load_end)))
entry=$((0$(sym "$map" kinit_enter)))
kcore_init_end=$((0$(sym "$kcore_map" __kcore_low_init_end)))

[ "$image_start" -eq "$IMAGE_BASE" ] && [ "$image_start" -lt "$image_end" ] && \
    [ "$image_end" -le "$HALF_MASK" ] || fail "invalid KINIT image bounds"
[ "$KINIT_CORE_TOP" -ge "$KINIT_MIN_STACK_WORDS" ] || fail "configured core is smaller than the KINIT stack reserve"
[ "$image_end" -le $((KINIT_CORE_TOP - KINIT_MIN_STACK_WORDS)) ] || fail "KINIT image leaves less than the configured stack reserve"
[ "$image_start" -le "$load_begin" ] && [ "$load_begin" -le "$load_end" ] && \
    [ "$load_end" -le "$image_end" ] || fail "invalid KCORE load slot"
[ "$image_start" -le "$entry" ] && [ "$entry" -lt "$image_end" ] || fail "invalid KINIT entry"
image_words=$((image_end - IMAGE_BASE))
kcore_words=$((kcore_init_end - KCORE_BASE))
[ $((load_end - load_begin)) -eq "$kcore_words" ] || fail "KCORE slot size mismatch"

bad=$(grep -nEv '^[0-7]{1,12}$' "$input" | head -1 || true)
[ -z "$bad" ] || fail "$input:${bad%%:*} is not an octal word"
bad=$(grep -nEv '^[0-7]{1,12}$' "$kcore" | head -1 || true)
[ -z "$bad" ] || fail "$kcore:${bad%%:*} is not an octal word"
input_lines=$(wc -l < "$input" | tr -d ' ')
kcore_lines=$(wc -l < "$kcore" | tr -d ' ')
[ "$input_lines" -eq $((image_words + 2)) ] || fail "KINIT stream length mismatch"
[ "$kcore_lines" -eq $((kcore_words + 2)) ] || fail "KCORE stream length mismatch"
header=$(sed -n '1p' "$input")
[ $((0$header)) -eq "$DAIMON_MAGIC" ] || fail "bad DAIMON stream header"

descriptor=$((((image_words & HALF_MASK) * 01000000) + ((entry - IMAGE_BASE) & HALF_MASK)))
off=$((2 + load_begin - IMAGE_BASE))
suffix=$((off + kcore_words + 1))
tmp=${output}.tmp.$$
trap 'rm -f "$tmp"' 0 1 2 3 15
{
        printf '%012o\n' "$DAIMON_MAGIC"
        printf '%012o\n' "$descriptor"
        if [ "$off" -gt 2 ]; then sed -n "3,${off}p" "$input"; fi
        tail -n +3 "$kcore"
        if [ "$suffix" -le "$input_lines" ]; then tail -n +"$suffix" "$input"; fi
} > "$tmp"
[ "$(wc -l < "$tmp" | tr -d ' ')" -eq "$input_lines" ] || fail "output stream length mismatch"
mv "$tmp" "$output"
trap - 0 1 2 3 15
