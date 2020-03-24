#!/usr/bin/python


import array
import os
import commands
import time
import math
import sys
import string
import fileinput
import re
from types import *


# ---------------------------------------------------------------------------
# IMPORTANT PARAMETERS
# ---------------------------------------------------------------------------
INFTY=1e10
ETOL=0.01
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------



def find_float(arr0,st0,fl0):
	for line in arr0:
		find = re.search(st0,line)
		if (find >= 0):
			t = re.search('-*[0-9]*\.*[0-9]+',line)
			if type(t) is not NoneType:
				fl0 = float(t.group())
				return 1,fl0
	return -1,fl0

def find_int(arr0,st0,in0):
	in0=0
	for line in arr0:
		find = re.search(st0,line)
		if (find >= 0):
			in0 = int(re.search('-*[0-9]+$',line).group())
			return 1,in0
	return -1,in0

def getbest(ubfile):
	ublist  = open(ubfile,'r')
	ublines = ublist.read().splitlines()
	bestub  = {}
	for iter in ublines:
		ubline = iter.split()
		f = INFTY
		try:
			f = float(ubline[1])
		except ValueError:
			f = INFTY
		bestub[ubline[0]] = f 
	ublist.close()
	return bestub

def print_usage():
	print "usage: python report_size.py -d <path to dir with output files>",
	print "-l <file containing instance names>",
	print "[-t time-limit]",
	print "[-b file containing best known-bounds]"

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

# parse command line
if (len(sys.argv)<4):
	print_usage()
	sys.exit(0)

i = 1
OUTPUT_DIR="./"
INST_LIST=""
has_user_dir = 0
has_user_list = 0
has_user_best = 0
error_on_inf = 0
time_limit = INFTY
ubfile = ""
while(i<len(sys.argv)):
	if (sys.argv[i]=='-d'):
		has_user_dir = 1
		if (i==len(sys.argv)-1):
			print "Missing argument to '-d'"
			print_usage()
			sys.exit(0)
		else:
			OUTPUT_DIR=sys.argv[i+1]
			if (os.path.exists(OUTPUT_DIR)):
				print "### Reading from directory:", OUTPUT_DIR
				i = i+1
			else:
				print "the specified directory %s is not accessible"%OUTPUT_DIR
				print_usage()
				sys.exit(0)
	elif (sys.argv[i]=='-l'):
		has_user_list = 1
		if (i==len(sys.argv)-1):
			print "Missing argument to '-l'"
			print_usage()
			sys.exit(0)
		else:
			INST_LIST=sys.argv[i+1]
			i = i+1
	elif (sys.argv[i]=='-t'):
		if (i==len(sys.argv)-1):
			print "Missing argument to '-t'"
			print_usage()
			sys.exit(0)
		else:
			time_limit=float(sys.argv[i+1])
			i = i+1
	elif (sys.argv[i]=='-b'):
		if (i==len(sys.argv)-1):
			print "Missing argument to '-b'"
			print_usage()
			sys.exit(0)
		else:
			has_user_best = 1
			ubfile=sys.argv[i+1]
			i = i+1
	else:
		print "invalid option: %s"%sys.argv[i]
		print_usage()
		sys.exit(0)
	i = i+1
		
if (has_user_dir<1 or has_user_list<1):
	print_usage()
	sys.exit(0)

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

# grep the log files and print the report

flist=open(INST_LIST,'r')
a=flist.read().split()
flist.close()
a.sort()
errors = []
unsolved = []
bestub = {}
#print "### Instance set:", INST_SET
print "###%19s"%"instance","%8s"%"time","%10s"%"nodes","%12s"%"ub","%12s"%"lb","%8s"%"gap",
if (1==has_user_best):
	print "%15s"%"bestub",

print "%8s"%"status",
print ''

if (1==has_user_best):
	bestub = getbest(ubfile)

for instance in a:
	print "%22s"%instance,

	is_ok = "OK"
	filename=OUTPUT_DIR+"/"+instance+".out"
	try:
		fil=open(filename,'r')
	except IOError:
		errors.append(instance)
		print "%8s"%"-1","%10s"%"NF","%12s"%"NF","%12s"%"NF","%8s"%"NF",
		if (1==has_user_best):
			print  "%15.3f"%bestub[instance],
		print  "%8s"%"NO-OUT"
		continue

	whole_file=fil.read().split("\n")
	fil.close()

	time = INFTY
	find,time=find_float(whole_file,'mntr-glob: time used =',time)
	if (find<0 or time >= INFTY):
		print  "%8s"%"-1",
		is_ok = "NO-OUT"
	else:
		print  "%8.2f"%time,
		if (time > time_limit):
			unsolved.append(instance) 
			is_ok = "TI-LIM"

	nodes = INFTY
	find,nodes=find_int(whole_file,'BranchAndBound: nodes processed =',nodes)
	if (find<0 or nodes >= INFTY):
		print  "%10s"%"-1",
		is_ok = "NO-OUT"
	else:
		print  "%10d"%nodes,

	ub = INFTY
	find,ub=find_float(whole_file,'best solution value = ',ub)
	if (find<0 or ub >= INFTY):
		print  "%12s"%"NF",
		if (time < time_limit):
			is_ok = "INF"
	else:
		print  "%12.3f"%ub,

	lb = INFTY
	find,lb=find_float(whole_file,'best bound estimate from remaining nodes = ',lb)
	if (find<0):
		lb = INFTY
	
	gap = INFTY
	if (lb >= INFTY and ub >= INFTY):
		gap = INFTY
	elif (lb >= INFTY):
		gap = 0.0
		lb = ub
	else:
		gap = abs(ub - lb)/(abs(ub) + 0.0001)*100

	if (lb >= INFTY):
		print  "%12s"%"NF",
	else:
		print  "%12.3f"%lb,

	if (gap >= INFTY):
		print  "%8s"%"NF",
	else:
		print  "%8.2f"%gap,

	if (1==has_user_best):
		print  "%15.3f"%bestub[instance],
		if (lb > bestub[instance]+ ETOL or ub < bestub[instance] - ETOL):
			is_ok="WR-SOL"

	print  "%8s"%is_ok,
	if (is_ok=="NO-OUT" or is_ok=="WR-SOL" or is_ok=="INF"):
		errors.append(instance)
	print ''

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

## print summary

print "### Number of instances = ", len(a)
print "### Number of errors    = ", len(errors)
print "### Number of unsolved  = ", len(unsolved)

print "###"
print "### errors: " 
for iter in errors:
	print "###    ", iter

print "###"
print "### unsolved: " 
for iter in unsolved:
	print "###    ", iter


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

