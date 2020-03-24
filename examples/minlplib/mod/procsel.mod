#  MINLP written by GAMS Convert at 01/12/18 13:41:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        5        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11        8        3        0        0        0        0        0
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
var x7 >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;

minimize obj:    1.8*x1 + 1.8*x2 + x3 + 1.2*x4 + 7*x5 - 11*x7 + 3.5*b8 + b9
     + 1.5*b10;

subject to

e1:  - 0.9*x6 + x7 = 0;

e2: exp(x3) - x1 = 1;

e3: exp(0.833333333333333*x4) - x2 = 1;

e4:  - x3 - x4 - x5 + x6 = 0;

e5:    x7 - 2*b8 <= 0;

e6:    x3 - 4*b9 <= 0;

e7:    x4 - 5*b10 <= 0;
