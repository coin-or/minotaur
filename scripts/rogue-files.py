#!/usr/bin/python
#
# Test Instances located in the text file!
# Usage:
# ---------------------------------------------------------------------
#	python rogue-files --help

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
usage = "usage: python rogue-files.py -b <path to the binary>"
usage += " -s <path to the source directory> "
usage += " [-r recursive] "
																

print "For usage help, try: python rogue-files.py --help"
parser = optparse.OptionParser(usage)
parser.add_option("-s", "--solutions", 
									help="Relative path to solution-csv file (default ../src/base/)",
									dest="src_dir",default="../src/")
parser.add_option("-r", "--recursive", action="store_true",
									help="Recursively search through sub-directories (default False)",
									dest="recursive",default=True)
									
(options, args) = parser.parse_args()

# Options from the shell
SRC_DIR = os.path.expandvars(options.src_dir)
RECURSIVE = bool(options.recursive)

# Make sure the path ends in a '/'
if options.src_dir[-1] != '/':
	options.src_dir += '/'

if not os.path.exists(str(SRC_DIR)):
	print "Source directory " + str(SRC_DIR) +  " does not exist"
	print "Try python rogue-files.py --help for more options"
	sys.exit()

# ---------------------------------------------------------------------
# 								Analyze code
# ---------------------------------------------------------------------

# Class to store attributes of a file quality
class Quality:

	def __init__(self,filename):
		
		self.filename = filename
		self.rogueLines = 0

	# Function: Find the number of rogue lines
	def setRogueLines(self):
	
		f=open(self.filename,'r')
		lines = f.readlines()
		f.close()

		# Find the number of lines > 80
		for line in lines:
			if len(line) > 80:
				self.rogueLines += 1
				

qualityDict = {}

# Print the report
def prettyPrint():

	output = "%30s :"%"Filename"
	output += "%6s"%" Lines with > 80 ch"
	output += '\n'
	output += "----------------------------------------------------------------"
	output += '\n'

	# Sort by rogue lines
	items = [(key, qualityDict[key].rogueLines) for key in qualityDict.keys()]
	items.sort(key=lambda tup: tup[1])
	keys = map(lambda item: item[0], items)
	
	# Go through the dictionary, sorted by good-bad code
	for key in keys:
	
		baseFileName = qualityDict[key].filename.split('/')[-1]
		if (qualityDict[key].rogueLines>0):
			output += "%30s :"%baseFileName
			output += "%6s "%qualityDict[key].rogueLines
			output += '\n'
	
	print output


# Parse through all the cpp files (recursively)
files = []
if RECURSIVE == True:
	for root, dirs, fnames in os.walk(SRC_DIR):
		for file in fnames:
			if file.endswith('.cpp'):
				files.append(os.path.join(root, file))
else:
	files = glob.glob(SRC_DIR+'*.cpp')
			
# Quality testing on each file		
for file in files:
	qfile = Quality(file)
	qfile.setRogueLines()
	qualityDict[file] = qfile



# ---------------------------------------------------------------------
#  Print the report
# ---------------------------------------------------------------------
print "Code quality review"
print "----------------------------------------------------------------"
prettyPrint()
