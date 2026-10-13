#!/usr/bin/env python3
"""Generate a relocatable MonitorFS namespace probe from one authoritative source.

The replacement names allow a standalone MRES link against a still-resident
KCORE map. This is not a runtime installation and must not be booted.
"""
import re
import sys
from pathlib import Path

EXPORTS = (
    'mfsdev_lookup', 'mfsdev_readdir', 'mfsdev_stat', 'mfsdev_readchar',
    'mfsproc_lookup', 'mfsproc_readdir', 'mfsproc_stat', 'mfsproc_read_words',
    'mfsproc_format_slot', 'mfsproc_parse_slot', 'mfsdom_exists',
)


def main() -> int:
    if len(sys.argv) != 3:
        print('usage: monitorfs-mres-probe-v15.py SOURCE OUTPUT', file=sys.stderr)
        return 2
    content = Path(sys.argv[1]).read_text()
    for symbol in EXPORTS:
        content = re.sub(r'\b' + symbol + r'\b', symbol + '_mres_v15', content)
    Path(sys.argv[2]).write_text('; Generated probe; not installed.\n' + content)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
