#! /usr/bin/env python

import sys

data = open(sys.argv[1],"r")

print sys.argv[1][:-4]
out0 = open(sys.argv[1][:-4]+"_0.txt","w")
out1 = open(sys.argv[1][:-4]+"_1.txt","w")

for line in data:
    if line.startswith("#"):  continue
    columns = line.split()
    if int(columns[0]) == 0: out0.write("{0:s} {1:s}\n".format(columns[1],columns[2]))
    if int(columns[0]) == 1: out1.write("{0:s} {1:s}\n".format(columns[1],columns[2]))

out0.close()
out1.close()
