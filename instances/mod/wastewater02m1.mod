#  NLP written by GAMS Convert at 01/12/18 13:47:04
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         15       12        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         20       20        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         61       45       16        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 1000000;
var x2 >= 0, <= 1000000;
var x3 >= 0, <= 1000000;
var x4 >= 0, <= 1000000;
var x5 >= 0, <= 1000000;
var x6 >= 0, <= 1000000;
var x7 >= 0, <= 1000000;
var x8 >= 0, <= 1000000;
var x9 >= 0, <= 1000000;
var x10 >= 0, <= 1000000;
var x11 >= 0, <= 1000000;
var x12 >= 0, <= 1000000;
var x13 >= 0, <= 1000000;
var x14 >= 0, <= 1000000;
var x15 >= 0, <= 1000000;
var x16 >= 0, <= 1000000;
var x17 >= 0, <= 1000000;
var x18 >= 0, <= 1000000;
var x19 >= 0, <= 1000000;

minimize obj:    x18 + x19;

subject to

e2:  - x9 - x13 - x14 = -60;

e3:  - x10 - x15 - x16 = -20;

e4:  - x5 - x7 - x13 - x15 + x18 = 0;

e5:  - x6 - x8 - x14 - x16 + x19 = 0;

e6:  - x5 - x6 - x11 + x18 = 0;

e7:  - x7 - x8 - x12 + x19 = 0;

e8:  - x9 - x10 - x11 - x12 + x17 = 0;

e9: x5*x3 + x7*x4 - x18*x1 + 400*x13 + 800*x15 = 0;

e10: x6*x3 + x8*x4 - x19*x2 + 400*x14 + 800*x16 = 0;

e11:    x1 <= 200;

e12:    x2 <= 1000;

e13:  - 0.01*x1 + x3 = 0;

e14:  - 0.2*x2 + x4 = 0;

e15: x11*x3 + x12*x4 + 400*x9 + 800*x10 - 10*x17 <= 0;
