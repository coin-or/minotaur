#  MINLP written by GAMS Convert at 01/12/18 13:47:04
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         55       54        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        137       47       90        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        529      169      360        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 0.1, >= 0.1, <= 0.9;
var x3 := 0.1, >= 0.1, <= 0.9;
var x4 := 0.1, >= 0.1, <= 0.9;
var x5 := 0.1, >= 0.1, <= 0.9;
var x6 := 0.1, >= 0.1, <= 0.9;
var x7 := 0.1, >= 0.1, <= 0.9;
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
var x41 >= 0, <= 10;
var x42 >= 0, <= 10;
var x43 >= 0, <= 10;
var x44 >= 0, <= 10;
var x45 >= 0, <= 10;
var x46 >= 0, <= 10;
var x47 >= 0, <= 10;
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
var b106 binary >= 0, <= 1;
var b107 binary >= 0, <= 1;
var b108 binary >= 0, <= 1;
var b109 binary >= 0, <= 1;
var b110 binary >= 0, <= 1;
var b111 binary >= 0, <= 1;
var b112 binary >= 0, <= 1;
var b113 binary >= 0, <= 1;
var b114 binary >= 0, <= 1;
var b115 binary >= 0, <= 1;
var b116 binary >= 0, <= 1;
var b117 binary >= 0, <= 1;
var b118 binary >= 0, <= 1;
var b119 binary >= 0, <= 1;
var b120 binary >= 0, <= 1;
var b121 binary >= 0, <= 1;
var b122 binary >= 0, <= 1;
var b123 binary >= 0, <= 1;
var b124 binary >= 0, <= 1;
var b125 binary >= 0, <= 1;
var b126 binary >= 0, <= 1;
var b127 binary >= 0, <= 1;
var b128 binary >= 0, <= 1;
var b129 binary >= 0, <= 1;
var b130 binary >= 0, <= 1;
var b131 binary >= 0, <= 1;
var b132 binary >= 0, <= 1;
var b133 binary >= 0, <= 1;
var b134 binary >= 0, <= 1;
var b135 binary >= 0, <= 1;
var b136 binary >= 0, <= 1;
var b137 binary >= 0, <= 1;

minimize obj:    x29;

subject to

e2:    x8 <= 0.0675;

e3:    x10 - x11 + x12 = 0;

e4:    x13 - x14 + x15 = 0;

e5:    x16 - x17 + x18 = 0;

e6:    x19 - x20 + x21 = 0;

e7:    x22 - x23 + x24 = 0;

e8:    x25 - x26 + x27 = 0;

e9:    x30 - x31 + x32 = 0;

e10:    x33 - x34 + x35 = 0;

e11:    x36 - x37 + x38 = 0;

e12:    x39 - x40 + x41 = 0;

e13:    x42 - x43 + x44 = 0;

e14:    x45 - x46 + x47 = 0;

e15: x2**0.29*x11 - x12 = 0;

e16: x3**0.13*x14 - x15 = 0;

e17: x4**0.06*x17 - x18 = 0;

e18: x5**0.15*x20 - x21 = 0;

e19: x6**0.2*x23 - x24 = 0;

e20: x7**0.17*x26 - x27 = 0;

e21: x2**0.74*x31 - x32 = 0;

e22: x3**0.79*x34 - x35 = 0;

e23: x4**0.71*x37 - x38 = 0;

e24: x5**0.8*x40 - x41 = 0;

e25: x6**0.7*x43 - x44 = 0;

e26: x7**0.85*x46 - x47 = 0;

e27: b48*x10 + b54*x12 + b60*x13 + b66*x15 + b72*x16 + b78*x18 + b84*x19 + b90*
     x21 + b96*x22 + b102*x24 + b108*x25 + b114*x27 - x11 + 0.675*b120 = 0;

e28: b49*x10 + b55*x12 + b61*x13 + b67*x15 + b73*x16 + b79*x18 + b85*x19 + b91*
     x21 + b97*x22 + b103*x24 + b109*x25 + b115*x27 - x14 + 0.675*b121 = 0;

e29: b50*x10 + b56*x12 + b62*x13 + b68*x15 + b74*x16 + b80*x18 + b86*x19 + b92*
     x21 + b98*x22 + b104*x24 + b110*x25 + b116*x27 - x17 + 0.675*b122 = 0;

e30: b51*x10 + b57*x12 + b63*x13 + b69*x15 + b75*x16 + b81*x18 + b87*x19 + b93*
     x21 + b99*x22 + b105*x24 + b111*x25 + b117*x27 - x20 + 0.675*b123 = 0;

e31: b52*x10 + b58*x12 + b64*x13 + b70*x15 + b76*x16 + b82*x18 + b88*x19 + b94*
     x21 + b100*x22 + b106*x24 + b112*x25 + b118*x27 - x23 + 0.675*b124 = 0;

e32: b53*x10 + b59*x12 + b65*x13 + b71*x15 + b77*x16 + b83*x18 + b89*x19 + b95*
     x21 + b101*x22 + b107*x24 + b113*x25 + b119*x27 - x26 + 0.675*b125 = 0;

e33: b48*x30 + b54*x32 + b60*x33 + b66*x35 + b72*x36 + b78*x38 + b84*x39 + b90*
     x41 + b96*x42 + b102*x44 + b108*x45 + b114*x47 - x31 + 0.649*b120 = 0;

e34: b49*x30 + b55*x32 + b61*x33 + b67*x35 + b73*x36 + b79*x38 + b85*x39 + b91*
     x41 + b97*x42 + b103*x44 + b109*x45 + b115*x47 - x34 + 0.649*b121 = 0;

e35: b50*x30 + b56*x32 + b62*x33 + b68*x35 + b74*x36 + b80*x38 + b86*x39 + b92*
     x41 + b98*x42 + b104*x44 + b110*x45 + b116*x47 - x37 + 0.649*b122 = 0;

e36: b51*x30 + b57*x32 + b63*x33 + b69*x35 + b75*x36 + b81*x38 + b87*x39 + b93*
     x41 + b99*x42 + b105*x44 + b111*x45 + b117*x47 - x40 + 0.649*b123 = 0;

e37: b52*x30 + b58*x32 + b64*x33 + b70*x35 + b76*x36 + b82*x38 + b88*x39 + b94*
     x41 + b100*x42 + b106*x44 + b112*x45 + b118*x47 - x43 + 0.649*b124 = 0;

e38: b53*x30 + b59*x32 + b65*x33 + b71*x35 + b77*x36 + b83*x38 + b89*x39 + b95*
     x41 + b101*x42 + b107*x44 + b113*x45 + b119*x47 - x46 + 0.649*b125 = 0;

e39: b126*x10 + b127*x13 + b128*x16 + b129*x19 + b130*x22 + b131*x25 - x8 = 0;

e40: b126*x30 + b127*x33 + b128*x36 + b129*x39 + b130*x42 + b131*x45 - x28 = 0;

e41: b132*x12 + b133*x15 + b134*x18 + b135*x21 + b136*x24 + b137*x27 - x9 = 0;

e42: b132*x32 + b133*x35 + b134*x38 + b135*x41 + b136*x44 + b137*x47 - x29 = 0;

e43:    b48 + b49 + b50 + b51 + b52 + b53 + b126 = 1;

e44:    b60 + b61 + b62 + b63 + b64 + b65 + b127 = 1;

e45:    b72 + b73 + b74 + b75 + b76 + b77 + b128 = 1;

e46:    b84 + b85 + b86 + b87 + b88 + b89 + b129 = 1;

e47:    b96 + b97 + b98 + b99 + b100 + b101 + b130 = 1;

e48:    b108 + b109 + b110 + b111 + b112 + b113 + b131 = 1;

e49:    b54 + b55 + b56 + b57 + b58 + b59 + b132 = 1;

e50:    b66 + b67 + b68 + b69 + b70 + b71 + b133 = 1;

e51:    b78 + b79 + b80 + b81 + b82 + b83 + b134 = 1;

e52:    b90 + b91 + b92 + b93 + b94 + b95 + b135 = 1;

e53:    b102 + b103 + b104 + b105 + b106 + b107 + b136 = 1;

e54:    b114 + b115 + b116 + b117 + b118 + b119 + b137 = 1;

e55:    b120 + b121 + b122 + b123 + b124 + b125 = 1;
