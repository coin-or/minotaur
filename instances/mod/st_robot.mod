#  NLP written by GAMS Convert at 01/12/18 13:43:34
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        9        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        9        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         25        9       16        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= -1, <= 1;
var x2 >= -1, <= 1;
var x3 >= -1, <= 1;
var x4 >= -1, <= 1;
var x5 >= -1, <= 1;
var x6 >= -1, <= 1;
var x7 >= -1, <= 1;
var x8 >= -1, <= 1;

minimize obj:  - 0;

subject to

e1: 0.004731*x1*x3 - 0.1238*x1 - 0.3578*x2*x3 - 0.001637*x2 - 0.9338*x4 + x7
     = 0.3571;

e2: 0.2238*x1*x3 + 0.2638*x1 + 0.7623*x2*x3 - 0.07745*x2 - 0.6734*x4 - x7
     = 0.6022;

e3: x6*x8 + 0.3578*x1 + 0.004731*x2 = 0;

e4:  - 0.7623*x1 + 0.2238*x2 = -0.3461;

e5: x1^2 + x2^2 = 1;

e6: x3^2 + x4^2 = 1;

e7: x5^2 + x6^2 = 1;

e8: x7^2 + x8^2 = 1;
