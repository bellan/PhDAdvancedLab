#! /usr/bin/env python

import sys

basename = sys.argv[1]

datach0 = open(basename+"_ch000.dat","r")
datach1 = open(basename+"_ch002.dat","r")

out = open(basename+".txt","w")

for line in datach0:
    if line.startswith("#") or line.startswith("H"):  continue
    out.write("0 "+ line)

for line in datach1:
    if line.startswith("#") or line.startswith("H"):  continue
    out.write("1 "+ line)
    
