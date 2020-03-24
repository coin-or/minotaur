#  MINLP written by GAMS Convert at 01/12/18 13:47:04
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         39       38        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         77       33       44        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        275       99      176        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 0.1, >= 0.1, <= 0.9;
var x3 := 0.1, >= 0.1, <= 0.9;
var x4 := 0.1, >= 0.1, <= 0.9;
var x5 := 0.1, >= 0.1, <= 0.9;
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
var x27 >= 0, <= 10;
var x28 >= 0, <= 10;
var x29 >= 0, <= 10;
var x30 >= 0, <= 10;
var x31 >= 0, <= 10;
var x32 >= 0, <= 10;
var x33 >= 0, <= 10;
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
var b54 binary >= 0, <= 1;
var b55 binary >= 0, <= 1;
var b56 binary >= 0, <= 1;
var b57 binary >= 0, <= 1;
var b58 binary >= 0, <= 1;
var b59 binary >= 0, <= 1;
var b60 binary >= 0, <= 1;
var b61 binary >= 0, <= 1;
var b62 binary >= 0, <= 1;
var b63 binary >= 0, <= 1;
var b64 binary >= 0, <= 1;
var b65 binary >= 0, <= 1;
var b66 binary >= 0, <= 1;
var b67 binary >= 0, <= 1;
var b68 binary >= 0, <= 1;
var b69 binary >= 0, <= 1;
var b70 binary >= 0, <= 1;
var b71 binary >= 0, <= 1;
var b72 binary >= 0, <= 1;
var b73 binary >= 0, <= 1;
var b74 binary >= 0, <= 1;
var b75 binary >= 0, <= 1;
var b76 binary >= 0, <= 1;
var b77 binary >= 0, <= 1;

minimize obj:    x21;

subject to

e2:    x6 <= 0.0675;

e3:    x8 - x9 + x10 = 0;

e4:    x11 - x12 + x13 = 0;

e5:    x14 - x15 + x16 = 0;

e6:    x17 - x18 + x19 = 0;

e7:    x22 - x23 + x24 = 0;

e8:    x25 - x26 + x27 = 0;

e9:    x28 - x29 + x30 = 0;

e10:    x31 - x32 + x33 = 0;

e11: x2**0.29*x9 - x10 = 0;

e12: x3**0.13*x12 - x13 = 0;

e13: x4**0.06*x15 - x16 = 0;

e14: x5**0.15*x18 - x19 = 0;

e15: x2**0.74*x23 - x24 = 0;

e16: x3**0.79*x26 - x27 = 0;

e17: x4**0.71*x29 - x30 = 0;

e18: x5**0.8*x32 - x33 = 0;

e19: b34*x8 + b38*x10 + b42*x11 + b46*x13 + b50*x14 + b54*x16 + b58*x17 + b62*
     x19 - x9 + 0.675*b66 = 0;

e20: b35*x8 + b39*x10 + b43*x11 + b47*x13 + b51*x14 + b55*x16 + b59*x17 + b63*
     x19 - x12 + 0.675*b67 = 0;

e21: b36*x8 + b40*x10 + b44*x11 + b48*x13 + b52*x14 + b56*x16 + b60*x17 + b64*
     x19 - x15 + 0.675*b68 = 0;

e22: b37*x8 + b41*x10 + b45*x11 + b49*x13 + b53*x14 + b57*x16 + b61*x17 + b65*
     x19 - x18 + 0.675*b69 = 0;

e23: b34*x22 + b38*x24 + b42*x25 + b46*x27 + b50*x28 + b54*x30 + b58*x31 + b62*
     x33 - x23 + 0.649*b66 = 0;

e24: b35*x22 + b39*x24 + b43*x25 + b47*x27 + b51*x28 + b55*x30 + b59*x31 + b63*
     x33 - x26 + 0.649*b67 = 0;

e25: b36*x22 + b40*x24 + b44*x25 + b48*x27 + b52*x28 + b56*x30 + b60*x31 + b64*
     x33 - x29 + 0.649*b68 = 0;

e26: b37*x22 + b41*x24 + b45*x25 + b49*x27 + b53*x28 + b57*x30 + b61*x31 + b65*
     x33 - x32 + 0.649*b69 = 0;

e27: b70*x8 + b71*x11 + b72*x14 + b73*x17 - x6 = 0;

e28: b70*x22 + b71*x25 + b72*x28 + b73*x31 - x20 = 0;

e29: b74*x10 + b75*x13 + b76*x16 + b77*x19 - x7 = 0;

e30: b74*x24 + b75*x27 + b76*x30 + b77*x33 - x21 = 0;

e31:    b34 + b35 + b36 + b37 + b70 = 1;

e32:    b42 + b43 + b44 + b45 + b71 = 1;

e33:    b50 + b51 + b52 + b53 + b72 = 1;

e34:    b58 + b59 + b60 + b61 + b73 = 1;

e35:    b38 + b39 + b40 + b41 + b74 = 1;

e36:    b46 + b47 + b48 + b49 + b75 = 1;

e37:    b54 + b55 + b56 + b57 + b76 = 1;

e38:    b62 + b63 + b64 + b65 + b77 = 1;

e39:    b66 + b67 + b68 + b69 = 1;
