import os
import sys
import re

def print_usage():
    sys.stdout.write("usage : python report_rough.py <path/to/dir/with/output/files>")

if len(sys.argv)<2:
    print_usage()
    sys.exit(0)

def find_float(arr0,st0,fl0,st1):
    for line in arr0:
        find = re.search(st0,line)
        if (find >= 0):
            if st1 != '':
                if st1 in line:
                    return 1,st1
            t = re.search('-*[0-9]*\.*[0-9]+',line)
            if type(t) is not type(None):
                fl0 = float(t.group())
                return 1,fl0
    return -1,fl0

def find_string(arr0,st0,fl0):
    for line in arr0:
        find = re.search(st0,line)
        if find >= 0:
            return 1, line[len(st0)+1:-1]
    return -1,fl0

def find_sum(arr0,st0,int0):
    fl0 = 0
    for line in arr0:
        find = re.search(st0,line)
        if (find >= 0):
            t = re.search('-*[0-9]*\.*[0-9]+',line)
            if type(t) is not type(None):
                fl0 += float(t.group())
                int0 = 1
    return int0,fl0

outputs = os.listdir(sys.argv[1])
sys.stdout.write("Instance,Number of quadratics,Total time in finding subgraphs,Total number of subgraphs found,Total time in checking convexity,Total number of convex subgraphs,Number of convex constraints,Problem identified as convex\n")
for out in outputs:
    sys.stdout.write(out[:-4] + ",")
    lines = open(os.path.join(sys.argv[1],out)).read().split('\n')
    find,num = find_float(lines, "Number of quadratics", 1e20, '')
    if (find<0 or num>=1e20):
        sys.stdout.write("NF,")
    else:
        sys.stdout.write(str(int(num)) + ",")

    find,time = find_sum(lines, "Time used in finding subgraphs", -1)
    if (find<0):
        sys.stdout.write("NF,")
    else:
        sys.stdout.write(str(time) + ",")

    find,num = find_sum(lines, "Number of subgraphs found for qf are", -1)
    if (find<0):
        sys.stdout.write("NF,")
    else:
        sys.stdout.write(str(int(num)) + ",")

    find,time = find_sum(lines, "Time used in checking convexity", -1)
    if (find<0):
        sys.stdout.write("NF,")
    else:
        sys.stdout.write(str(time) + ",")
    
    find,num = find_sum(lines, "Number of convex subgraphs", -1)
    if (find<0):
        sys.stdout.write("NF,")
    else:
        sys.stdout.write(str(int(num)) + ",")

    find,num = find_sum(lines, "Convex constraint", -1)
    if (find<0):
        sys.stdout.write("NF,")
    else:
        sys.stdout.write(str(int(num)) + ",")

    find,num = find_string(lines, "The problem is Convex and Local solver can be used instead of glob", 1e20)
    if (find<0):
        sys.stdout.write("No\n")
    else:
        sys.stdout.write("Yes\n")

