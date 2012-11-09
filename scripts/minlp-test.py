#!/usr/bin/python
#
# Test Instances located in the text file!
# Usage:
# ---------------------------------------------------------------------
#	python minlp-test --help

# Terminal colors
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

import os
import glob
import commands
import re
import sys

# Not sure if these are there
try:
	import optparse
except:
	print "Optparse module missing: see http://docs.python.org/library/optparse.html"
	
try:
	from multiprocessing import Pool
except:
	print "Multiprocessing module missing: see http://docs.python.org/library/multiprocessing.html"

# ---------------------------------------------------------------------
# Options for the script
# ---------------------------------------------------------------------
usage = "usage: python minlp-test.py -b <path to the binary>"
usage += " -i <path to the instance directory> "
usage += " -s <path to the solutions.csv file> "
usage += " [-t time-limit] \n"
usage += " [-j number of jobs] "													
																

print "For usage help, try: python minlp-test.py --help"
parser = optparse.OptionParser(usage)
parser.add_option("-b", "--binary", help="Binary to test (put options in quotes)",
									dest="binary", default="../build/bin/bnb")
parser.add_option("-t", "--timelimit", 
									help="Time limit for each instance (default=20secs)", 
									dest="time_limit", default=20)
parser.add_option("-j", "--jobs", 
									help="Number of parellel processes (default=1)",
									dest="jobs", default=1)
parser.add_option("-s", "--solutions", 
									help="Relative path to solution-csv file (default convex-easy)",
									dest="solution_file",default="../../../instances/solutions/convex-easy.csv")
parser.add_option("-i", "--instance_dir", 
									help="Relative path to the instance directory (default convex/nl)",
									dest="instance_dir", default="../../../instances/convex/nl/")

(options, args) = parser.parse_args()

# Options from the shell
BINARY = options.binary
JOBS = int(options.jobs)
TIME_LIMIT = options.time_limit
SOLUTION_FILE = os.path.expandvars(options.solution_file)
# Make sure the path ends in a '/'
if options.instance_dir[-1] != '/':
	options.instance_dir += '/'

INSTANCE_DIR=os.path.expandvars(options.instance_dir)
zTol = 1E-5

# ---------------------------------------------------------------------
# Error checking for the tests
# ---------------------------------------------------------------------

BIN_DIR=os.path.expandvars(options.binary)
if not os.path.exists(str(BIN_DIR)):
	print "Binary " + BINARY +  " does not exist"
	print "Provide the right path to the binary"
	print "Try python minlp-test.py --help for more options"
	sys.exit()

if not os.path.exists(str(INSTANCE_DIR)):
	print "Instance directory " + str(INSTANCE_DIR) +  " does not exist"
	print "Provide the right path to the where the instances are"
	print "Try python minlp-test.py --help for more options"
	sys.exit()

if not os.path.exists(str(SOLUTION_FILE)):
	print "Solutions file " + str(SOLUTION_FILE) +  " does not exist"
	print "Provide the right path to the solutions"
	print "Try python minlp-test.py --help for more options"
	sys.exit()
	
# ---------------------------------------------------------------------
# These are the names of the files in the testSet
# Answers form (http://wiki.mcs.anl.gov/leyffer/index.php/MacMINLP)
testSet={}

# Put the test sets and their solutions as a csv file

f = open(SOLUTION_FILE,'r')
for row in f.readlines():
	row = row.split(',')
	instance = row[0]
	solution = float(row[1])

	testSet[instance]=solution
f.close()

# ---------------------------------------------------------------------
# Testing function
# Input  : 
#					observed	 : Observed value from Minataur
#					listed : Listed valie in the csv file
# Output : 
#					status : True or False if correct or wrong
# ---------------------------------------------------------------------
def check_solution(observed, listed):

	# No solution (infeasible)
	if observed == "":
		return False
	
	observed = float(observed)
	
	# Correct solution
	if abs( (abs(observed - listed) / abs(listed + zTol))) < zTol:
		return True
	
	return False

# ---------------------------------------------------------------------
# Testing function
# Input  : 
#					iid	 : Instance ID
# Output : 
#					status : Returns the status i.e (OK, ERR, TO)
# ---------------------------------------------------------------------
class KeyboardInterruptError(Exception): pass
def run_test(iid):
	
	try:
		# List of instances
		instances = map(lambda test: str(INSTANCE_DIR+test), testSet.keys())
		file = instances[iid]
		
		# This is the script file
		script = BINARY+" "+file+options
		try:
			output = commands.getoutput(script)
		except KeyboardInterrupt:
			raise KeyboardInterruptError()
		
		solution = ""
		
		# The solutions are located from the log files	
		for lines in output.split('\n'):		
			locator = "best solution value"
			if locator in lines:
				solution = lines.split('=')[1]


		# Output log usually has the name of the instance, solution and status						
		instance = file.split('/')[-1]
		outputLog = instance[0:10] + '\t'
		outputLog += str(solution) + '\t'
				
		# Check if the values are within zero tolerance
		if check_solution(solution, testSet[instance]):
			
			status = "OK"
			outputLog += "OK"
					
		# Don't waste too much time on a test
		elif "Reached time limit" in output:
			status = "TO"
			outputLog += bcolors.WARNING
			outputLog += "TO"
			outputLog += bcolors.ENDC
		else: 
			status = "ERR"
			outputLog += bcolors.FAIL
			outputLog += "ERR (should be "+str(testSet[instance])+")"
			outputLog += bcolors.ENDC
			
		print outputLog
		return status

	except KeyboardInterrupt:
		raise KeyboardInterruptError()
	
# ---------------------------------------------------------------------
# Compling the results for all the Tests
# ---------------------------------------------------------------------

# Log all the instances
numOk = 0
numErr = 0
numTO = 0

# Some hard coded instances
# testSet['synthes1.nl'] = 6.00976
# testSet['synthes2.nl'] = 73.0353
# testSet['synthes3.nl'] = 68.0097
# testSet['optprloc.nl'] = -8.06414
# Print log header

print "-------------------------------------------------------"
print "Testing instances located at "+str(INSTANCE_DIR)
print "Binary    : "+str(BIN_DIR)+BINARY
print "Solutions : "+str(SOLUTION_FILE)
print "-------------------------------------------------------"


# Run only till 20 seconds
options = " -bnb_time_limit="+str(TIME_LIMIT)


# Loop through all the test instances
# ---------------------------------------------------------------------
instances = map(lambda test: str(INSTANCE_DIR+test), testSet.keys())
pool = Pool(JOBS)
		
try:
	statusList = pool.map(run_test,range(len(instances)))
except KeyboardInterrupt:
	print "You cancelled the program!"
	pool.terminate()
	sys.exit(1)
				
	
for status in statusList:
	if "OK" in status:
		numOk += 1
	if "ERR" in status:
		numErr += 1
	if "TO" in status:
		numTO += 1



# ---------------------------------------------------------------------
# Print a summary report
# ---------------------------------------------------------------------

# Give a warning if something has failed	
if numErr > 0:
	print "\n\n"
	print "WARNING : Some test cases have failed"

# Print a status repory
print "-------------------------------------------------------"
print " Results "
print "-------------------------------------------------------"
print " Total Cases : "+str(numOk+numErr+numTO)
print " Sucess      : "+str(numOk)
print " Failures    : "+str(numErr)
print " TimeOuts    : "+str(numTO)
print "-------------------------------------------------------"
