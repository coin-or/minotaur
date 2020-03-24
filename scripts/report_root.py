import sys
import os
import re

if len(sys.argv) < 3:
    print("USAGE: python report_root.py path/to/dir/of/output/files/ filename")

def find_float(arr0,st0,fl0):
    for line in arr0:
        find = re.search(st0,line)
        if (find >= 0):
            t = re.search('-*[0-9]*\.*[0-9]+',line)
            if type(t) is not type(None):
                fl0 = float(t.group())
                return 1,fl0
    return -1,fl0

outputs = os.listdir(sys.argv[1])
cgs = []
qfs = []
for out in outputs:
    if out == "summary.csv":
        continue
    if out[-6:] == "cg.out":
        cgs.append(out)
    else:
        print(out)
        assert(out[-6:] == "qf.out")
        qfs.append(out)

cg_new = list(cgs)
qf_new = list(qfs)

for cg in cg_new:
    if cg[:-6] + "qf.out" not in qfs:
        print(cg[:-7], "not in qf")
        cgs.remove(cg)

for qf in qf_new:
    if qf[:-6] + "cg.out" not in cgs:
        print(qf[:-7], "not in cg")
        qfs.remove(qf)

cgs.sort()
qfs.sort()
assert(len(cgs) == len(qfs))

with open(sys.argv[2], "w") as fw:
    fw.write("Instance,CG,QF\n")
    for i in range(len(cgs)):
        fw.write(cgs[i][:-7] + ",")
        find,sol = find_float(open(os.path.join(sys.argv[1],cgs[i])).read().split('\n'),
                               "mntr-glob: best bound estimate from remaining nodes",
                               1e20)
        if (find<0 or sol>=1e20):
            fw.write("NF,")
        else:
            fw.write(str(sol) + ",")
        find,sol = find_float(open(os.path.join(sys.argv[1],qfs[i])).read().split('\n'),
                               "mntr-glob: best bound estimate from remaining nodes",
                               1e20)
        if (find<0 or sol>=1e20):
            fw.write("NF\n")
        else:
            fw.write(str(sol) + "\n")
