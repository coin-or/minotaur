#  NLP written by GAMS Convert at 01/12/18 13:43:23
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        1        3        0        0        0        0
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


var x1 >= 0, <= 4;
var x2 := 1, >= 1, <= 5;

minimize obj: (5 + x1 - x2)*(-1 + x1 + x2) + x1;

subject to

e1:    2*x1 + 3*x2 >= 9;

e2:    3*x1 - x2 <= 8;

e3:  - x1 + 2*x2 <= 8;

e4:    x1 + 2*x2 <= 12;
