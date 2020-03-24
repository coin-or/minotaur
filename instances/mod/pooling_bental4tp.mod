#  NLP written by GAMS Convert at 01/12/18 13:38:39
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         17        8        0        9        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         14       14        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         58       46       12        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 100;
var x5 >= 0, <= 200;
var x6 >= 0, <= 50;
var x7 >= 0, <= 50;
var x8 >= 0, <= 100;
var x9 >= 0, <= 200;
var x10 >= 0, <= 300;
var x11 >= 0, <= 50;
var x12 >= 0, <= 300;
var x13 >= 0, <= 100;
var x14 >= 0, <= 200;

minimize obj:  - 3*x4 - 9*x5 + 6*x6 + 7*x8 + x9 + x13 - 5*x14;

subject to

e2:    x4 + x5 <= 300;

e3:    x6 + x7 <= 50;

e4:    x8 + x9 <= 300;

e5:    x13 + x14 <= 300;

e6:    x4 + x5 + x6 + x7 + x8 + x9 <= 300;

e7:    x4 + x6 + x8 + x13 <= 100;

e8:    x5 + x7 + x9 + x14 <= 200;

e9:    0.5*x4 - 1.5*x6 - 1.5*x8 - 0.5*x13 <= 0;

e10:    1.5*x5 - 0.5*x7 - 0.5*x9 + 0.5*x14 <= 0;

e11:    x2 + x3 = 1;

e12: -x2*x10 + x4 = 0;

e13: -x3*x10 + x5 = 0;

e14: -x2*x11 + x6 = 0;

e15: -x3*x11 + x7 = 0;

e16: -x2*x12 + x8 = 0;

e17: -x3*x12 + x9 = 0;
