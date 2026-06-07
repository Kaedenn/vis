#!/usr/bin/env python3

"""Module for parsing MIDI files and extracting note events."""

import csv
import contextlib
import struct
import sys
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import argparse

import mido

DEFAULT_TEMPO = 500_000  # 120 BPM in microseconds/beat
PHDR_SIZE = 38

def iter_chunks(data: bytes, start: int, end: int):
    """Iterate over RIFF chunks in data."""
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
    """Find the phdr chunk in a RIFF SoundFont file."""
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
    """Clean null-terminated strings from SF2."""
    return raw.split(b"\x00", 1)[0].decode("latin1", errors="replace").strip()

def parse_sf2_presets(sf2_path: Path) -> Dict[Tuple[int, int], str]:
    """Parse SF2 file to extract preset names mapping."""
    data = sf2_path.read_bytes()
    phdr = find_phdr(data)

    if len(phdr) % PHDR_SIZE != 0:
        raise ValueError(f"bad phdr size: {len(phdr)}")

    records = len(phdr) // PHDR_SIZE
    presets: Dict[Tuple[int, int], str] = {}

    for i in range(records):
        rec = phdr[i * PHDR_SIZE:(i + 1) * PHDR_SIZE]

        name = clean_name(rec[0:20])
        preset, bank, _ = struct.unpack_from("<HHH", rec, 20)

        # Final terminal record; not a real playable preset.
        if i == records - 1:
            continue

        presets[(bank, preset)] = name

    return presets

@dataclass(frozen=True)
class NoteEvent:
    """Represents a single parsed MIDI note event."""
    index: int
    track: int
    channel: int
    note: int
    note_name: str
    velocity: int
    start_ticks: int
    start_seconds: float
    bank_msb: int = 0
    bank_lsb: int = 0
    bank: int = 0
    program: int = 0
    preset: Optional[str] = None
    duration_ticks: Optional[int] = None
    duration_seconds: Optional[float] = None

    @property
    def start_msec(self):
        """Get start time in milliseconds."""
        return int(self.start_seconds * 1000)

    @property
    def duration_msec(self):
        """Get duration in milliseconds."""
        return int(self.duration_seconds * 1000)

NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]

def note_to_name(n: int) -> str:
    """
    Convert a MIDI note to a standard note-octave pair.
    """
    octave = (n // 12) - 1
    name = NOTE_NAMES[n % 12]
    return f"{name}{octave}"

def build_tempo_segments(mid: mido.MidiFile) -> List[Tuple[int, int]]:
    """
    Returns a list of (abs_tick, tempo_us_per_beat) sorted by abs_tick for the
    *whole song*, based on merged tracks (so you get global timing changes).
    """
    tempo_changes: List[Tuple[int, int]] = [(0, DEFAULT_TEMPO)]
    abs_tick = 0

    for msg in mido.merge_tracks(mid.tracks):
        abs_tick += msg.time  # msg.time is delta ticks in merged stream
        if msg.type == "set_tempo":
            tempo_changes.append((abs_tick, msg.tempo))

    # ensure sorted, and collapse duplicates at same tick (keep last)
    tempo_changes.sort(key=lambda x: x[0])
    collapsed: List[Tuple[int, int]] = []
    for t, tempo in tempo_changes:
        if collapsed and collapsed[-1][0] == t:
            collapsed[-1] = (t, tempo)
        else:
            collapsed.append((t, tempo))
    return collapsed

def ticks_to_seconds(
    ticks: int,
    ticks_per_beat: int,
    tempo_segments: List[Tuple[int, int]]
) -> float:
    """
    Convert an absolute tick value to absolute seconds from time 0,
    accounting for tempo changes.
    """
    # Walk segments and integrate.
    total_seconds = 0.0
    for i, (seg_start_tick, tempo) in enumerate(tempo_segments):
        seg_end_tick = tempo_segments[i + 1][0] if i + 1 < len(tempo_segments) else None

        if ticks <= seg_start_tick:
            break

        end = ticks if seg_end_tick is None else min(ticks, seg_end_tick)
        dticks = end - seg_start_tick
        if dticks > 0:
            seconds_per_tick = (tempo / 1_000_000.0) / ticks_per_beat
            total_seconds += dticks * seconds_per_tick

        if seg_end_tick is not None and ticks < seg_end_tick:
            break

    return total_seconds

def extract_notes(
    mid: mido.MidiFile,
    limit: Optional[int] = None,
    presets: Optional[Dict[Tuple[int, int], str]] = None
) -> List[NoteEvent]:
    """Extract note events from a MIDI file."""
    tpb = mid.ticks_per_beat
    tempo_segments = build_tempo_segments(mid)

    results: List[NoteEvent] = []
    note_index = 0

    # For durations, track active notes per
    # (track, channel, note) -> (start_ticks, start_seconds, velocity)
    active: Dict[Tuple[int, int, int], Tuple[int, float, int]] = {}

    for ti, track in enumerate(mid.tracks):
        abs_tick = 0
        programs: Dict[int, int] = {}
        bank_msb: Dict[int, int] = {}
        bank_lsb: Dict[int, int] = {}
        for msg in track:
            abs_tick += msg.time  # delta ticks -> absolute ticks

            if msg.type == "control_change":
                if msg.control == 0:
                    bank_msb[msg.channel] = msg.value
                elif msg.control == 32:
                    bank_lsb[msg.channel] = msg.value
            elif msg.type == "program_change":
                programs[msg.channel] = msg.program

            # Treat "note_on velocity=0" as note_off
            is_note_on = msg.type == "note_on" and msg.velocity > 0
            is_note_off = (msg.type == "note_off") or (msg.type == "note_on" and msg.velocity == 0)

            if is_note_on:
                start_sec = ticks_to_seconds(abs_tick, tpb, tempo_segments)
                key = (ti, msg.channel, msg.note)
                active[key] = (abs_tick, start_sec, msg.velocity)

                msb = bank_msb.get(msg.channel, 0)
                lsb = bank_lsb.get(msg.channel, 0)
                bank = msb
                prog = programs.get(msg.channel, 0)
                preset = presets.get((bank, prog)) if presets else None

                note_index += 1
                results.append(
                    NoteEvent(
                        index=note_index,
                        track=ti,
                        channel=msg.channel,
                        note=msg.note,
                        note_name=note_to_name(msg.note),
                        velocity=msg.velocity,
                        start_ticks=abs_tick,
                        start_seconds=start_sec,
                        bank_msb=msb,
                        bank_lsb=lsb,
                        bank=bank,
                        program=prog,
                        preset=preset,
                    )
                )

            elif is_note_off:
                key = (ti, msg.channel, msg.note)
                if key in active:
                    start_tick, start_sec, _ = active.pop(key)
                    end_sec = ticks_to_seconds(abs_tick, tpb, tempo_segments)
                    dur_ticks = abs_tick - start_tick
                    dur_sec = end_sec - start_sec

                    # attach duration to the *first matching* stored event among our results
                    # (same track/channel/note/start_tick)
                    for i in range(len(results) - 1, -1, -1):
                        ev = results[i]
                        if (ev.track == ti and ev.channel == msg.channel and
                                ev.note == msg.note and ev.start_ticks == start_tick):
                            results[i] = replace(
                                ev, duration_ticks=dur_ticks, duration_seconds=dur_sec
                            )
                            break

    # If the file has multiple tracks, the "first notes of the song" are usually by global time.
    # Sort by absolute time (ticks), then by track to stabilize ordering.
    results.sort(key=lambda e: (e.start_ticks, e.track, e.channel, e.note))
    if limit is not None:
        return results[:limit]
    return results

@contextlib.contextmanager
def open_output_file(name: Optional[Path]):
    """Open a file for writing, or use stdout if no name is provided."""
    if name:
        with open(name, "wt", encoding="utf-8") as fobj:
            yield fobj
    else:
        yield sys.stdout

def main() -> None:
    p = argparse.ArgumentParser(
        description="Print first N note-on events with absolute timestamps."
    )
    p.add_argument("midi_file", type=Path)
    p.add_argument("-n", "--num", type=int)
    p.add_argument("-o", "--output", type=Path, help="write to file instead of stdout")
    p.add_argument("-f", "--font", type=Path, help="soundfont to parse for preset names")
    args = p.parse_args()

    presets = parse_sf2_presets(args.font) if args.font else None
    mid = mido.MidiFile(str(args.midi_file))
    events = extract_notes(mid, args.num, presets)

    with open_output_file(args.output) as fobj:
        csvw = csv.writer(fobj)
        csvw.writerow((
            "idx",
            "t(msec)",
            "t(ticks)",
            "dur(msec)",
            "dur(ticks)",
            "trk",
            "ch",
            "bmsb",
            "blsb",
            "bank",
            "prog",
            "preset",
            "note",
            "name",
            "vel",
        ))
        for ev in events:
            csvw.writerow((
                ev.index,
                ev.start_msec,
                ev.start_ticks,
                ev.duration_msec,
                ev.duration_ticks,
                ev.track,
                ev.channel,
                ev.bank_msb,
                ev.bank_lsb,
                ev.bank,
                ev.program,
                ev.preset,
                ev.note,
                ev.note_name,
                ev.velocity,
            ))

if __name__ == "__main__":
    main()

# vim: set ts=4 sts=4 sw=4: