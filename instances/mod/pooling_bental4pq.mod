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
#         59       47       12        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 1;
var x5 >= 0, <= 100;
var x6 >= 0, <= 200;
var x7 >= 0, <= 100;
var x8 >= 0, <= 200;
var x9 >= 0, <= 100;
var x10 >= 0, <= 200;
var x11 >= 0, <= 50;
var x12 >= 0, <= 50;
var x13 >= 0, <= 100;
var x14 >= 0, <= 200;

minimize obj:    x5 - 5*x6 - 3*x9 - 9*x10 + 6*x11 + 7*x13 + x14;

subject to

e2:    x9 + x10 <= 300;

e3:    x11 + x12 <= 50;

e4:    x13 + x14 <= 300;

e5:    x5 + x6 <= 300;

e6:    x9 + x10 + x11 + x12 + x13 + x14 <= 300;

e7:    x5 + x9 + x11 + x13 <= 100;

e8:    x6 + x10 + x12 + x14 <= 200;

e9:  - 0.5*x5 + 0.5*x9 - 1.5*x11 - 1.5*x13 <= 0;

e10:    0.5*x6 + 1.5*x10 - 0.5*x12 - 0.5*x14 <= 0;

e11:    x2 + x3 + x4 = 1;

e12: -x2*x7 + x9 = 0;

e13: -x2*x8 + x10 = 0;

e14: -x3*x7 + x11 = 0;

e15: -x3*x8 + x12 = 0;

e16: -x4*x7 + x13 = 0;

e17: -x4*x8 + x14 = 0;
