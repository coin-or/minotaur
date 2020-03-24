#  NLP written by GAMS Convert at 01/12/18 13:43:32
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         31       25        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;

minimize obj: 6*x1 - 3*x1^2 - 2.5*x2^2 + 5*x2 - 2*x3^2 + 4*x3 - 1.5*x4^2 + 3*x4
     - x5^2 + 2*x5 - 0.5*x6^2 + x6;

subject to

e1:    6*x1 + x2 + 9*x4 + 3*x5 + 5*x6 <= 96;

e2:    x1 + 7*x3 + 6*x4 + 2*x5 + 2*x6 <= 72;

e3:    5*x1 + 4*x2 + x3 + 3*x4 + 8*x5 <= 84;

e4:    9*x1 + x2 + 2*x4 + 7*x5 + 6*x6 <= 100;

e5:    2*x1 + 6*x4 + 3*x5 + 9*x6 <= 80;
