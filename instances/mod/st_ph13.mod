#  NLP written by GAMS Convert at 01/12/18 13:43:31
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         11        1        0       10        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         28       25        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;

minimize obj: x1 - 0.5*x1^2 - 0.5*x2^2 + x2 - 0.5*x3^2 + x3;

subject to

e1:    x1 <= 4;

e2:    x2 <= 4;

e3:    x3 <= 4;

e4:    2*x1 + 3*x2 + 4*x3 <= 35;

e5:    2*x1 + 3*x2 - 4*x3 <= 19;

e6:    2*x1 - 3*x2 + 4*x3 <= 23;

e7:  - 2*x1 + 3*x2 + 4*x3 <= 27;

e8:    2*x1 - 3*x2 - 4*x3 <= 7;

e9:  - 2*x1 - 3*x2 + 4*x3 <= 15;

e10:  - 2*x1 + 3*x2 - 4*x3 <= 11;
