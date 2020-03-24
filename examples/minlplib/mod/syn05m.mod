#  MINLP written by GAMS Convert at 01/12/18 13:43:48
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         29        6        6       17        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         21       16        5        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         84       81        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 10;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;
var x7 >= 0;
var x8 >= 0;
var x9 >= 0;
var x10 >= 0;
var x11 >= 0;
var x12 >= 0;
var x13 >= 0, <= 7;
var x14 >= 0;
var x15 >= 0;
var x16 >= 0;
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;

maximize obj:    5*x8 - 2*x13 + 200*x14 + 250*x15 + 300*x16 - 5*b17 - 8*b18
     - 6*b19 - 10*b20 - 6*b21;

subject to

e2:    x2 - x3 - x4 = 0;

e3:  - x5 - x6 + x7 = 0;

e4:    x7 - x8 - x9 = 0;

e5:    x9 - x10 - x11 - x12 = 0;

e6: -log(1 + x3) + x5 + b17 <= 1;

e7:    x3 - 10*b17 <= 0;

e8:    x5 - 2.39789527279837*b17 <= 0;

e9: -1.2*log(1 + x4) + x6 + b18 <= 1;

e10:    x4 - 10*b18 <= 0;

e11:    x6 - 2.87747432735804*b18 <= 0;

e12:  - 0.75*x10 + x14 + b19 <= 1;

e13:  - 0.75*x10 + x14 - b19 >= -1;

e14:    x10 - 2.87747432735804*b19 <= 0;

e15:    x14 - 2.15810574551853*b19 <= 0;

e16: -1.5*log(1 + x11) + x15 + b20 <= 1;

e17:    x11 - 2.87747432735804*b20 <= 0;

e18:    x15 - 2.03277599268042*b20 <= 0;

e19:  - x12 + x16 + b21 <= 1;

e20:  - x12 + x16 - b21 >= -1;

e21:  - 0.5*x13 + x16 + b21 <= 1;

e22:  - 0.5*x13 + x16 - b21 >= -1;

e23:    x12 - 2.87747432735804*b21 <= 0;

e24:    x13 - 7*b21 <= 0;

e25:    x16 - 3.5*b21 <= 0;

e26:    b17 + b18 = 1;

e27:    b17 + b18 - b19 >= 0;

e28:    b17 + b18 - b20 >= 0;

e29:    b17 + b18 - b21 >= 0;
