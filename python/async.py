#!/usr/bin/env python

import errno, fcntl, os

def initialize(handle):
  fd = handle.fileno()
  fcntl.fcntl(fd, fcntl.F_SETFL,
              os.O_NONBLOCK | fcntl.fcntl(fd, fcntl.F_GETFL))
  return handle

def read(handle):
  try:
    return handle.read()
  except IOError, e:
    if e.errno != errno.EAGAIN:
      raise
    return ""

