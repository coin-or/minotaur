#  NLP written by GAMS Convert at 01/12/18 13:43:34
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         13        1        0       12        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         79       73        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;

minimize obj: 0.5*x1*x2 - x1*x1 + 0.5*x2*x1 - x2*x2 + 0.5*x2*x3 + 0.5*x3*x2 - 
    x3*x3 + 0.5*x3*x4 + 0.5*x4*x3 - x4*x4 + 0.5*x4*x5 + 0.5*x5*x4 - x5*x5 + 0.5
    *x5*x6 + 0.5*x6*x5 - x6*x6;

subject to

e1:  - x1 - 2*x2 - 3*x3 - 4*x4 - 5*x5 - 6*x6 <= 0;

e2:  - 2*x1 - 3*x2 - 4*x3 - 5*x4 - 6*x5 - x6 <= 0;

e3:  - 3*x1 - 4*x2 - 5*x3 - 6*x4 - x5 - 2*x6 <= 0;

e4:  - 4*x1 - 5*x2 - 6*x3 - x4 - 2*x5 - 3*x6 <= 0;

e5:  - 5*x1 - 6*x2 - x3 - 2*x4 - 3*x5 - 4*x6 <= 0;

e6:  - 6*x1 - x2 - 2*x3 - 3*x4 - 4*x5 - 5*x6 <= 0;

e7:    x1 + 2*x2 + 3*x3 + 4*x4 + 5*x5 + 6*x6 <= 21;

e8:    2*x1 + 3*x2 + 4*x3 + 5*x4 + 6*x5 + x6 <= 21;

e9:    3*x1 + 4*x2 + 5*x3 + 6*x4 + x5 + 2*x6 <= 21;

e10:    4*x1 + 5*x2 + 6*x3 + x4 + 2*x5 + 3*x6 <= 21;

e11:    5*x1 + 6*x2 + x3 + 2*x4 + 3*x5 + 4*x6 <= 21;

e12:    6*x1 + x2 + 2*x3 + 3*x4 + 4*x5 + 5*x6 <= 21;
