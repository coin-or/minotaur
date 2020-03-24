#  MINLP written by GAMS Convert at 01/12/18 13:43:54
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         24        3        0       21        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         18       10        8        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         91       79       12        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 2;
var x2 >= 0, <= 2;
var x3 >= 0, <= 1;
var x4 >= 0, <= 2;
var x5 >= 0, <= 2;
var x6 >= 0, <= 2;
var x7 >= 0, <= 2;
var x8 >= 0, <= 1;
var x9 >= 0, <= 3;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var b13 binary >= 0, <= 1;
var b14 binary >= 0, <= 1;
var b15 binary >= 0, <= 1;
var b16 binary >= 0, <= 1;
var b17 binary >= 0, <= 1;

minimize obj: exp(x1) - 10*x1 + exp(0.833333*x2) - 15*x2 - 65*log(1 + x3 + x4)
     + 15*x3 + 80*x4 - 90*log(1 + x5) + 25*x5 - 80*log(1 + x6) + 35*x6 - 40*x7
     + 15*x8 - 35*x9 + 5*b10 + 8*b11 + 6*b12 + 10*b13 + 6*b14 + 7*b15 + 4*b16
     + 5*b17 + 120;

subject to

e1: (-1.5*log(1 + x5)) - log(1 + x6) - x8 <= 0;

e2: -log(1 + x3 + x4) <= 0;

e3:  - x1 - x2 + x3 + 2*x4 + 0.8*x5 + 0.8*x6 - 0.5*x7 - x8 - 2*x9 <= 0;

e4:  - x1 - x2 + 2*x4 + 0.8*x5 + 0.8*x6 - 2*x7 - x8 - 2*x9 <= 0;

e5:  - 2*x4 - 0.8*x5 - 0.8*x6 + 2*x7 + x8 + 2*x9 <= 0;

e6:  - 0.8*x5 - 0.8*x6 + x8 <= 0;

e7:  - x4 + x7 + x9 <= 0;

e8:  - 0.4*x5 - 0.4*x6 + 1.5*x8 <= 0;

e9:    0.16*x5 + 0.16*x6 - 1.2*x8 <= 0;

e10:    x3 - 0.8*x4 <= 0;

e11:  - x3 + 0.4*x4 <= 0;

e12: exp(x1) - 10*b10 <= 1;

e13: exp(0.833333*x2) - 10*b11 <= 1;

e14:    x7 - 10*b12 <= 0;

e15:    0.8*x5 + 0.8*x6 - 10*b13 <= 0;

e16:    2*x4 - 2*x7 - 2*x9 - 10*b14 <= 0;

e17:    x5 - 10*b15 <= 0;

e18:    x6 - 10*b16 <= 0;

e19:    x3 + x4 - 10*b17 <= 0;

e20:    b10 + b11 = 1;

e21:    b13 + b14 <= 1;

e22:  - b13 + b15 + b16 = 0;

e23:    b12 - b17 <= 0;
