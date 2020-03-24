#  NLP written by GAMS Convert at 01/12/18 13:43:20
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        1        6        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         19       15        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 10000;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;

minimize obj: 0.25*x1*x1 - x1 + 0.25*x2*x2 - x2 + 0.25*x3*x3 - x3 + 0.25*x4*x4
     - x4;

subject to

e1:    x1 + x2 >= 1;

e2:    2*x3 + 2*x4 >= 4;

e3:    x1 + x3 >= 3;

e4:    x2 + x4 >= 4;

e5:  - x2 - 2*x3 - 3*x4 >= -8;

e6:  - 3*x2 - x3 - 2*x4 >= -10;
