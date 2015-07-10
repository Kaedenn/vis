#!/usr/bin/env python

import sys

def process_fileobj(fileobj):
    labels = []
    labels.append(['START-TRACK'])
    labels.extend([process_line(l) for l in fileobj])
    labels.append(['END-TRACK'])
    return labels

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
    for label in labels:
        print ", ".join(str(l) for l in label)

