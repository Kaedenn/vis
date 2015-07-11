#!/usr/bin/env python

import sys

def process_fileobj(fileobj):
    return [process_line(l) for l in fileobj]

def process_line(line):
    if '\t' in line:
        b, e, l = line.strip().split(None, 2)
        begin = float(b)
        end = float(e)
        return [int(begin*1000), int(end*1000), l]
    return []

fileobjs = [sys.stdin]
if len(sys.argv) > 1:
    fileobjs = [open(arg) for arg in sys.argv[1:]]
for fileobj in fileobjs:
    labels = process_fileobj(fileobj)
    labels.sort(key = lambda l: l[-1])
    for label in labels:
        print "    %5d, %5d, -- %s" % (tuple(label))

