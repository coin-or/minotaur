#  MINLP written by GAMS Convert at 01/12/18 13:47:04
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         47       46        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        105       40       65        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        392      132      260        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 0.1, >= 0.1, <= 0.9;
var x3 := 0.1, >= 0.1, <= 0.9;
var x4 := 0.1, >= 0.1, <= 0.9;
var x5 := 0.1, >= 0.1, <= 0.9;
var x6 := 0.1, >= 0.1, <= 0.9;
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
var x34 >= 0, <= 10;
var x35 >= 0, <= 10;
var x36 >= 0, <= 10;
var x37 >= 0, <= 10;
var x38 >= 0, <= 10;
var x39 >= 0, <= 10;
var x40 >= 0, <= 10;
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
var b78 binary >= 0, <= 1;
var b79 binary >= 0, <= 1;
var b80 binary >= 0, <= 1;
var b81 binary >= 0, <= 1;
var b82 binary >= 0, <= 1;
var b83 binary >= 0, <= 1;
var b84 binary >= 0, <= 1;
var b85 binary >= 0, <= 1;
var b86 binary >= 0, <= 1;
var b87 binary >= 0, <= 1;
var b88 binary >= 0, <= 1;
var b89 binary >= 0, <= 1;
var b90 binary >= 0, <= 1;
var b91 binary >= 0, <= 1;
var b92 binary >= 0, <= 1;
var b93 binary >= 0, <= 1;
var b94 binary >= 0, <= 1;
var b95 binary >= 0, <= 1;
var b96 binary >= 0, <= 1;
var b97 binary >= 0, <= 1;
var b98 binary >= 0, <= 1;
var b99 binary >= 0, <= 1;
var b100 binary >= 0, <= 1;
var b101 binary >= 0, <= 1;
var b102 binary >= 0, <= 1;
var b103 binary >= 0, <= 1;
var b104 binary >= 0, <= 1;
var b105 binary >= 0, <= 1;

minimize obj:    x25;

subject to

e2:    x7 <= 0.0675;

e3:    x9 - x10 + x11 = 0;

e4:    x12 - x13 + x14 = 0;

e5:    x15 - x16 + x17 = 0;

e6:    x18 - x19 + x20 = 0;

e7:    x21 - x22 + x23 = 0;

e8:    x26 - x27 + x28 = 0;

e9:    x29 - x30 + x31 = 0;

e10:    x32 - x33 + x34 = 0;

e11:    x35 - x36 + x37 = 0;

e12:    x38 - x39 + x40 = 0;

e13: x2**0.29*x10 - x11 = 0;

e14: x3**0.13*x13 - x14 = 0;

e15: x4**0.06*x16 - x17 = 0;

e16: x5**0.15*x19 - x20 = 0;

e17: x6**0.2*x22 - x23 = 0;

e18: x2**0.74*x27 - x28 = 0;

e19: x3**0.79*x30 - x31 = 0;

e20: x4**0.71*x33 - x34 = 0;

e21: x5**0.8*x36 - x37 = 0;

e22: x6**0.7*x39 - x40 = 0;

e23: b41*x9 + b46*x11 + b51*x12 + b56*x14 + b61*x15 + b66*x17 + b71*x18 + b76*
     x20 + b81*x21 + b86*x23 - x10 + 0.675*b91 = 0;

e24: b42*x9 + b47*x11 + b52*x12 + b57*x14 + b62*x15 + b67*x17 + b72*x18 + b77*
     x20 + b82*x21 + b87*x23 - x13 + 0.675*b92 = 0;

e25: b43*x9 + b48*x11 + b53*x12 + b58*x14 + b63*x15 + b68*x17 + b73*x18 + b78*
     x20 + b83*x21 + b88*x23 - x16 + 0.675*b93 = 0;

e26: b44*x9 + b49*x11 + b54*x12 + b59*x14 + b64*x15 + b69*x17 + b74*x18 + b79*
     x20 + b84*x21 + b89*x23 - x19 + 0.675*b94 = 0;

e27: b45*x9 + b50*x11 + b55*x12 + b60*x14 + b65*x15 + b70*x17 + b75*x18 + b80*
     x20 + b85*x21 + b90*x23 - x22 + 0.675*b95 = 0;

e28: b41*x26 + b46*x28 + b51*x29 + b56*x31 + b61*x32 + b66*x34 + b71*x35 + b76*
     x37 + b81*x38 + b86*x40 - x27 + 0.649*b91 = 0;

e29: b42*x26 + b47*x28 + b52*x29 + b57*x31 + b62*x32 + b67*x34 + b72*x35 + b77*
     x37 + b82*x38 + b87*x40 - x30 + 0.649*b92 = 0;

e30: b43*x26 + b48*x28 + b53*x29 + b58*x31 + b63*x32 + b68*x34 + b73*x35 + b78*
     x37 + b83*x38 + b88*x40 - x33 + 0.649*b93 = 0;

e31: b44*x26 + b49*x28 + b54*x29 + b59*x31 + b64*x32 + b69*x34 + b74*x35 + b79*
     x37 + b84*x38 + b89*x40 - x36 + 0.649*b94 = 0;

e32: b45*x26 + b50*x28 + b55*x29 + b60*x31 + b65*x32 + b70*x34 + b75*x35 + b80*
     x37 + b85*x38 + b90*x40 - x39 + 0.649*b95 = 0;

e33: b96*x9 + b97*x12 + b98*x15 + b99*x18 + b100*x21 - x7 = 0;

e34: b96*x26 + b97*x29 + b98*x32 + b99*x35 + b100*x38 - x24 = 0;

e35: b101*x11 + b102*x14 + b103*x17 + b104*x20 + b105*x23 - x8 = 0;

e36: b101*x28 + b102*x31 + b103*x34 + b104*x37 + b105*x40 - x25 = 0;

e37:    b41 + b42 + b43 + b44 + b45 + b96 = 1;

e38:    b51 + b52 + b53 + b54 + b55 + b97 = 1;

e39:    b61 + b62 + b63 + b64 + b65 + b98 = 1;

e40:    b71 + b72 + b73 + b74 + b75 + b99 = 1;

e41:    b81 + b82 + b83 + b84 + b85 + b100 = 1;

e42:    b46 + b47 + b48 + b49 + b50 + b101 = 1;

e43:    b56 + b57 + b58 + b59 + b60 + b102 = 1;

e44:    b66 + b67 + b68 + b69 + b70 + b103 = 1;

e45:    b76 + b77 + b78 + b79 + b80 + b104 = 1;

e46:    b86 + b87 + b88 + b89 + b90 + b105 = 1;

e47:    b91 + b92 + b93 + b94 + b95 = 1;
