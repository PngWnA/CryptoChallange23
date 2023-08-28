#!/usr/bin/python3

import sys

f = open(sys.argv[1])

# read file
l = list(map(int, f.readlines()))

# calculate average
print(sum(l) / len(l), end='\n\n')

f.close()
