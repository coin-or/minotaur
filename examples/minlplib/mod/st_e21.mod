#  NLP written by GAMS Convert at 01/12/18 13:43:23
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        4        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21       18        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 3;
var x2 >= 0, <= 4;
var x3 >= 0, <= 4;
var x4 >= 0, <= 2;
var x5 >= 0, <= 2;
var x6 >= 0, <= 6;

minimize obj: x1**0.6 + x2**0.6 + x3**0.4 - 4*x3 + 2*x4 + 5*x5 - x6;

subject to

e1:  - 3*x1 + x2 - 3*x4 = 0;

e2:  - 2*x2 + x3 - 2*x5 = 0;

e3:    4*x4 - x6 = 0;

e4:    x1 + 2*x4 <= 4;

e5:    x2 + x5 <= 4;

e6:    x3 + x6 <= 6;
