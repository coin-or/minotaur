#  NLP written by GAMS Convert at 01/12/18 13:43:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        4        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          9        6        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 12.5;
var x2 >= 0, <= 37.5;
var x3 >= 0, <= 50;

minimize obj:  - 0;

subject to

e1: x3*x3 - 0.000169*x2^3*x1 = 0;

e2:    x1 + x2 + x3 = 50;

e3:  - 3*x1 + x2 = 0;
