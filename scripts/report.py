#!/usr/bin/python

# search for "Select columns to print"  to select columns for printing.



#########################STARTS HERE##########################################################################
# Insertion of list of all the libraries to be  imported for running the program is done here ################
#########################STARTS HERE##########################################################################



import array
import os
#import commands
import time
import math
import sys
import string
import fileinput
import re

#########################ENDS HERE############################################################################
# Insertion of list of all the libraries to be  imported for running the program is done here ################
#########################ENDS HERE############################################################################

#########################STARTS HERE############################################################################
# Declaration of all the global variables is done here #########################################################
#########################STARTS HERE############################################################################

# ---------------------------------------------------------------------------
# IMPORTANT PARAMETERS 
# ---------------------------------------------------------------------------
INST_LIST=""
OUTPUT_DIR=""
BEST_FILE=""
INFTY=1e12
ABSTOL=0.01
RELTOL=0.001

#########################ENDS HERE##############################################################################
# Declaration of all the global variables is done here #########################################################
#########################ENDS HERE##############################################################################
        
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
#########################STARTS HERE##########################################################################################################
# Function named 'find_str' has been defined here with two arguments namely 'arr0,st0' and is returning a value at the end ################### #########################STARTS HERE##########################################################################################################

def find_str(arr0,st0):
	for line in arr0:
		find = re.search(st0,line)
		if (find !=None):
			return 1
	return 0

#########################ENDS HERE############################################################################################################
# Function named 'find_str' has been defined here with two arguments namely 'arr0,st0' and is returning a variable at the end ################### #########################ENDS HERE############################################################################################################

########starts here################################################################################
########Funtion named 'find_float' has been defined here with three arguments and is returning two  variables at the end ###########
####################starts here ##################

def find_float(arr0,st0,fl0):
	for line in arr0:
		find = re.search(st0,line)
		if (find !=None):
			try:
				fl0 = float(re.search('-*[0-9]+\.[0-9]+',line).group())
				return 1,fl0
			except AttributeError:
				return -1,0.0
	return -1,fl0

##########Funtion named 'find_float' has been defined here with three arguments namely 'arr0,st0,fl0' and is ruturning with

def find_float_1(arr0,st0,fl0):
	st0 = st0+" "
	for line in arr0:
		find = re.search(st0,line)
		if (find >= 0):
			find = re.search('-*[0-9]+\.[0-9]+',line)
			if (find>=0):
				l = re.findall('-*[0-9]+\.[0-9]+',line)
				if (len(l)<1):
					return -1,fl0
				else:
					fl0 = float(l[len(l)-1])
					return 1,fl0
			else:
				return -1,fl0
	return -1,fl0

def find_float_e(arr0,st0,fl0):
	for line in arr0:
		find = re.search(st0,line)
		if (find >= 0):
			fl0 = float(re.search('-*[0-9]+\.[0-9]*e*\+*[0-9]*',line).group())
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

def getInstList(fname):
	ins_list = []
	f = open(fname,'r')
	#CHANGE THIS CODE TO READ SOLU FILE
	for line in f:
            if len(line.split(None, 2)) == 0:
                continue
            ins_list.append(line.split(None, 2)[1])
	f.close()
	return ins_list


        
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class Col:
    title = ""
    fmt = "%8s"
    fval = "NF"
    width = 20
    
    def __init__(self, title, width, fmt):
        self.title = title
        self.fmt = fmt
        self.width = width
    
    def extract(self):
        return self.writeFail()
        
    def write(self, val):
        print(self.fmt%val, end="")
        return self.fmt%val
        
    def writeFail(self):
        s = "%%%ds"%self.width
        print(s%"NF", end="")
        return s%"NF"
        
    def writeTitle(self):
        s = "%%%ds"%self.width
        print(s%self.title, end="")
        return s%self.title
        
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
    
class Cuts(Col):
    val = INFTY
    def extract(self):
        val = INFTY 
        find, val = find_int(outfile, "QPDProcessor: cuts added", val)
        if(find<0 or val>=INFTY):
            self.val = INFTY
            return self.writeFail()
        else:
            self.val = val
            return self.write(self.val)


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class BKnown(Col):

	val = INFTY
	def extract(self):
		self.val = INFTY
		find = -1
		#ASSUME IT IS A SOLU FILE.
		lines = open(BEST_FILE).read().split('\n')
		for l in lines:
			n_v_pair = l.split()
			if (len(n_v_pair)>2 and n_v_pair[1]==instance):
				self.val = float(n_v_pair[2])
				break
		return self.write(self.val)


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class EChk(Col):

	val = "NO-OUT"
	def extract(self):
		if (claims_opt.val=="ERR"):
			self.val = "NO-OUT"
			errors.append(instance)
		elif (BEST_FILE != "" and 'bnd' in globals() and 'sol' in globals()):
			if (claims_opt.val=="INF" and bknown.val<INFTY):
				self.val = "W-FEAS"
				errors.append(instance)
			elif (claims_opt.val=="UNB" and abs(bknown.val)<INFTY):
				self.val = "W-UNB"
				errors.append(instance)
			elif (osen.val*(bnd.val-bknown.val) > ABSTOL and
					osen.val*(bnd.val-bknown.val)/(abs(bknown.val)+1e-6) > RELTOL):
				self.val = "W-BND"
				errors.append(instance)
			elif (bknown.val-sol.val) > ABSTOL and (bknown.val-sol.val)/(abs(bknown.val)+1e-6) > RELTOL:
				self.val = "NEWSOL"
				errors.append(instance)
			elif (abs(bknown.val) >= INFTY and abs(sol.val) < INFTY): 
				self.val = "NEWSOL"
				errors.append(instance)
			else:
				self.val = "OK"
		else:
			self.val = "OK"
		return self.write(self.val)


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class Bnd(Col):
    def extract(self):
        self.val = INFTY
        val = INFTY
        find,val=find_float(outfile,"best bound estimate from remaining nodes = ",val)
        if (find<0 or val >= INFTY):
            self.writeFail()
            self.val = INFTY
            return self.val
        else:
            self.write(val)
            self.val = val
            return self.val
                


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class Nodes(Col):

	val = INFTY
	def extract(self):
		val = INFTY
		find,val=find_int(outfile,'branch-and-bound: nodes processed =', val)
		if (find<0 or val >= INFTY):
			self.val = INFTY
			self.writeFail()
		else:
			self.val = val
			self.write(self.val)

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class ObjSen(Col):
	val = 0
	def extract(self):
		val = 0
		find = find_str(outfile,'objective sense: maximize')
		if (find>=0):
			self.val = -1
			return max

		find = find_str(outfile,'objective sense: minimize')
		if (find>=0):
			self.val = 1
		return min

	def writeTitle(self):
		s = ""

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class Status(Col):

	val = "ERR"
	def extract(self):
		self.val = "ERR"
		find = find_str(outfile,'Optimal solution found')
		if (find>0):
			self.val = "OPT"
		else:
			find = find_str(outfile,'Detected infeasibility')
			if (find>0):
				self.val = "INF"
			else:
				find = find_str(outfile,'Relaxation Unbounded')
				if (find>0):
					self.val = "UNB"
				else:
					find = find_str(outfile,'Reached time limit')
					if (find>0):
						self.val = "LIM"
						unsolved.append(instance)
		return self.write(self.val)


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class Sol(Col):
    val = INFTY
    def extract(self):
        self.val = INFTY
        val = INFTY
        find,val=find_float(outfile,"best solution value = ",val)
        if (find<0 or val >= INFTY):
            self.writeFail()
            self.val = INFTY
            return self.val
        else:
            self.write(val)
            self.val = val
            return self.val

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
class WallTime(Col):

	def extract(self):
		val = INFTY
		find,val=find_float(outfile,"time used \(s\)",val)
		if (find<0 or val >= INFTY):
			val = -1.0
		self.write(val)
		return 

def print_usage():
    print("usage: python report.py -l <file containing instance names>")
    print("-d <path to dir> [-b <file conataining best solution>]")

if (len(sys.argv)<2):
    print_usage()
    sys.exit(0)
    
i = 1
while(i < len(sys.argv)):
    if (sys.argv[i]=='-d'):
        if(i==len(sys.argv)-1):
            print("Missing atgument to '-d'")
            print_usage()
            sys.exit(0)
        else:
            OUTPUT_DIR=sys.argv[i+1]
            if (os.path.exists(OUTPUT_DIR)):
                print("### Reading output files from directory:", OUTPUT_DIR)
                i = i+1
            else:
                print("the specified directory %s is not accessible"%OUTPUT_DIR)
                print_usage()
                sys.exit(0)
    elif(sys.argv[i]=='-l'):
        if(i==len(sys.argv)-1):
            print("Missing argument to '-l")
            print_usage()
            sys.exit(0)
        else:
            INST_LIST=sys.argv[i+1]
            print("### Instances listed in :", INST_LIST)
            i = i+1
    elif(sys.argv[i]=='-b'):
        if(i==len(sys.argv)-1):
            print("Missing argument to '-b'")
            print_usage()
            sys.exit(0)
        else:
            BEST_FILE=sys.argv[i+1]
            print("### Best solution listed in :", BEST_FILE)
            i = i+1
    else:
        print("Invalid option: %s"%sys.argv[i])
        print_usage()
        sys.exit(0)
    i = i+1
    
if(OUTPUT_DIR==""):
    print_usage()
    sys.exit(0)
    
if(INST_LIST==""):
    print_usage()
    sys.exit(0)


instCol=Col("######## instance", 30, "%30s")

othercols=[]
errors=[]
unsolved=[]

ins_list=getInstList(INST_LIST)

# Mandatory columns ######################################

othercols.append(WallTime("time", 10, "%10.2f"))
claims_opt = Status("opt", 4, "%4s")
othercols.append(claims_opt)
osen = ObjSen("sen", 3, "%3s")        

# Select columns to print #######

#othercols.append(Cuts("cuts", 8, "%8d"))
#othercols.append(Nodes("nodes", 8, "%8d"))
sol = Sol("sol", 18, "%18.4f")
othercols.append(sol)
bnd = Bnd("bnd", 18, "%18.4f")
othercols.append(bnd)

if (BEST_FILE != ""):
	bknown = BKnown("best-known", 18, "%18.4f")
	othercols.append(bknown)


othercols.append(EChk("err", 8, "%8s"))

# End of 'columns to print' #######

instCol.writeTitle()
for c in othercols:
	c.writeTitle()
print()

csv_data = []

for instance in ins_list:
    list1 = []
    ins = instCol.write(instance)
    list1.append(ins)
    
    filename = OUTPUT_DIR + '/' + instance + ".out"
    try:
        fil = open(filename, 'r')
        outfile = fil.read().split('\n')
        fil.close()
    except IOError:
        errors.append(instance)
        for c in othercols:
            c.writeFail()
        
        print()
        continue
    
    for c in othercols:
        c.extract()
    print()
    csv_data.append(list1)



fields = ["instances","time taken to solve","status","Sol", "bnd","Echk","BEST_FILE" ]

rows = csv_data


print("### Number of instances = ", len(ins_list))
print("### Number of errors    = ", len(errors))
print("### Number of unsolved  = ", len(unsolved))

print("###")
print("### errors: " )
for iter in errors:
	print ("###    ", iter)

print("###")
print("### unsolved: " )
for iter in unsolved:
	print("###    ", iter)
