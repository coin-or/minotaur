#  NLP written by GAMS Convert at 01/12/18 13:43:20
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13        9        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2;
var x3 >= -10000, <= 10000;
var x4 >= -10000, <= 10000;

minimize obj: 2*x1*x1 - 1.33333*x1 + 4*x2*x2 - 2.66667*x2 + 6*x3*x3 - 4*x3 + 
    0.5*x4*x4 - 10*x4;

subject to

e1:  - x1 - x2 - x3 - x4 <= -1;

e2:    x1 + x2 + x3 + x4 <= 1;
