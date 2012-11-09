#!/usr/bin/env python

import random
import sys

# number of variables
NV = 10

# ML Cons
NMLCONS = 4

# max degree
MD = 6

ix = []
print NV
for i in xrange(NV):
    ix.append(i)
    r = random.random()
    if r < 0.5:
        print random.uniform(-3,3), " ", random.uniform(4,6)
    else:
        print random.uniform(1,4), " ", random.uniform(5,8)
print 0
print NMLCONS

print ix

for i in xrange(NMLCONS):
    ix2 = ix[:]
    for j in xrange(MD):
        t = random.choice(ix2)
        ix2.remove(t)
        print ix2
    
