import os
import sys
import re

def print_usage():
    sys.stdout.write("usage : python report_outputs.py <path/to/dir/with/output/files>")

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

outputs = os.listdir(sys.argv[1])
sys.stdout.write("Instance,Status,Time,Nodes,Ub,Lb,Gap\n")
for out in outputs:
    sys.stdout.write(out[:-4] + ",")
    lines = open(os.path.join(sys.argv[1],out)).read().split('\n')
    find,status = find_string(lines, "mntr-glob: status of branch-and-bound:", '')
    if (find<0):
        sys.stdout.write("ERR,NF,NF,NF,NF,NF\n")
        continue
    else:
        if "Reached time" in status:
            status = "TLIM"
        elif "Optimal" in status:
            status = "OPT"
        sys.stdout.write(status + ",")

    find,sol = find_float(lines, "mntr-glob: time used =", 1e20, '')
    if (find<0 or sol>=1e20):
        sys.stdout.write("\nTIME NOT FOUND\n")
        sys.exit(0)
    else:
        sys.stdout.write(str(sol) + ",")

    find,sol = find_float(lines, "BranchAndBound: nodes processed =", 1e20, '')
    if (find<0 or sol>=1e20):
        sys.stdout.write("\nNODES NOT FOUND\n")
        sys.exit(0)
    else:
        sys.stdout.write(str(sol) + ",")

    find,sol = find_float(lines, "mntr-glob: best solution value =", 1e20, 'inf')
    if (find<0):
        sys.stdout.write("\nUB NOT FOUND\n")
        sys.exit(0)
    else:
        sys.stdout.write(str(sol) + ",")

    if status == "OPT":
        sys.stdout.write(str(sol) + ",0\n")
        continue

    find,sol = find_float(lines, "mntr-glob: best bound estimate from remaining nodes =", 1e20, 'inf')
    if (find<0):
        sys.stdout.write("\nLB NOT FOUND\n")
        sys.exit(0)
    else:
        sys.stdout.write(str(sol) + ",")

    find,sol = find_float(lines, "mntr-glob: gap =", 1e20, 'inf')
    if (find<0):
        sys.stdout.write("\nGAP NOT FOUND\n")
        sys.exit(0)
    else:
        sys.stdout.write(str(sol) + "\n")

