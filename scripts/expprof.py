#!/usr/bin/env python
#

import getopt
import os
import re
import sys
import tempfile
import subprocess
from types import *
from   string import atoi, atof

INFTY = 1e50

class Options:
	gnuplot = False
	logbase = 2.0
	title   = ""
	maxlimit= 1e50
	xup     = 1e50
	xdown   = 0.0
	minpos  = 1e-6
	names   = []
	autolim_= 1

class Solver:
	fName = ""
	fCol  = 0
	name  = ""
	def __init__(self, fn, c):
		self.fName  = fn
		self.fCol   = c-1
		self.vals   = []
		self.ratios = []
		self.name   = self.fName

	def appendRatio(self, r):
		self.ratios.append(r)
	
	def readMetric(self, opts):
		ff=open(self.fName,'r')
		a=ff.read().split("\n")
		ff.close()
		aa = []
		maxlimit = opts.maxlimit
		minpos = opts.minpos
		for l in a:
			if (type(re.match("^[\s]*[%#]",l))  is NoneType and 
				type(re.match("^[\s]*$",l)) is NoneType):
				entries = l.split()
				if (len(entries)<=self.fCol):
					print "no column ",(self.fCol+1),
					print "in line ",l," in file ",
					print self.fName
					sys.exit(5)
				val =  atof(entries[self.fCol])
				if (val<0 or val > maxlimit):
					self.vals.append(-1.0)
				elif (val < minpos):
					self.vals.append(minpos)
				else:
					self.vals.append(atof(entries[self.fCol]))


def calcRat(ss, s, i):
	minval = INFTY
	if (s.vals[i]<0):
		rat = -1.0
		return rat
	for j in ss:
		if (j<>s):
			if (j.vals[i]>=0):
				minval = min(minval, j.vals[i])
	if (minval >= INFTY):
		rat = 0.0
	else:
		rat = s.vals[i]/minval
	return rat


def calcRatios(ss):
	for i in range(len(ss[0].vals)):
		for s in ss:
			rat = calcRat(ss, s, i) 
			s.appendRatio(rat)

def changeMinus(solvers, opts):
	maxr = -1.0
	# get the highest ratio and bump -1s to more than the highest ratio
	for s in solvers:
		for r in s.ratios:
			if (r >= 0 and maxr < r):
				maxr = r
	if (opts.xup>maxr):
		opts.autolim_ = maxr+1
	else:
		opts.autolim_ = opts.xup
	for s in solvers:
		for i in range(len(s.ratios)):
			if (s.ratios[i] < 0):
				s.ratios[i] = opts.autolim_+1000

def chkFile(f):
	if not os.access(f, os.R_OK):
		print "file ", f, " can not be read!"
		sys.exit(1)


def dispTable(ss, rat_only):
	j = 0
	n = len(ss[0].vals)
	for i in range(len(ss[0].vals)):
		j += 1
		print "%4d/%d"%(j,n),
		if (rat_only):
			for s in ss:
				print "%8.2f"%s.vals[i],
		for s in ss:
			print "%8.2f"%s.ratios[i],
		print ""


def gnuplot(solvers, opts):
	# first write data file
	df = tempfile.NamedTemporaryFile(delete=False)
	writeTable(solvers,df)
	df.close()
	proc = subprocess.Popen("gnuplot",stdin=subprocess.PIPE)
	proc.stdin.write("set terminal postscript color solid\n")
	proc.stdin.write("set title \"%s\"\n"%opts.title)
	proc.stdin.write("set key right bottom\n")
	proc.stdin.write("set xlabel \"Ratio to Fastest\"\n")
	proc.stdin.write("set ylabel \"Fraction of Instances\"\n")
	proc.stdin.write("set log x\n")
	proc.stdin.write("set logscale x %f\n"%opts.logbase)
	proc.stdin.write("set yrange [0:1]\n")
	proc.stdin.write("set ytics 0.1\n")
	proc.stdin.write("set arrow from 1,0 to 1,1 nohead lc rgb \'black\'\n")
	if (opts.xdown>1e-10):
		proc.stdin.write("set xrange [%f:%f]\n"%(opts.xdown,opts.autolim_))
	else:
		proc.stdin.write("set xrange [:%f]\n"%opts.autolim_)
	i = 2
	for s in solvers:
		if (s==solvers[0]):
			proc.stdin.write("plot ")
		else:
			proc.stdin.write(", ")
		proc.stdin.write("\"%s\" using %d:1 with steps lw 4 title \"%s\""%(df.name,i,s.name))
		i+=1

	proc.stdin.write("\nexit\n")
	proc.wait()

def sortRatios(ss):
	for s in ss:
		s.ratios.sort()


def readArgs(s, opts):
	if (len(s)<2):
		usage()
		sys.exit(2)
	try:
		argopts, args = getopt.getopt(s[1:], "hgl:m:M:t:X:x:",
				             ["names="])
	except getopt.GetoptError, err:
		print str(err) # print "option -a not recognized"
		usage()
		sys.exit(2)
	for o, a in argopts:
		if o in ("-h"):
			usage()
			sys.exit()
		elif o == "-g":
			opts.gnuplot = True
		elif o == "-l":
			opts.logbase = atof(a)
		elif o == "-m":
			opts.minpos = atof(a)
			if (opts.minpos<=0):
				opts.minpos = 1e-6
		elif o == "-M":
			opts.maxlimit = atof(a)
		elif o == "-t":
			opts.title = a
		elif o == "-X":
			opts.xup = atof(a)
		elif o == "-x":
			opts.xdown = atof(a)
		elif o in ("--names"):
			opts.names = re.split(',', a)
		else:
			assert False, "unhandled option"
	num_solvers = 0
	i = 0
	solvers = []
	while(i<len(args)):
		fn = args[i]
		chkFile(fn)
		i+=1
		if (i>=len(args)):
			print "column number required for file ",fn
			sys.exit(3)
		fcols = sepColNums(args[i])
		i+=1
		num_solvers += len(fcols)
		for c in fcols:
			solvers.append(Solver(fn, c))
		
	i = 0
	for n in opts.names:
		solvers[i].name=n
		i += 1

	return solvers



def sepColNums(s):
	l0=re.split(',', s)
	fcols = []
	for ss in l0:
		i = int(ss)
		if (i<1):
			print "column number should be greater than 0!"
			print l0
			sys.exit(4)
		fcols.append(i)
	return fcols


def usage():
	print "usage:"
	print "expprof.py [-g] [-l val] [-t val] [-m val] [-M val] [-x val]",\
	"[-X val] [--names comma-separated-names] file1 c1,c2,c3,... file2",\
	"c4,c5,c6,... file3 c7 file1 c8"

	print "where file1, file2, ... filen are the names of files from",\
		"which columns are read. Columns c1,c2,c3 are read from",\
		"file1, columns c4,c5,c6 are read from file2 and so forth.",\
		"Files can be repeated."
	print ""
	print "Options:"
	print "-g	Use gnuplot to generate eps output. The user should",\
		"redirect output to an eps file. Otherwise, a table is",\
		"printed."
	print "-l val	Use log with base 'val' for the x-axis, default 2"
	print "-t val	Use val as the title of the plot. Should be",\
		"enclosed in double quotes."
	print "-m val	If any value is less than 'val', it is taken as val.",\
		"It is used to avoid division by zero. Use -m 1 if",\
		"profiling integer values like nodes."
	print "-M val	If any value is greater than or equal to val, it is",\
		"taken as failure (e.g. time limit reached)."
	print "-x val	Minimum x-coordinate. Everything to left is pruned."
	print "-X val	Maximum x-coordinate. Everything to right is pruned."
	print "--names	list of names separated by commas that will be",\
		"assigned to each curve. Default is to use filename."
	print ""
	print "e.g. expprof.py -g -l 4 -t \"my title\" file1 2 file2",\
		"2,3 > out.eps",\


def writeTable(ss,df):
	j = 0
	n = len(ss[0].vals)
	for i in range(len(ss[0].vals)):
		j += 1
		df.write("%7.5f"%(1.0*j/n))
		for s in ss:
			df.write("%8.2f"%s.ratios[i])
		df.write("\n")

##################
# main 
##################

opts = Options()
solvers = readArgs(sys.argv, opts) 
for s in solvers:
	s.readMetric(opts)
	if (len(solvers[0].vals) <> len(s.vals)):
		print "number of data points for solver",s.fName, (s.fCol+1),
		print "different from solver",solvers[0].fName,
		print (solvers[0].fCol+1)
		sys.exit(6)
	

calcRatios(solvers)
#dispTable(solvers, 1)
changeMinus(solvers, opts)
sortRatios(solvers)
if (opts.gnuplot==True):
	gnuplot(solvers, opts)
else:
	dispTable(solvers, 0)



