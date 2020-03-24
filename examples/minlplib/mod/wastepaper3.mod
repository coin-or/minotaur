#  MINLP written by GAMS Convert at 01/12/18 13:47:04
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         31       30        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         53       26       27        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        178       70      108        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 0.1, >= 0.1, <= 0.9;
var x3 := 0.1, >= 0.1, <= 0.9;
var x4 := 0.1, >= 0.1, <= 0.9;
var x5 >= 0, <= 10;
var x6 >= 0, <= 10;
var x7 >= 0, <= 10;
var x8 >= 0, <= 10;
var x9 >= 0, <= 10;
var x10 >= 0, <= 10;
var x11 >= 0, <= 10;
var x12 >= 0, <= 10;
var x13 >= 0, <= 10;
var x14 >= 0, <= 10;
var x15 >= 0, <= 10;
var x16 >= 0, <= 10;
var x17 >= 0, <= 10;
var x18 >= 0, <= 10;
var x19 >= 0, <= 10;
var x20 >= 0, <= 10;
var x21 >= 0, <= 10;
var x22 >= 0, <= 10;
var x23 >= 0, <= 10;
var x24 >= 0, <= 10;
var x25 >= 0, <= 10;
var x26 >= 0, <= 10;
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
var b37 binary >= 0, <= 1;
var b38 binary >= 0, <= 1;
var b39 binary >= 0, <= 1;
var b40 binary >= 0, <= 1;
var b41 binary >= 0, <= 1;
var b42 binary >= 0, <= 1;
var b43 binary >= 0, <= 1;
var b44 binary >= 0, <= 1;
var b45 binary >= 0, <= 1;
var b46 binary >= 0, <= 1;
var b47 binary >= 0, <= 1;
var b48 binary >= 0, <= 1;
var b49 binary >= 0, <= 1;
var b50 binary >= 0, <= 1;
var b51 binary >= 0, <= 1;
var b52 binary >= 0, <= 1;
var b53 binary >= 0, <= 1;

minimize obj:    x17;

subject to

e2:    x5 <= 0.0675;

e3:    x7 - x8 + x9 = 0;

e4:    x10 - x11 + x12 = 0;

e5:    x13 - x14 + x15 = 0;

e6:    x18 - x19 + x20 = 0;

e7:    x21 - x22 + x23 = 0;

e8:    x24 - x25 + x26 = 0;

e9: x2**0.29*x8 - x9 = 0;

e10: x3**0.13*x11 - x12 = 0;

e11: x4**0.06*x14 - x15 = 0;

e12: x2**0.74*x19 - x20 = 0;

e13: x3**0.79*x22 - x23 = 0;

e14: x4**0.71*x25 - x26 = 0;

e15: b27*x7 + b30*x9 + b33*x10 + b36*x12 + b39*x13 + b42*x15 - x8 + 0.675*b45
      = 0;

e16: b28*x7 + b31*x9 + b34*x10 + b37*x12 + b40*x13 + b43*x15 - x11 + 0.675*b46
      = 0;

e17: b29*x7 + b32*x9 + b35*x10 + b38*x12 + b41*x13 + b44*x15 - x14 + 0.675*b47
      = 0;

e18: b27*x18 + b30*x20 + b33*x21 + b36*x23 + b39*x24 + b42*x26 - x19
      + 0.649*b45 = 0;

e19: b28*x18 + b31*x20 + b34*x21 + b37*x23 + b40*x24 + b43*x26 - x22
      + 0.649*b46 = 0;

e20: b29*x18 + b32*x20 + b35*x21 + b38*x23 + b41*x24 + b44*x26 - x25
      + 0.649*b47 = 0;

e21: b48*x7 + b49*x10 + b50*x13 - x5 = 0;

e22: b48*x18 + b49*x21 + b50*x24 - x16 = 0;

e23: b51*x9 + b52*x12 + b53*x15 - x6 = 0;

e24: b51*x20 + b52*x23 + b53*x26 - x17 = 0;

e25:    b27 + b28 + b29 + b48 = 1;

e26:    b33 + b34 + b35 + b49 = 1;

e27:    b39 + b40 + b41 + b50 = 1;

e28:    b30 + b31 + b32 + b51 = 1;

e29:    b36 + b37 + b38 + b52 = 1;

e30:    b42 + b43 + b44 + b53 = 1;

e31:    b45 + b46 + b47 = 1;
