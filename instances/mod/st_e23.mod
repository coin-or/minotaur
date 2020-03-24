#  NLP written by GAMS Convert at 01/12/18 13:43:23
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        5        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 5;
var x2 >= 0, <= 5;

minimize obj: x1*x2 - x1 - x2;

subject to

e1:  - 6*x1 + 8*x2 <= 3;

e2:    3*x1 - x2 <= 3;
