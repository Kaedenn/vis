#!/usr/bin/env python

import sys

labels = [['START-TRACK']]
last_label = 0
for l in sys.stdin:
    if '\t' in l:
        b, e, l = l.split()
        begin = float(b)
        end = float(e)
        l = int(l)
        if l < last_label:
            labels.append(['END-TRACK', 'START-TRACK'])
        else:
            labels.append([int(begin*1000), int(end*1000)])
        last_label = l
labels.append(['END-TRACK'])

for label in labels:
    print ", ".join(str(i) for i in label)
