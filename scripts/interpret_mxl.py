#!/usr/bin/env python3

"""
Extract information from a MusicXML file
"""

import argparse
import csv
import logging
import sys

import music21

logging.basicConfig(format="%(module)s:%(lineno)s: %(levelname)s: %(message)s",
                    level=logging.INFO)
logger = logging.getLogger(__name__)

def main():
  ap = argparse.ArgumentParser()
  ap.add_argument("score", help="path to score.xml")
  ap.add_argument("-t", "--tempo", type=int, default=120,
                  help="override tempo in BPM (default: %(default)s)")
  ap.add_argument("-o", "--output", metavar="FILE", help="write to %(metavar)s")
  ap.add_argument("-v", "--verbose", action="store_true", help="verbose output")
  args = ap.parse_args()

  if args.verbose:
    logger.setLevel(logging.DEBUG)

  score = music21.converter.parse(args.score)

  if args.tempo is not None:
    # Use overridden tempo
    tempo_bpm = args.tempo
  else:
    # Try to extract tempo from score, fallback to 120 BPM if missing or invalid
    try:
      tempo_elem = score.recurse().getElementsByClass(music21.tempo.MetronomeMark)[0]
      tempo_bpm = float(tempo_elem.number)
    except (IndexError, ValueError, AttributeError, TypeError):
      tempo_bpm = 120.0
      logger.warning("No valid tempo found, defaulting to 120 BPM")

  ticks_per_beat = 480

  parts = list(score.getElementsByClass(music21.stream.Part))
  if not parts:
    parts = [score]

  events = []

  for trk_idx, part in enumerate(parts):
    ch = 0
    inst = part.getInstrument()
    if hasattr(inst, 'midiChannel') and inst.midiChannel is not None:
      ch = inst.midiChannel

    # flatten() resolves measure offsets to absolute offsets
    for element in part.flatten().notes:
      offset = float(element.offset)
      dur = float(element.duration.quarterLength)

      t_ticks = int(offset * ticks_per_beat)
      dur_ticks = int(dur * ticks_per_beat)

      # 1 beat = 60 / tempo_bpm seconds = 60000 / tempo_bpm milliseconds
      ms_per_beat = 60000.0 / tempo_bpm
      t_msec = int(offset * ms_per_beat)
      dur_msec = int(dur * ms_per_beat)

      lyric = element.lyric if element.lyric else ""

      # Try to extract velocity
      try:
        vel = int(element.volume.velocity)
      except (AttributeError, ValueError, TypeError):
        vel = 64

      pitches = element.pitches
      for pitch in pitches:
        try:
          note_midi = pitch.midi
          note_name = pitch.nameWithOctave
        except AttributeError:
          continue

        events.append({
          "beat": offset,
          "t(msec)": t_msec,
          "t(ticks)": t_ticks,
          "dur(msec)": dur_msec,
          "dur(ticks)": dur_ticks,
          "trk": trk_idx,
          "ch": ch,
          "note": note_midi,
          "name": note_name,
          "vel": vel,
          "lyric": lyric
        })

  # Sort by absolute time (ticks), then by track, channel, note
  events.sort(key=lambda e: (e["t(ticks)"], e["trk"], e["ch"], e["note"]))

  for i, ev in enumerate(events):
    ev["idx"] = i + 1

  header = ["idx", "beat", "t(msec)", "t(ticks)", "dur(msec)", "dur(ticks)", "trk", "ch", "note", "name", "vel", "lyric"]

  if args.output:
    with open(args.output, "w", newline="", encoding="utf-8") as f:
      writer = csv.DictWriter(f, fieldnames=header)
      writer.writeheader()
      for ev in events:
        writer.writerow(ev)
  else:
    print(f"tempo: {tempo_bpm} bpm, ticks_per_beat: {ticks_per_beat}")
    print()
    print(" idx   beat   t(msec) t(ticks)  dur(msec) dur(ticks)  trk ch note name  vel lyric")
    print("---- ------   ------- --------  --------  ---------   --- -- ---- ----  --- -----")
    for ev in events:
      print(
        f"{ev['idx']:>4} {ev['beat']:>6.2f}   {ev['t(msec)']:>7d} {ev['t(ticks)']:>8}  "
        f"{ev['dur(msec)']:>8}  {ev['dur(ticks)']:>9}  "
        f"{ev['trk']:>3} {ev['ch']:>2} {ev['note']:>4} {ev['name']:>4}  {ev['vel']:>3} "
        f"{ev['lyric']}"
      )

if __name__ == "__main__":
  main()

# vim: set ts=2 sts=2 sw=2:
