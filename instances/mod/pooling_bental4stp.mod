#  NLP written by GAMS Convert at 01/12/18 13:38:39
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         24       15        0        9        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         19       19        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         79       55       24        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 300;
var x3 >= 0, <= 50;
var x4 >= 0, <= 300;
var x5 >= 0, <= 100;
var x6 >= 0, <= 200;
var x7 >= 0, <= 100;
var x8 >= 0, <= 200;
var x9 >= 0, <= 1;
var x10 >= 0, <= 1;
var x11 >= 0, <= 1;
var x12 >= 0, <= 1;
var x13 >= 0, <= 1;
var x14 >= 0, <= 100;
var x15 >= 0, <= 200;
var x16 >= 0, <= 50;
var x17 >= 0, <= 50;
var x18 >= 0, <= 100;
var x19 >= 0, <= 200;

minimize obj:    x5 - 5*x6 - 3*x14 - 9*x15 + 6*x16 + 7*x18 + x19;

subject to

e2:    x14 + x15 <= 300;

e3:    x16 + x17 <= 50;

e4:    x18 + x19 <= 300;

e5:    x5 + x6 <= 300;

e6:    x14 + x15 + x16 + x17 + x18 + x19 <= 300;

e7:    x5 + x14 + x16 + x18 <= 100;

e8:    x6 + x15 + x17 + x19 <= 200;

e9:  - 0.5*x5 + 0.5*x14 - 1.5*x16 - 1.5*x18 <= 0;

e10:    0.5*x6 + 1.5*x15 - 0.5*x17 - 0.5*x19 <= 0;

e11:    x9 + x10 + x11 = 1;

e12:    x12 + x13 = 1;

e13: -x9*x7 + x14 = 0;

e14: -x9*x8 + x15 = 0;

e15: -x10*x7 + x16 = 0;

e16: -x10*x8 + x17 = 0;

e17: -x11*x7 + x18 = 0;

e18: -x11*x8 + x19 = 0;

e19: -x12*x2 + x14 = 0;

e20: -x13*x2 + x15 = 0;

e21: -x12*x3 + x16 = 0;

e22: -x13*x3 + x17 = 0;

e23: -x12*x4 + x18 = 0;

e24: -x13*x4 + x19 = 0;
