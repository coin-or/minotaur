with open("nvs24.mod") as f:
    for line in f:
        if line.strip() == "subject to":
            break

    constraints = []
    start = False
    con = ''
    for line in f:
        if "e" in line:
            start = True
        if ';' in line:
            start = False
            con += line[4:-2]
            constraints.append(con)
            con = ''
        if start == True:
            con += line[4:-1]

class var:
    def __init__(self, var_id, lb, ub):
        self.var_id = var_id
        self.lb = lb
        self.ub = ub

class Term:
    def __init__(self, coeff, var1, exp1, var2=None, exp2=0):
        self.coeff = coeff
        self.var1 = var1
        self.var2 = var2
        self.exp1 = exp1
        self.exp2 = exp2
        self.lb = 0
        self.ub = 0

class cons:
    def __init__(self, cons_id, terms, rhs):
        self.cons_id = cons_id
        self.lb = 0
        self.ub = 0
        self.terms = terms
        self.rhs = rhs

def add(int1, int2):
    return [int1[0] + int2[0], int1[1] + int2[1]]

def mult(int1, int2):
    lb = min(int1[0]*int2[0], int1[0]*int2[1], int1[1]*int2[0], int1[1]*int2[1])
    ub = max(int1[0]*int2[0], int1[0]*int2[1], int1[1]*int2[0], int1[1]*int2[1])
    return [lb, ub]

def scalar_mult(s, int1):
    return mult([s,s], int1)

def square(int1):
    if int1[0] >= 0:
        return [int1[0]**2, int1[1]**2]
    elif int1[1] < 0:
        return [int1[1]**2, int1[0]**2]
    else:
        return [0, max(int1[0]**2, int1[1]**2)]

vars_ = []
for i in range(1,11):
    vars_.append(var(i,0,200))

i = 0
for constraint in constraints:
    if '(' in constraint:
        constraint = constraint.replace('(', '')
        constraint = constraint.replace(')', '')
    constraint = constraint.replace(' ', '')
    constraint = constraint.replace('=', '')
    terms = []
    term = ''
    lb = ''
    lb_flag = False
    for char in constraint:
        if char == '>':
            lb_flag = True
            continue
        if lb_flag:
            lb += char
            continue
        if char not in '-+':
            term += char
        else:
            if term != '':
                terms.append(term)
                term = char
            else:
                term = char
    constraints[i] = [terms, int(lb)]
    i += 1

for i in range(len(constraints)):
    con = []
    for term in constraints[i][0]:
        if term[0] == 'i':
            coeff = 1
            term = term[1:]
        else:
            coeff = int(term[:term.index('*')])
            term = term[term.index('*') + 2:]
        if '^' in term:
            exp1 = 2
            var1 = vars_[int(term[:term.index('^')]) - 1]
            con.append(Term(coeff, var1, exp1))
        else:
            exp1 = 1
            var1 = vars_[int(term[:term.index('*')]) - 1]
            exp2 = 1
            var2 = vars_[int(term[term.index('*') + 2:]) - 1]
            con.append(Term(coeff, var1, exp1, var2, exp2))
    constraints[i] = cons(i+1, con, constraints[i][1])

for constraint in constraints:
    for term in constraint.terms:
        if term.var2:
            interval = scalar_mult(term.coeff, mult([term.var1.lb, term.var1.ub], [term.var2.lb, term.var2.ub]))
        else:
            interval = scalar_mult(term.coeff, square([term.var1.lb, term.var1.ub]))
        term.lb = interval[0]
        term.ub = interval[1]
        interval = add([constraint.lb, constraint.ub], interval)
        constraint.lb = interval[0]
        constraint.ub = interval[1]
        if constraint.cons_id == 1:
            print(term.lb, term.ub)

for constraint in constraints:
    print(constraint.cons_id, constraint.lb, constraint.ub, constraint.rhs)
