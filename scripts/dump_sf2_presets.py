#!/usr/bin/env python3

import struct
import sys
from pathlib import Path

PHDR_SIZE = 38

def iter_chunks(data: bytes, start: int, end: int):
    pos = start
    while pos + 8 <= end:
        chunk_id = data[pos:pos+4]
        size = struct.unpack_from("<I", data, pos + 4)[0]
        body_start = pos + 8
        body_end = body_start + size

        yield chunk_id, body_start, body_end

        # RIFF chunks are word-aligned
        pos = body_end + (size & 1)

def find_phdr(data: bytes) -> bytes:
    if data[0:4] != b"RIFF" or data[8:12] != b"sfbk":
        raise ValueError("not a RIFF SoundFont file")

    riff_size = struct.unpack_from("<I", data, 4)[0]
    riff_end = 8 + riff_size

    for chunk_id, body_start, body_end in iter_chunks(data, 12, riff_end):
        if chunk_id != b"LIST":
            continue

        list_type = data[body_start:body_start+4]
        if list_type != b"pdta":
            continue

        for sub_id, sub_start, sub_end in iter_chunks(data, body_start + 4, body_end):
            if sub_id == b"phdr":
                return data[sub_start:sub_end]

    raise ValueError("no pdta/phdr chunk found")

def clean_name(raw: bytes) -> str:
    return raw.split(b"\x00", 1)[0].decode("latin1", errors="replace").strip()

def dump_presets(sf2_path: Path):
    data = sf2_path.read_bytes()
    phdr = find_phdr(data)

    if len(phdr) % PHDR_SIZE != 0:
        raise ValueError(f"bad phdr size: {len(phdr)}")

    records = len(phdr) // PHDR_SIZE

    for i in range(records):
        rec = phdr[i * PHDR_SIZE:(i + 1) * PHDR_SIZE]

        name = clean_name(rec[0:20])
        preset, bank, bag_index = struct.unpack_from("<HHH", rec, 20)

        # Final terminal record; not a real playable preset.
        if i == records - 1:
            continue

        print(f"{bank:03d}-{preset:03d} {name}")

def main():
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} file.sf2", file=sys.stderr)
        sys.exit(2)

    dump_presets(Path(sys.argv[1]))

if __name__ == "__main__":
    main()
