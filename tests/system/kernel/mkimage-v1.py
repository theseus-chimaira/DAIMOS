#!/usr/bin/env python3
"""Install the already-linked resident image into the high KINIT stream."""

import argparse
from pathlib import Path

WORD_MASK = (1 << 36) - 1
HALF_MASK = (1 << 18) - 1
IMAGE_BASE = 0o40000
KCORE_BASE = 0o60
DAIMON_MAGIC = 0o444151555756


def pair18(left, right):
    return ((left & HALF_MASK) << 18) | (right & HALF_MASK)


def read_map(path):
    symbols = {}
    for line in path.read_text(encoding="ascii").splitlines():
        fields = line.split()
        if len(fields) == 2:
            try:
                symbols[fields[0]] = int(fields[1], 8)
            except ValueError:
                pass
    return symbols


def require(symbols, name):
    if name not in symbols:
        raise SystemExit("missing link symbol: " + name)
    return symbols[name]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--map", required=True, type=Path)
    ap.add_argument("--input", required=True, type=Path)
    ap.add_argument("--resident", required=True, type=Path)
    ap.add_argument("--resident-map", required=True, type=Path)
    ap.add_argument("--output", required=True, type=Path)
    args = ap.parse_args()

    syms = read_map(args.map)
    rsyms = read_map(args.resident_map)
    image_start = require(syms, "__kinit_image_start")
    image_end = require(syms, "__kinit_image_end")
    load_begin = require(syms, "__resident_load_begin")
    load_end = require(syms, "__resident_load_end")
    entry = require(syms, "kinit_enter")
    resident_init_end = require(rsyms, "__resident_low_init_end")

    if image_start != IMAGE_BASE or not (image_start < image_end <= HALF_MASK):
        raise SystemExit("invalid KINIT image bounds")
    if not (image_start <= load_begin <= load_end <= image_end):
        raise SystemExit("invalid resident load slot")
    if not (image_start <= entry < image_end):
        raise SystemExit("invalid KINIT entry")

    image_words = image_end - IMAGE_BASE
    resident_words = resident_init_end - KCORE_BASE
    if load_end - load_begin != resident_words:
        raise SystemExit("resident slot size mismatch")

    words = [int(x, 8) for x in args.input.read_text(encoding="ascii").splitlines() if x]
    resident = [int(x, 8) for x in args.resident.read_text(encoding="ascii").splitlines() if x]
    if words[0] != DAIMON_MAGIC:
        raise SystemExit("bad DAIMON stream header")
    if len(words) != image_words + 2:
        raise SystemExit("KINIT stream length mismatch")
    if len(resident) < 2:
        raise SystemExit("short resident stream")
    resident = resident[2:]
    if len(resident) != resident_words:
        raise SystemExit("resident stream length mismatch")

    words[1] = pair18(image_words, entry - IMAGE_BASE)
    off = 2 + (load_begin - IMAGE_BASE)
    words[off:off + resident_words] = resident
    args.output.write_text("".join("%012o\n" % (w & WORD_MASK) for w in words), encoding="ascii")


if __name__ == "__main__":
    main()
