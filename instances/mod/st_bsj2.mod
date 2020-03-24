#  NLP written by GAMS Convert at 01/12/18 13:43:20
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         19       16        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;

minimize obj: 2*x1 - x1^2 - x2^2 - x3^2 + 2*x3;

subject to

e1:    x1 + x2 - x3 <= 1;

e2:  - x1 + x2 - x3 <= -1;

e3:    12*x1 + 5*x2 + 12*x3 <= 34.8;

e4:    12*x1 + 12*x2 + 7*x3 <= 29.1;

e5:  - 6*x1 + x2 + x3 <= -4.1;
