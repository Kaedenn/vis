#!/usr/bin/env python

import sys

labels = [['START-TRACK']]
last_label = 0
for l in sys.stdin:
    if '\t' in l:
        b, e, l = l.strip().split(None, 2)
        begin = float(b)
        end = float(e)
        labels.append([int(begin*1000), int(end*1000), l])
labels.append(['END-TRACK'])

for label in labels:
    print ", ".join(str(i) for i in label)
