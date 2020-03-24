#  NLP written by GAMS Convert at 01/12/18 13:43:23
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13       11        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 8;
var x2 >= 0, <= 4;

minimize obj: (-x1^2) - 4*x2^2;

subject to

e1:    x1 + x2 <= 10;

e2:    x1 + 5*x2 <= 22;

e3:  - 3*x1 + 2*x2 <= 2;

e4:  - x1 - 4*x2 <= -4;

e5:    x1 - 2*x2 <= 4;
