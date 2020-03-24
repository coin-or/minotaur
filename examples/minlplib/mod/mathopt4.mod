#  NLP written by GAMS Convert at 01/12/18 13:34:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        2        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        3        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= -10, <= 10;
var x2 >= -10, <= 10;

minimize obj: (2*x1^2 - x2^2)^2 + (x2 - 6*x1^2)^2;

subject to

e2: x1 - (100*sin(2*x1 + 3*x2) + 10*x2) = 0;

e3:    x1 + x2 <= 2;
