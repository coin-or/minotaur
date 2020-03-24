#  NLP written by GAMS Convert at 01/12/18 13:43:31
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21       17        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;

minimize obj: 16*x1 - 4*x1^2 - 0.5*x2^2 + x2 - 2.5*x3^2 + 15*x3 - x4^2 + 8*x4;

subject to

e1:    x1 - x2 + 3*x3 - 2*x4 <= 6;

e2:  - x1 + 4*x2 + x3 - 2*x4 <= 7;

e3:    2*x1 + x2 + 2*x3 + x4 <= 29;

e4:    x1 - x2 + x3 + x4 <= 11;
