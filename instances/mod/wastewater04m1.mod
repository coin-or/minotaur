#  NLP written by GAMS Convert at 01/12/18 13:47:04
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         22       16        0        6        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         24       24        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         90       58       32        0
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
var x20 >= 0, <= 1000000;
var x21 >= 0, <= 1000000;
var x22 >= 0, <= 1000000;
var x23 >= 0, <= 1000000;

minimize obj:    x22 + x23;

subject to

e2:  - x13 - x17 - x18 = -40;

e3:  - x14 - x19 - x20 = -40;

e4:  - x9 - x11 - x17 - x19 + x22 = 0;

e5:  - x10 - x12 - x18 - x20 + x23 = 0;

e6:  - x9 - x10 - x15 + x22 = 0;

e7:  - x11 - x12 - x16 + x23 = 0;

e8:  - x13 - x14 - x15 - x16 + x21 = 0;

e9: x9*x5 + x11*x7 - x22*x1 + 100*x17 + 15*x19 = 0;

e10: x9*x6 + x11*x8 - x22*x2 + 20*x17 + 200*x19 = 0;

e11: x10*x5 + x12*x7 - x23*x3 + 100*x18 + 15*x20 = 0;

e12: x10*x6 + x12*x8 - x23*x4 + 20*x18 + 200*x20 = 0;

e13:    x1 <= 200;

e14:    x2 <= 200;

e15:    x3 <= 200;

e16:    x4 <= 200;

e17:  - 0.05*x1 + x5 = 0;

e18:  - x2 + x6 = 0;

e19:  - x3 + x7 = 0;

e20:  - 0.024*x4 + x8 = 0;

e21: x15*x5 + x16*x7 + 100*x13 + 15*x14 - 10*x21 <= 0;

e22: x15*x6 + x16*x8 + 20*x13 + 200*x14 - 10*x21 <= 0;
