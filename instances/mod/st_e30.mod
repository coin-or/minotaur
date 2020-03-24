#  NLP written by GAMS Convert at 01/12/18 13:43:25
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         16        8        3        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         15       15        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         44       30       14        0
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
var x9 >= -1, <= 1;
var x10 >= -1, <= 1;
var x11 >= 0, <= 3;
var x12 >= 0, <= 3;
var x13 >= 0, <= 3;
var x14 >= 0, <= 3;

minimize obj:  - x14;

subject to

e1: -x12*x7 - x1 + x3 = 0;

e2: -x12*x8 - x2 + x4 = 0;

e3: (-x13*x7) - x11*x9 - x1 + x5 = 0;

e4: (-x13*x8) - x11*x10 - x2 + x6 = 0;

e5: x7^2 + x8^2 = 1;

e6:    x8 + x9 = 0;

e7:  - x7 + x10 = 0;

e8:  - x12 + x14 <= 0;

e9:  - x11 + x14 <= 0;

e10:    2*x1 + x2 >= -1;

e11:    2*x3 + x4 >= -1;

e12:    2*x5 + x6 >= -1;

e13:    x1 + x2 <= 1;

e14:    x3 + x4 <= 1;

e15:    x5 + x6 <= 1;
