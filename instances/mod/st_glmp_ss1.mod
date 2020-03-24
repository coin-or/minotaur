#  NLP written by GAMS Convert at 01/12/18 13:43:28
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         12        4        0        8        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        6        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         28       26        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 13;
var x2 >= 0, <= 13;
var x4;
var x5;
var x6;

minimize obj: x5*x6 + x4;

subject to

e1:    x1 - 2*x2 <= 100;

e2:  - 3*x1 - 4*x2 <= -12;

e3:  - x1 - x2 <= 100;

e4:  - x1 + 4*x2 <= 100;

e5:  - x1 + 2*x2 <= 18;

e6:    3*x1 + 4*x2 <= 100;

e7:    x1 + x2 <= 13;

e8:    x1 - 4*x2 <= 8;

e9:    x1 - x4 = 0;

e10:    x1 - x2 - x5 = -10;

e11:    x1 + x2 - x6 = 6;
