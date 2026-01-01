#!/usr/bin/env python3

import csv
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import argparse

import mido

DEFAULT_TEMPO = 500_000  # 120 BPM in microseconds/beat

@dataclass(frozen=True)
class NoteEvent:
    index: int
    track: int
    channel: int
    note: int
    note_name: str
    velocity: int
    start_ticks: int
    start_seconds: float
    duration_ticks: Optional[int] = None
    duration_seconds: Optional[float] = None

    @property
    def start_msec(self):
        return int(self.start_seconds * 1000)

    @property
    def duration_msec(self):
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

def ticks_to_seconds(ticks: int, ticks_per_beat: int, tempo_segments: List[Tuple[int, int]]) -> float:
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

def extract_notes(mid: mido.MidiFile, limit: int = 32) -> List[NoteEvent]:
    tpb = mid.ticks_per_beat
    tempo_segments = build_tempo_segments(mid)

    results: List[NoteEvent] = []
    note_index = 0

    # For durations, track active notes per (track, channel, note) -> (start_ticks, start_seconds)
    active: Dict[Tuple[int, int, int], Tuple[int, float, int]] = {}

    for ti, track in enumerate(mid.tracks):
        abs_tick = 0
        for msg in track:
            abs_tick += msg.time  # delta ticks -> absolute ticks

            # Treat "note_on velocity=0" as note_off
            is_note_on = msg.type == "note_on" and msg.velocity > 0
            is_note_off = (msg.type == "note_off") or (msg.type == "note_on" and msg.velocity == 0)

            if is_note_on:
                start_sec = ticks_to_seconds(abs_tick, tpb, tempo_segments)
                key = (ti, msg.channel, msg.note)
                active[key] = (abs_tick, start_sec, msg.velocity)

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
                    )
                )

                if limit is not None:
                    if len(results) >= limit:
                        # We still might want to compute durations for these first notes,
                        # so we do NOT break early here; we'll fill durations as note-offs appear.
                        pass

            elif is_note_off:
                key = (ti, msg.channel, msg.note)
                if key in active:
                    start_tick, start_sec, start_vel = active.pop(key)
                    end_sec = ticks_to_seconds(abs_tick, tpb, tempo_segments)
                    dur_ticks = abs_tick - start_tick
                    dur_sec = end_sec - start_sec

                    # attach duration to the *first matching* stored event among our results
                    # (same track/channel/note/start_tick)
                    for i in range(len(results) - 1, -1, -1):
                        ev = results[i]
                        if ev.track == ti and ev.channel == msg.channel and ev.note == msg.note and ev.start_ticks == start_tick:
                            results[i] = NoteEvent(
                                **{**ev.__dict__, "duration_ticks": dur_ticks, "duration_seconds": dur_sec}
                            )
                            break

            # If we already have enough results and all their durations are filled (optional),
            # we could stop. For simplicity, stop once we have 'limit' note-ons and we're past them a bit.
            if limit is not None:
                if len(results) >= limit and abs_tick > results[-1].start_ticks:
                    # If you don't care about durations, you can break earlier.
                    # We'll keep scanning a little to fill some durations naturally.
                    pass

        if limit is not None:
            if len(results) >= limit:
                # continue scanning other tracks only if you need cross-track earliest notes.
                # Many MIDI files store melody in one track; but not always.
                # We'll keep going to collect earliest per-track note-ons; you can change this behavior.
                pass

    # If the file has multiple tracks, the "first notes of the song" are usually by global time.
    # Sort by absolute time (ticks), then by track to stabilize ordering.
    results.sort(key=lambda e: (e.start_ticks, e.track, e.channel, e.note))
    if limit is not None:
        return results[:limit]
    return results

def main() -> None:
    p = argparse.ArgumentParser(description="Print first N note-on events with absolute timestamps.")
    p.add_argument("midi_file", type=Path)
    p.add_argument("-n", "--num", type=int)
    p.add_argument("-o", "--output", type=Path, help="write to csv")
    args = p.parse_args()

    mid = mido.MidiFile(str(args.midi_file))
    events = extract_notes(mid, args.num)

    if args.output:
        with open(args.output, "wt") as fobj:
            csvw = csv.writer(fobj)
            csvw.writerow(("idx", "t(msec)", "t(ticks)", "dur(msec)", "dur(ticks)", "trk", "ch", "note", "name", "vel"))
            for ev in events:
                csvw.writerow((ev.index, ev.start_msec, ev.start_ticks, ev.duration_msec, ev.duration_ticks,
                    ev.track, ev.channel, ev.note, ev.note_name, ev.velocity))
        print(f"wrote {len(events)} notes to {args.output}")
    else:
        print(f"ticks_per_beat: {mid.ticks_per_beat}")
        print()
        print(" idx  t(msec) t(ticks)  dur(msec) dur(ticks)  trk ch note name  vel")
        print("----  ------- --------  --------  ---------   --- -- ---- ----  ---")
        for ev in events:
            dur_s = f"{ev.duration_msec:d}" if ev.duration_seconds is not None else "-"
            dur_t = f"{ev.duration_ticks}" if ev.duration_ticks is not None else "-"
            print(
                f"{ev.index:>4}  {ev.start_msec:>7d} {ev.start_ticks:>8}  "
                f"{dur_s:>8}  {dur_t:>9}  "
                f"{ev.track:>3} {ev.channel:>2} {ev.note:>4} {ev.note_name:>4}  {ev.velocity:>3}"
            )


if __name__ == "__main__":
    main()

