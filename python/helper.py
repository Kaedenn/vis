#!/usr/bin/env python

import errno
import os
import random
import sys

def writeto(stream, string):
  try:
    stream.write(string)
  except IOError, e:
    if e.errno != errno.EAGAIN:
      raise

def writelnto(stream, string):
  try:
    stream.write(string + "\n")
  except IOError, e:
    if e.errno != errno.EAGAIN:
      raise

def debug(enable, string, *args):
  if enable:
    if args == ():
      writelnto(sys.stderr, string)
    else:
      writelnto(sys.stderr, string % args)

def rand(low, high):
  return random.random() * (high - low) + low


