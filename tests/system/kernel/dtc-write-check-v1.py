#!/usr/bin/env python3
import argparse
import struct
from pathlib import Path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--tape', required=True)
    ap.add_argument('--block', type=int, required=True)
    args = ap.parse_args()
    data = Path(args.tape).read_bytes()
    if len(data) % 4:
        raise SystemExit('bad DECtape image size')
    words18 = struct.unpack('<%dI' % (len(data) // 4), data)
    base = args.block * 256
    if base + 256 > len(words18):
        raise SystemExit('block outside DECtape image')
    for i in range(128):
        got = ((words18[base + 2*i] & 0x3ffff) << 18) | (words18[base + 2*i + 1] & 0x3ffff)
        if got != i:
            raise SystemExit('DTC block word %d: got %012o expected %012o' % (i, got, i))
    print('PDP-6 DTC resident block write test PASS')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
