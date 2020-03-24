#  NLP written by GAMS Convert at 01/12/18 13:43:31
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         16       13        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;

minimize obj: 1.25*x1 - 2.5*x1^2 - 5*x2^2 + 2.5*x2 - 7.5*x3^2 + 5*x3;

subject to

e1:    10*x1 + 0.2*x2 - 0.1*x3 <= 11;

e2:  - 0.3*x1 + 9*x2 + 0.2*x3 <= 8;

e3:  - 0.1*x1 + 0.4*x2 + 11*x3 <= 12;

e4:    6*x1 + 8*x2 + 9*x3 <= 18;
