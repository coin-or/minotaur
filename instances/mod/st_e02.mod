#  NLP written by GAMS Convert at 01/12/18 13:43:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        4        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          9        5        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 9.422;
var x2 >= 0, <= 5.9023;
var x3 >= 0, <= 267.417085245;

minimize obj:    x3;

subject to

e1: 30*x1 - 6*x1*x1 - x3 = -250;

e2: 20*x2 - 12*x2*x2 - x3 = -300;

e3: 0.5*(x1 + x2)^2 - x3 = -150;
