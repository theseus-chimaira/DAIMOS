#!/usr/bin/env python3
"""Verify the exact Type 340 DATAO stream for the DAIMOS boot banner."""

import argparse
import re
from pathlib import Path

DATO_RE = re.compile(r"DPY 131 DATO ([0-7]{12})")
MODE_POINT = 1
MODE_CHAR = 3
SPACE = 0o40


def param_mode(mode):
    return (mode & 0o7) << 13


def point_coord(yflag, coord, next_mode):
    value = (next_mode & 0o7) << 13
    if yflag:
        value |= 0o200000
    value |= coord & 0o1777
    return value & 0o777777


def char3(c0, c1, c2):
    return ((c0 & 0o77) << 12) | ((c1 & 0o77) << 6) | (c2 & 0o77)


def inst(left, right):
    return ((left & 0o777777) << 18) | (right & 0o777777)


def type342_code(ch):
    value = ord(ch)
    if "A" <= ch <= "Z":
        return value - ord("A") + 1
    if 0o40 <= value <= 0o77:
        return value
    return 0o77


def banner_words(version):
    text = "DAIMOS V" + version + "  "
    codes = [type342_code(ch) for ch in text]
    words = [
        inst(param_mode(MODE_POINT), point_coord(0, 0o240, MODE_POINT)),
        inst(point_coord(1, 0o1000, 0), param_mode(0)),
    ]

    first = codes[:3]
    codes = codes[3:]
    while len(first) < 3:
        first.append(SPACE)
    words.append(inst(param_mode(MODE_CHAR), char3(*first)))

    while codes:
        group = codes[:6]
        codes = codes[6:]
        while len(group) < 6:
            group.append(SPACE)
        words.append(inst(char3(*group[:3]), char3(*group[3:])))
    return words


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", required=True)
    parser.add_argument("--version", required=True)
    args = parser.parse_args()

    version = Path(args.version).read_text(encoding="ascii").strip()
    seen = [int(match.group(1), 8) for match in
            DATO_RE.finditer(Path(args.log).read_text(encoding="ascii"))]
    expected = banner_words(version)
    if seen != expected:
        raise SystemExit("DPY banner DATAO mismatch:\nseen=%s\nexpected=%s" %
                         ([format(word, "012o") for word in seen],
                          [format(word, "012o") for word in expected]))
    print("PDP-6 DPY DAIMOS version banner test PASS")


if __name__ == "__main__":
    main()
