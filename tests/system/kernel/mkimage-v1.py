#!/usr/bin/env python3
"""Patch the KINIT V0.1 test stream with linked addresses."""

import argparse
from pathlib import Path

WORD_MASK = (1 << 36) - 1
HALF_MASK = (1 << 18) - 1
IMAGE_BASE = 0o40000
DAIMON_MAGIC = 0o444151555756
KMAN01_MAGIC = 0o535541562021


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
    parser = argparse.ArgumentParser()
    parser.add_argument("--map", required=True, type=Path)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    symbols = read_map(args.map)
    image_end = require(symbols, "__kinit_image_end")
    code_start = require(symbols, "__kinit_code_start")
    kcore = require(symbols, "test_kcore")
    entry = require(symbols, "kinit_enter")

    if not (0 < code_start < image_end <= HALF_MASK):
        raise SystemExit("invalid KINIT image bounds")
    if not (0 <= kcore < code_start):
        raise SystemExit("invalid test KCORE location")
    if not (0 <= entry < image_end):
        raise SystemExit("invalid KINIT entry")

    words = [int(line, 8) for line in
             args.input.read_text(encoding="ascii").splitlines() if line]
    if len(words) != image_end + 2:
        raise SystemExit("stream length does not match linked image")
    if words[0] != DAIMON_MAGIC:
        raise SystemExit("bad DAIMON stream header")
    if words[2] != KMAN01_MAGIC:
        raise SystemExit("KMAN01 is not at image offset zero")

    words[1] = pair18(image_end, entry)
    words[4] = pair18(IMAGE_BASE + kcore, 1)
    words[5] = pair18(IMAGE_BASE + code_start, image_end - code_start)

    args.output.write_text(
        "".join("%012o\n" % (word & WORD_MASK) for word in words),
        encoding="ascii")


if __name__ == "__main__":
    main()
