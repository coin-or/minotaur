#  MINLP written by GAMS Convert at 01/12/18 13:43:24
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        1        0        6        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        3        2        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         17       15        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var x3 >= 0, <= 6;
var x4 >= 0, <= 5;

minimize obj: 4*x3 - x3^2 - x4^2 + 2*x4 + 2*b1 + 2*b2 + 2;

subject to

e1:  - x3 + 3*x4 <= 5;

e2:    2*x3 - x4 <= 5;

e3:  - 2*x3 + x4 <= 0;

e4:    x3 - 3*x4 <= 0;

e5:  - 6*b1 + x3 <= 0;

e6:  - 5*b2 + x4 <= 0;
