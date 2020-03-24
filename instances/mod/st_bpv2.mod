#  NLP written by GAMS Convert at 01/12/18 13:43:20
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        4        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         14       11        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 4;
var x2 >= 0, <= 4;
var x3 >= 0, <= 5;
var x4 >= 0, <= 5;

minimize obj: x2*x3 + x2 + x3 - x1*x3;

subject to

e1:  - 4*x1 - x2 >= -12;

e2:    3*x1 - x2 >= -1;

e3:    4*x3 - x4 <= 12;

e4:  - x3 - x4 >= -8;

e5:    4*x3 - x4 >= -1;
