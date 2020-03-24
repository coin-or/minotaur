#  NLP written by GAMS Convert at 01/12/18 13:43:33
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        9        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;

minimize obj: 2*x1 - 2*x1*x1 + 2*x1*x2 + 3*x2 - 2*x2*x2;

subject to

e1:  - x1 + x2 <= 1;

e2:    x1 - x2 <= 1;

e3:  - x1 + 2*x2 <= 3;

e4:    2*x1 - x2 <= 3;
