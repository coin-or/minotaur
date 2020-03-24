#  NLP written by GAMS Convert at 01/12/18 13:43:32
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         10        1        0        9        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         28       26        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;

minimize obj: 15*x1 - x1^2 - x2^2 - 2*x2 + x3;

subject to

e1:  - 4*x1 - 3*x2 + 4*x3 <= 30;

e2:    4*x1 + 9*x2 - 2*x3 <= 114;

e3:    2*x2 - x3 <= 8;

e4:    2*x1 + 15*x2 - 8*x3 <= 64;

e5:    x2 <= 14;

e6:  - 4*x1 + 3*x2 - 2*x3 <= -18;

e7:    4*x1 - 9*x2 + 4*x3 <= -6;

e8:  - 6*x1 + 5*x2 - 4*x3 <= -40;

e9:    4*x1 - 9*x2 - 3*x3 <= -132;
