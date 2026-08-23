#!/usr/bin/env python3
"""Verify the WCNSLS color-scope DAIMOS boot banner."""

import argparse
import re
from pathlib import Path

DATAO_RE = re.compile(r"WCNSLS DATAIO: DATAO ([0-7]{12})")
CONO_RE = re.compile(r"WCNSLS CONO: ([0-7]{6})")

TITLE = [
    0o364306143076,
    0o164307743061,
    0o371020410237,
    0o216726543061,
    0o164306143056,
    0o174101602076,
    0,
]
DIGITS = [
    0o164316563056,
    0o043020410216,
    0o164204210437,
    0o360205602076,
    0o021452276102,
    0o374103602076,
    0o164103643056,
    0o370210420410,
    0o164305643056,
    0o164305702056,
]
GLYPH_V = 0o214306142504
GLYPH_DOT = 0o000000000306
GLYPH_DASH = 0o000003700000
GREEN_CONO = 0o3740


def version_glyph(ch):
    if "0" <= ch <= "9":
        return DIGITS[ord(ch) - ord("0")]
    if ch == "V":
        return GLYPH_V
    if ch == ".":
        return GLYPH_DOT
    if ch == "-":
        return GLYPH_DASH
    return 0


def glyph_points(glyph, x):
    points = []
    for row in range(7):
        for col in range(5):
            bit = 34 - row * 5 - col
            if (glyph >> bit) & 1:
                px = x + col * 7
                py = 0o330 - row * 7
                points.append(((px & 0o777) << 9) | (py & 0o777))
    return points


def banner_points(version):
    points = []
    x = 0o25
    for glyph in TITLE:
        points.extend(glyph_points(glyph, x))
        x += 42
    for ch in "V" + version + "  ":
        points.extend(glyph_points(version_glyph(ch), x))
        x += 42
    return points


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--debug-log", required=True)
    parser.add_argument("--simh-log", required=True)
    parser.add_argument("--version", required=True)
    args = parser.parse_args()

    version = Path(args.version).read_text(encoding="ascii").strip()
    debug_text = Path(args.debug_log).read_text(encoding="ascii")
    simh_text = Path(args.simh_log).read_text(encoding="ascii")

    cono = [int(m.group(1), 8) for m in CONO_RE.finditer(debug_text)]
    if not cono or cono[0] != GREEN_CONO:
        raise SystemExit("WCNSLS banner CONO mismatch: seen=%s expected=%06o" %
                         ([format(v, "06o") for v in cono], GREEN_CONO))

    seen = [int(m.group(1), 8) for m in DATAO_RE.finditer(debug_text)]
    expected = banner_points(version)
    if seen != expected:
        raise SystemExit("WCNSLS banner DATAO mismatch:\nseen=%s\nexpected=%s" %
                         ([format(v, "012o") for v in seen],
                          [format(v, "012o") for v in expected]))

    lines = [line.rstrip() for line in simh_text.splitlines()]
    expected_diag = "WCNSLS                            LOADED"
    if expected_diag not in lines:
        raise SystemExit("WCNSLS LOADED diagnostic missing")

    print("PDP-6 WCNSLS color-scope DAIMOS version banner test PASS")


if __name__ == "__main__":
    main()
