#  NLP written by GAMS Convert at 01/12/18 13:43:28
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        3        0        6        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21       19        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= -10, <= 1;
var x2 >= -10, <= 10;
var x3;
var x4;

minimize obj: x3*x4;

subject to

e1:    2*x1 + x2 <= 2;

e2:    x1 + x2 <= 2;

e3:  - 4*x1 + x2 <= 12;

e4:  - 2*x1 - x2 <= 6;

e5:  - x1 - 2*x2 <= 6;

e6:    x1 - x2 <= 3;

e8:    x1 + x2 - x3 = 0;

e9:    x1 - x2 - x4 = 0;
