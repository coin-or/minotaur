#  NLP written by GAMS Convert at 01/12/18 13:43:32
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        2        0        0        0        0        0
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


var x1 >= 0;
var x2 >= 0;

minimize obj: 2*x1 - x1*x1 - x2*x2 + 4*x2;

subject to

e1:    x1 - 4*x2 >= -8;

e2:  - 3*x1 + x2 >= -9;
