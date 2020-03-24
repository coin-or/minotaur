#  NLP written by GAMS Convert at 01/12/18 13:38:44
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         19       11        0        8        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         15       15        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         57       41       16        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 300;
var x3 >= 0, <= 300;
var x4 >= 0, <= 100;
var x5 >= 0, <= 200;
var x6 >= 0, <= 100;
var x7 >= 0, <= 200;
var x8 >= 0, <= 1;
var x9 >= 0, <= 1;
var x10 >= 0, <= 1;
var x11 >= 0, <= 1;
var x12 >= 0, <= 100;
var x13 >= 0, <= 200;
var x14 >= 0, <= 100;
var x15 >= 0, <= 200;

minimize obj:    x4 - 5*x5 - 3*x12 - 9*x13 + 7*x14 + x15;

subject to

e2:    x12 + x13 <= 300;

e3:    x14 + x15 <= 300;

e4:    x4 + x5 <= 300;

e5:    x12 + x13 + x14 + x15 <= 300;

e6:    x4 + x12 + x14 <= 100;

e7:    x5 + x13 + x15 <= 200;

e8:  - 0.5*x4 + 0.5*x12 - 1.5*x14 <= 0;

e9:    0.5*x5 + 1.5*x13 - 0.5*x15 <= 0;

e10:    x8 + x9 = 1;

e11:    x10 + x11 = 1;

e12: -x8*x6 + x12 = 0;

e13: -x8*x7 + x13 = 0;

e14: -x9*x6 + x14 = 0;

e15: -x9*x7 + x15 = 0;

e16: -x10*x2 + x12 = 0;

e17: -x11*x2 + x13 = 0;

e18: -x10*x3 + x14 = 0;

e19: -x11*x3 + x15 = 0;
