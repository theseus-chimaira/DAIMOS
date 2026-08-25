#!/usr/bin/env python3
import argparse
import struct


def read_records(path):
    records = []
    with open(path, "rb") as f:
        while True:
            raw = f.read(4)
            if not raw:
                break
            if len(raw) != 4:
                raise RuntimeError("short SIMH tape header")
            n = struct.unpack("<I", raw)[0]
            if n == 0:
                records.append(b"")
                continue
            data = f.read(n)
            if len(data) != n:
                raise RuntimeError("short SIMH tape record")
            if n & 1:
                if len(f.read(1)) != 1:
                    raise RuntimeError("short SIMH tape pad")
            trailer = f.read(4)
            if len(trailer) != 4 or struct.unpack("<I", trailer)[0] != n:
                raise RuntimeError("bad SIMH tape trailer")
            records.append(data)
    return records


def decode_words(data):
    if len(data) % 6 != 0:
        raise RuntimeError("7-track record is not a whole number of words")
    out = []
    for off in range(0, len(data), 6):
        word = 0
        for ch in data[off:off + 6]:
            word = (word << 6) | (ch & 0o77)
        out.append(word)
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--tape", required=True)
    parser.add_argument("--record", type=int, required=True)
    parser.add_argument("--word", action="append", default=[])
    args = parser.parse_args()
    records = read_records(args.tape)
    if args.record >= len(records):
        raise RuntimeError("missing written SIMH tape record")
    got = decode_words(records[args.record])
    want = [int(value, 8) for value in args.word]
    if got != want:
        raise RuntimeError("MTC write mismatch: got %r want %r" % (got, want))
    print("PDP-6 MTC resident write test PASS")


if __name__ == "__main__":
    main()
