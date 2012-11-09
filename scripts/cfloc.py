#!/usr/bin/python
import sys
import os

def print_usage():
	print "usage: python cfloc.py name-of-file.cpp"


# starting at line pos, get the name of the next function.
def get_fun_name(pos):
	fun_name = "unknown function "
	while (pos < total_lines):
		line = whole_file[pos]
		if (len(line)>0):
			if(line[0] != ' ' and ord(line[0]) < 123 and ord(line[0]) > 64):
				words = line.split()
				if (len(words)>0 and words[0] != "using" and words[0] != "namespace"):
					fun_name = line
					break
		pos = pos + 1
	if (len(fun_name)>0):
		#remove everything after the opening bracket
		fun_name = fun_name[0:fun_name.find('(')]

	return fun_name

def get_fun_begin(pos):
	while (pos < total_lines):
		line = whole_file[pos]
		if (len(line)>0):
			if(line[0] == '{'):
				break
		pos = pos+1
	return pos


def get_fun_end(pos):
	while (pos < total_lines):
		line = whole_file[pos]
		if (len(line)>0):
			if(line[0] == '}'):
				break
		pos = pos+1
	return pos

def display_fun(ns_list):
	for i in ns_list:
		print "%70s :"%i[0],"%4d"%i[1]



#############################################################################
#############################################################################

if (len(sys.argv) != 2):
	print_usage()
	sys.exit(1)


filename = sys.argv[1]
if (not os.path.exists(filename)):
	print filename,"not found."
	sys.exit(1)


fil=open(filename,'r')
whole_file=fil.read().split("\n")
fil.close()

total_lines = len(whole_file)
#print "Total number of lines =", total_lines

rogueLines = 0
for lines in whole_file:
	if len(lines) > 80:
		rogueLines += 1

print "%70s :"%"Number of lines more than 80 chars","%4d"%rogueLines
print

pos = 0
fun_name = "unknown function "
ns_list = []
while (pos < total_lines):
	fun_name = get_fun_name(pos)
	pos1 = get_fun_begin(pos)
	pos2 = get_fun_end(pos1)
	s = pos2-pos1+1
	name_size = fun_name,s
	#print name_size
	ns_list.append(name_size)
	pos = pos2+1

ns_list.sort(key=lambda p: p[1])
display_fun(ns_list)
