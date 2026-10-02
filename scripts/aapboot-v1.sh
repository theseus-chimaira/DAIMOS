#!/bin/sh
set -eu

if [ "$#" -ne 5 ]; then
    echo "usage: aapboot-v1.sh BUILD EMULATOR CONFIG DTA2DTR ROOTSET" >&2
    exit 2
fi

build=$1
emulator=$2
config=$3
dta2dtr=$4
rootset=$5

: "${TMPDIR:=$HOME/tmp}"
mkdir -p "$TMPDIR"

case $rootset in
    1|2|3|4) ;;
    *) echo "aapboot: ROOTSET must be 1, 2, 3, or 4" >&2; exit 2 ;;
esac

if ! command -v "$emulator" >/dev/null 2>&1 && [ ! -x "$emulator" ]; then
    echo "aapboot: emulator not found: $emulator" >&2
    exit 1
fi
[ -r "$config" ] || { echo "aapboot: configuration not found: $config" >&2; exit 1; }
[ -x "$dta2dtr" ] || { echo "aapboot: converter not found: $dta2dtr" >&2; exit 1; }
[ -r "$build/stage1-dtc.pt" ] || { echo "aapboot: missing Stage1" >&2; exit 1; }
[ -r "$build/media/dtc0.tap" ] || { echo "aapboot: missing DTC0 boot tape" >&2; exit 1; }
[ -r "$build/media/dtc1.tap" ] || { echo "aapboot: missing DTC1 root tape" >&2; exit 1; }
[ -r /dev/tty ] && [ -w /dev/tty ] || { echo "aapboot: controlling terminal required" >&2; exit 1; }

run="$TMPDIR/daimos-aapboot-v1-$$"
media="$run/media"
emudir="$run/emu"
mkdir -p "$media" "$emudir"

tty_state=$(stty -g < /dev/tty)
cleanup()
{
    stty "$tty_state" < /dev/tty 2>/dev/null || true
    exec 9>&- 9<&- || true
    rm -rf "$run"
}
trap cleanup EXIT HUP INT TERM

cp "$build/media/dtc0.tap" "$media/dtc0.dta"
truncate -s 591872 "$media/dtc0.dta"
cp "$build/media/dtc1.tap" "$media/dtc1.dta"
truncate -s 591872 "$media/dtc1.dta"
if [ -r "$build/media/dtc2.tap" ]; then
    cp "$build/media/dtc2.tap" "$media/dtc2.dta"
else
    : > "$media/dtc2.dta"
fi
truncate -s 591872 "$media/dtc2.dta"

"$dta2dtr" "$media/dtc0.dta" "$media/dx1.dtr"
"$dta2dtr" "$media/dtc1.dta" "$media/dx2.dtr"
"$dta2dtr" "$media/dtc2.dta" "$media/dx3.dtr"
: > "$media/blank.dta"
truncate -s 591872 "$media/blank.dta"
"$dta2dtr" "$media/blank.dta" "$media/blank.dtr"
i=4
while [ "$i" -le 8 ]; do
    cp "$media/blank.dtr" "$media/dx$i.dtr"
    i=$((i + 1))
done

cp "$build/stage1-dtc.pt" "$media/stage1-dtc.pt"
cp "$config" "$emudir/init.ini"
config_dir=$(dirname "$config")
if [ -d "$config_dir/art" ]; then
    ln -s "$config_dir/art" "$run/art"
fi
root_switch=$((2 + ((rootset - 1) << 3)))
root_switch_octal=$(printf "0%o" "$root_switch")

cat >> "$emudir/init.ini" <<EOF
mount tty /dev/tty
mount ptr $media/stage1-dtc.pt
mount dx1 $media/dx1.dtr
mount dx2 $media/dx2.dtr
mount dx3 $media/dx3.dtr
mount dx4 $media/dx4.dtr
mount dx5 $media/dx5.dtr
mount dx6 $media/dx6.dtr
mount dx7 $media/dx7.dtr
mount dx8 $media/dx8.dtr
switches $root_switch_octal
ptrmotor ptr on
power on
deposit 020 0710600000060
deposit 021 0710740000010
deposit 022 0254000000021
deposit 023 0710440000026
deposit 024 0710740000010
deposit 025 0254000000024
deposit 026 000000000000
deposit 027 0254000000021
start 020
EOF

mkfifo "$run/cmd.fifo"
exec 9<> "$run/cmd.fifo"
echo "DAIMOS aap oldemu: 256K, PCLK, Type 340, JOY/OJOY, DCT/551, 8 DECtapes"
echo "CTY: /dev/tty   root: TSFS tape set $rootset"
cd "$emudir"
"$emulator" < "$run/cmd.fifo"
