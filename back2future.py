#! /usr/bin/env python

import sys, numpy

data = open(sys.argv[1],"r")
ch = int(sys.argv[2])

previous = -1.
previousline = ''

for line in data:
    if line.startswith("#") or line.startswith("H"):  continue
    columns = line.split()
    if not int(columns[0]) == ch: continue
    if (numpy.int64(columns[1])) < previous :
        print "I found an anomaly, from {0} back to {1}".format(previous, columns[1])
        print previousline, line
    previous = numpy.int64(columns[1])
    previousline = line
