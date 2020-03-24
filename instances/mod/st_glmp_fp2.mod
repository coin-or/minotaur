#  NLP written by GAMS Convert at 01/12/18 13:43:28
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         10        3        2        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         23       21        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 5;
var x2 >= 0, <= 10;
var x3;
var x4;

minimize obj: x3*x4;

subject to

e1:    2*x1 + x2 <= 14;

e2:    x1 + x2 <= 10;

e3:    1.44*x1 + x2 >= 4.89;

e4:  - 1.58*x1 + x2 <= 5.65;

e5:  - 1.03*x1 + x2 <= 5.93;

e6:    x1 + 2*x2 >= 6;

e7:    x1 - x2 <= 3;

e9:    x1 + x2 - x3 = 0;

e10:    x1 - x2 - x4 = -7;
