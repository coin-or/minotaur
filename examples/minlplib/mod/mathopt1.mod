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


var x1 := 8, >= -10, <= 20;
var x2 := -14, >= -15, <= 20;

minimize obj: 10*(x1^2 - x2)^2 + (-1 + x1)^2;

subject to

e2: x1 - x1*x2 = 0;

e3:    3*x1 + 4*x2 <= 25;
