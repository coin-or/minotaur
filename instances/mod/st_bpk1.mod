#  NLP written by GAMS Convert at 01/12/18 13:43:19
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        1        0        6        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         17       13        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;

minimize obj: x1 - x1*x3 - x3 + x1*x4 + x2*x3 - x2 - x2*x4;

subject to

e1:    x1 + 4*x2 <= 8;

e2:    4*x1 + x2 <= 12;

e3:    3*x1 + 4*x2 <= 12;

e4:    2*x3 + x4 <= 8;

e5:    x3 + 2*x4 <= 8;

e6:    x3 + x4 <= 5;
