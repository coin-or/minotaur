#  NLP written by GAMS Convert at 01/12/18 13:43:22
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        2        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         12       10        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 3;
var x2 >= 0, <= 4;
var x3 >= 0, <= 2;
var x4 >= 0, <= 1;

minimize obj: x1**0.6 + x2**0.6 - 6*x1 - 4*x3 + 3*x4;

subject to

e1:  - 3*x1 + x2 - 3*x3 = 0;

e2:    x1 + 2*x3 <= 4;

e3:    x2 + 2*x4 <= 4;
