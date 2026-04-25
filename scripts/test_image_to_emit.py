#!/usr/bin/env python3

"""
Perform some crude tests against image_to_emit.py
"""

import argparse
import logging
import os
import sys

import image_to_emit
from image_to_emit import *

def main():
  ap = argparse.ArgumentParser()
  ag = ap.add_argument_group("logging")
  mg = ag.add_mutually_exclusive_group()
  ag.add_argument("-v", "--verbose", action="store_true", help="verbose output")
  ag.add_argument("-t", "--trace", action="store_true", help="verbose output")
  args = ap.parse_args()
  if args.verbose:
    logger.setLevel(logging.DEBUG)
  elif args.trace:
    logger.setLevel(TRACE)

  results = []
  # Test HSTRIP:
  # x=1 y=1: stride_x=100 stride_y=2
  # x=1 y=2: stride_x=100 stride_y=2  (flatten to 1, 1, 100, 4)
  # x=1 y=3: stride_x=50 stride_y=2
  # x=51 y=3: stride_x=50 stride_y=2
  # x=51 y=4: stride_x=50 stride_y=2 (flatten to 51, 3, 50, 4)
  # x=1 y=5: stride_x=100 stride_y=2
  test_entries = (
    (1,  1, 100, 2), # at 1, 1  width=100 height=2
    (1,  2, 100, 2), # at 1, 2  width=100 height=2 (merge)
    (1,  3,  50, 2), # at 1, 3  width=50  height=2
    (51, 3,  50, 2), # at 51,3  width=50  height=2
    (51, 4,  50, 2), # at 51,4  width=50  height=2
    (1,  5, 100, 2), # at 1, 5  width=100 height=2
    (1,  6, 100, 2), # at 1, 6  width=100 height=2
  )

  for idx, entry in enumerate(test_entries):
    logger.info("Entry %d: %s", idx+1, entry)

  for stride in flatten_strides(test_entries, optimizer=Optimizer.HSTRIP):
    results.append(stride)

  image_to_emit.logger.info("Flattened %d strides to %d (HSTRIP)",
      len(test_entries), len(results))
  for idx, result in enumerate(results):
    image_to_emit.logger.info("Result %d: %s", idx+1, result)

if __name__ == "__main__":
  main()

# vim: set ts=2 sts=2 sw=2:
