#  NLP written by GAMS Convert at 01/12/18 13:43:33
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        5        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        6        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         27       22        5        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;

minimize obj: 10*x1 - 0.34*x1*x1 - 0.28*x1*x2 + 10*x2 - 0.22*x1*x3 + 10*x3 - 
    0.24*x1*x4 + 10*x4 - 0.51*x1*x5 + 10*x5 - 0.28*x2*x1 - 0.34*x2*x2 - 0.23*x2
    *x3 - 0.24*x2*x4 - 0.45*x2*x5 - 0.22*x3*x1 - 0.23*x3*x2 - 0.35*x3*x3 - 0.22
    *x3*x4 - 0.34*x3*x5 - 0.24*x4*x1 - 0.24*x4*x2 - 0.22*x4*x3 - 0.2*x4*x4 - 
    0.38*x4*x5 - 0.51*x5*x1 - 0.45*x5*x2 - 0.34*x5*x3 - 0.38*x5*x4 - 0.99*x5*x5
    ;

subject to

e1:    x1 + x2 + 2*x3 + x4 + x5 >= 10;

e2:    2*x1 + 3*x2 + x5 >= 8;

e3:    x2 + 4*x3 - x4 + 2*x5 >= 12;

e4:    8*x1 - x2 - x3 + 6*x4 >= 20;

e5:  - 2*x1 - x2 - 3*x3 - x4 - x5 >= -30;
