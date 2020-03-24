#  MINLP written by GAMS Convert at 01/12/18 13:35:48
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         45        9       14       22        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         36       22       14        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        111      104        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0;
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
var x13 >= 0;
var x14 >= 0;
var x15 >= 0;
var x16 >= 0;
var x17 >= 0;
var x18 >= 0;
var x19 >= 0;
var x20 >= 0;
var x21 >= 0;
var x22 >= 0;
var b23 binary >= 0, <= 1;
var b24 binary >= 0, <= 1;
var b25 binary >= 0, <= 1;
var b26 binary >= 0, <= 1;
var b27 binary >= 0, <= 1;
var b28 binary >= 0, <= 1;
var b29 binary >= 0, <= 1;
var b30 binary >= 0, <= 1;
var b31 binary >= 0, <= 1;
var b32 binary >= 0, <= 1;
var b33 binary >= 0, <= 1;
var b34 binary >= 0, <= 1;
var b35 binary >= 0, <= 1;
var b36 binary >= 0, <= 1;

minimize obj: 42.18*x2*x2 + 40.36*x2*x3 + 21.76*x2*x4 + 10.6*x2*x5 + 24.64*x2*
    x6 + 47.68*x2*x7 + 34.82*x2*x8 + 70.89*x3*x3 + 43.16*x3*x4 + 30.82*x3*x5 + 
    46.48*x3*x6 + 47.6*x3*x7 + 25.24*x3*x8 + 25.51*x4*x4 + 19.2*x4*x5 + 45.26*
    x4*x6 + 26.44*x4*x7 + 9.4*x4*x8 + 22.33*x5*x5 + 20.64*x5*x6 + 20.92*x5*x7
     + 2*x5*x8 + 30.01*x6*x6 + 32.72*x6*x7 + 14.4*x6*x8 + 42.23*x7*x7 + 19.8*x7
    *x8 + 16.42*x8*x8 - 0.06435*x2 - 0.0548*x3 - 0.02505*x4 - 0.0762*x5 - 
    0.03815*x6 - 0.0927*x7 - 0.031*x8;

subject to

e1:    x2 + x3 + x4 + x5 + x6 + x7 + x8 = 1;

e2:    x2 - x9 + x16 = 0.2;

e3:    x3 - x10 + x17 = 0.2;

e4:    x4 - x11 + x18 = 0;

e5:    x5 - x12 + x19 = 0;

e6:    x6 - x13 + x20 = 0.2;

e7:    x7 - x14 + x21 = 0.2;

e8:    x8 - x15 + x22 = 0.2;

e9:    x9 + x10 + x11 + x12 + x13 + x14 + x15 <= 0.3;

e10:    x9 - 0.11*b23 <= 0;

e11:    x10 - 0.1*b24 <= 0;

e12:    x11 - 0.07*b25 <= 0;

e13:    x12 - 0.11*b26 <= 0;

e14:    x13 - 0.2*b27 <= 0;

e15:    x14 - 0.1*b28 <= 0;

e16:    x15 - 0.1*b29 <= 0;

e17:    x9 - 0.03*b23 >= 0;

e18:    x10 - 0.04*b24 >= 0;

e19:    x11 - 0.04*b25 >= 0;

e20:    x12 - 0.03*b26 >= 0;

e21:    x13 - 0.03*b27 >= 0;

e22:    x14 - 0.03*b28 >= 0;

e23:    x15 - 0.03*b29 >= 0;

e24:    x16 - 0.2*b30 <= 0;

e25:    x17 - 0.15*b31 <= 0;

e26:    x18 <= 0;

e27:    x19 <= 0;

e28:    x20 - 0.1*b34 <= 0;

e29:    x21 - 0.15*b35 <= 0;

e30:    x22 - 0.2*b36 <= 0;

e31:    x16 - 0.02*b30 >= 0;

e32:    x17 - 0.02*b31 >= 0;

e33:    x18 - 0.04*b32 >= 0;

e34:    x19 - 0.04*b33 >= 0;

e35:    x20 - 0.04*b34 >= 0;

e36:    x21 - 0.04*b35 >= 0;

e37:    x22 - 0.04*b36 >= 0;

e38:    b23 + b30 <= 1;

e39:    b24 + b31 <= 1;

e40:    b25 + b32 <= 1;

e41:    b26 + b33 <= 1;

e42:    b27 + b34 <= 1;

e43:    b28 + b35 <= 1;

e44:    b29 + b36 <= 1;
