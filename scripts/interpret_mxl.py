#!/usr/bin/env python3

"""

"""

import argparse
import logging
import os
import sys

import music21

logging.basicConfig(format="%(module)s:%(lineno)s: %(levelname)s: %(message)s",
                    level=logging.INFO)
logger = logging.getLogger(__name__)

def main():
  ap = argparse.ArgumentParser()
  ap.add_argument("score", help="path to score.xml")
  ap.add_argument("-v", "--verbose", action="store_true", help="verbose output")
  args = ap.parse_args()
  if args.verbose:
    logger.setLevel(logging.DEBUG)

  score = music21.converter.parse(args.score)
  try:
    tempo_elem = score.recurse().getElementsByClass(music21.tempo.MetronomeMark)[0]
    tempo_bpm = tempo_elem.number
  except IndexError:
    temp_bpm = 90

  for note in score.recurse().notes:
    pitch = note.pitches
    duration = note.duration.quarterLength
    lyric = note.lyric
    print(pitch, duration, lyric)

if __name__ == "__main__":
  main()

# vim: set ts=2 sts=2 sw=2:
