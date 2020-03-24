#  MINLP written by GAMS Convert at 01/12/18 13:33:45
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         25        4        6       15        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         27       15       12        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         87       84        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 29;
var x2 >= 0, <= 29;
var x3 >= 0, <= 29;
var x4 >= 0, <= 29;
var x5 >= 0, <= 29;
var x6 >= 0, <= 29;
var x7 := 1, >= 1, <= 40;
var x8 := 1, >= 1, <= 50;
var x9 := 1, >= 1, <= 60;
var x10 := 1, >= 1, <= 40;
var x11 := 1, >= 1, <= 50;
var x12 := 1, >= 1, <= 60;
var x13 >= 0, <= 30;
var x14 >= 0, <= 30;
var b15 binary >= 0, <= 1;
var b16 binary >= 0, <= 1;
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;
var b22 binary >= 0, <= 1;
var b23 binary >= 0, <= 1;
var b24 binary >= 0, <= 1;
var b25 binary >= 0, <= 1;
var b26 binary >= 0, <= 1;

minimize obj:    2*x13 + 2*x14;

subject to

e2:  - x1 - x7 + x13 >= 0;

e3:  - x2 - x8 + x13 >= 0;

e4:  - x3 - x9 + x13 >= 0;

e5:  - x4 - x10 + x14 >= 0;

e6:  - x5 - x11 + x14 >= 0;

e7:  - x6 - x12 + x14 >= 0;

e8: 40/x10 - x7 <= 0;

e9: 50/x11 - x8 <= 0;

e10: 60/x12 - x9 <= 0;

e11:    x1 - x2 + x7 + 69*b15 <= 69;

e12:    x1 - x3 + x7 + 69*b16 <= 69;

e13:    x2 - x3 + x8 + 79*b17 <= 79;

e14:  - x1 + x2 + x8 + 79*b18 <= 79;

e15:  - x1 + x3 + x9 + 89*b19 <= 89;

e16:  - x2 + x3 + x9 + 89*b20 <= 89;

e17:    x4 - x5 + x10 + 69*b21 <= 69;

e18:    x4 - x6 + x10 + 69*b22 <= 69;

e19:    x5 - x6 + x11 + 79*b23 <= 79;

e20:  - x4 + x5 + x11 + 79*b24 <= 79;

e21:  - x4 + x6 + x12 + 89*b25 <= 89;

e22:  - x5 + x6 + x12 + 89*b26 <= 89;

e23:    b15 + b18 + b21 + b24 = 1;

e24:    b16 + b19 + b22 + b25 = 1;

e25:    b17 + b20 + b23 + b26 = 1;
