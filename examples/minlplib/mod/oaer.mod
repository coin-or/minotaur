#  MINLP written by GAMS Convert at 01/12/18 13:37:02
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        4        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         10        7        3        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         26       24        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;

minimize obj:    1.8*x1 + 1.8*x2 + 7*x3 + x4 + 1.2*x5 - 11*x6 + 3.5*b7 + b8
     + 1.5*b9;

subject to

e2: -log(1 + x1) + x4 = 0;

e3: -1.2*log(1 + x2) + x5 = 0;

e4:  - 0.9*x3 - 0.9*x4 - 0.9*x5 + x6 = 0;

e5:    x6 - b7 <= 0;

e6:    x4 - 1.111111*b8 <= 0;

e7:    x5 - 1.111111*b9 <= 0;

e8:    b8 + b9 <= 1;
