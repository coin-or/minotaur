#  MINLP written by GAMS Convert at 01/12/18 13:31:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         75       31        6       38        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        131       31      100        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        347      297       50        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 5;
var x2 >= 0, <= 5;
var x3 >= 0, <= 5;
var x4 >= 0, <= 5;
var x5 >= 0, <= 5;
var x6 >= 0, <= 5;
var x7 >= 0, <= 5;
var x8 >= 0, <= 5;
var x9 >= 0, <= 5;
var x10 >= 0, <= 5;
var x11 >= 0, <= 5;
var x12 >= 0, <= 5;
var x13 >= 0, <= 5;
var x14 >= 0, <= 5;
var x15 >= 0, <= 5;
var x16 >= 0, <= 5;
var x17 >= 0, <= 5;
var x18 >= 0, <= 5;
var x19 >= 0, <= 5;
var x20 >= 0, <= 5;
var x21 >= 0, <= 5;
var x22 >= 0, <= 5;
var x23 >= 0, <= 5;
var x24 >= 0, <= 5;
var x25 >= 0, <= 5;
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
var x106 >= 0, <= 15;
var x107 >= 0, <= 12;
var x108 >= 0, <= 9;
var x109 >= 0, <= 6;
var x110 >= 0, <= 6;
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

minimize obj:    0.1*b101 + 0.2*b102 + 0.3*b103 + 0.4*b104 + 0.5*b105 + x106
     + x107 + x108 + x109 + x110;

subject to

e2: x106*x1 + x107*x2 + x108*x3 + x109*x4 + x110*x5 >= 12;

e3: x106*x6 + x107*x7 + x108*x8 + x109*x9 + x110*x10 >= 6;

e4: x106*x11 + x107*x12 + x108*x13 + x109*x14 + x110*x15 >= 15;

e5: x106*x16 + x107*x17 + x108*x18 + x109*x19 + x110*x20 >= 6;

e6: x106*x21 + x107*x22 + x108*x23 + x109*x24 + x110*x25 >= 9;

e7:  - 330*x1 - 360*x6 - 370*x11 - 415*x16 - 435*x21 + 1800*b101 <= 0;

e8:  - 330*x2 - 360*x7 - 370*x12 - 415*x17 - 435*x22 + 1800*b102 <= 0;

e9:  - 330*x3 - 360*x8 - 370*x13 - 415*x18 - 435*x23 + 1800*b103 <= 0;

e10:  - 330*x4 - 360*x9 - 370*x14 - 415*x19 - 435*x24 + 1800*b104 <= 0;

e11:  - 330*x5 - 360*x10 - 370*x15 - 415*x20 - 435*x25 + 1800*b105 <= 0;

e12:    330*x1 + 360*x6 + 370*x11 + 415*x16 + 435*x21 - 2000*b101 <= 0;

e13:    330*x2 + 360*x7 + 370*x12 + 415*x17 + 435*x22 - 2000*b102 <= 0;

e14:    330*x3 + 360*x8 + 370*x13 + 415*x18 + 435*x23 - 2000*b103 <= 0;

e15:    330*x4 + 360*x9 + 370*x14 + 415*x19 + 435*x24 - 2000*b104 <= 0;

e16:    330*x5 + 360*x10 + 370*x15 + 415*x20 + 435*x25 - 2000*b105 <= 0;

e17:  - x1 - x6 - x11 - x16 - x21 + b101 <= 0;

e18:  - x2 - x7 - x12 - x17 - x22 + b102 <= 0;

e19:  - x3 - x8 - x13 - x18 - x23 + b103 <= 0;

e20:  - x4 - x9 - x14 - x19 - x24 + b104 <= 0;

e21:  - x5 - x10 - x15 - x20 - x25 + b105 <= 0;

e22:    x1 + x6 + x11 + x16 + x21 - 5*b101 <= 0;

e23:    x2 + x7 + x12 + x17 + x22 - 5*b102 <= 0;

e24:    x3 + x8 + x13 + x18 + x23 - 5*b103 <= 0;

e25:    x4 + x9 + x14 + x19 + x24 - 5*b104 <= 0;

e26:    x5 + x10 + x15 + x20 + x25 - 5*b105 <= 0;

e27:    b101 - x106 <= 0;

e28:    b102 - x107 <= 0;

e29:    b103 - x108 <= 0;

e30:    b104 - x109 <= 0;

e31:    b105 - x110 <= 0;

e32:  - 15*b101 + x106 <= 0;

e33:  - 12*b102 + x107 <= 0;

e34:  - 9*b103 + x108 <= 0;

e35:  - 6*b104 + x109 <= 0;

e36:  - 6*b105 + x110 <= 0;

e37:    x106 + x107 + x108 + x109 + x110 >= 10;

e38:  - b101 + b102 <= 0;

e39:  - b102 + b103 <= 0;

e40:  - b103 + b104 <= 0;

e41:  - b104 + b105 <= 0;

e42:  - x106 + x107 <= 0;

e43:  - x107 + x108 <= 0;

e44:  - x108 + x109 <= 0;

e45:  - x109 + x110 <= 0;

e46:    x1 - b26 - 2*b27 - 4*b28 = 0;

e47:    x2 - b29 - 2*b30 - 4*b31 = 0;

e48:    x3 - b32 - 2*b33 - 4*b34 = 0;

e49:    x4 - b35 - 2*b36 - 4*b37 = 0;

e50:    x5 - b38 - 2*b39 - 4*b40 = 0;

e51:    x6 - b41 - 2*b42 - 4*b43 = 0;

e52:    x7 - b44 - 2*b45 - 4*b46 = 0;

e53:    x8 - b47 - 2*b48 - 4*b49 = 0;

e54:    x9 - b50 - 2*b51 - 4*b52 = 0;

e55:    x10 - b53 - 2*b54 - 4*b55 = 0;

e56:    x11 - b56 - 2*b57 - 4*b58 = 0;

e57:    x12 - b59 - 2*b60 - 4*b61 = 0;

e58:    x13 - b62 - 2*b63 - 4*b64 = 0;

e59:    x14 - b65 - 2*b66 - 4*b67 = 0;

e60:    x15 - b68 - 2*b69 - 4*b70 = 0;

e61:    x16 - b71 - 2*b72 - 4*b73 = 0;

e62:    x17 - b74 - 2*b75 - 4*b76 = 0;

e63:    x18 - b77 - 2*b78 - 4*b79 = 0;

e64:    x19 - b80 - 2*b81 - 4*b82 = 0;

e65:    x20 - b83 - 2*b84 - 4*b85 = 0;

e66:    x21 - b86 - 2*b87 - 4*b88 = 0;

e67:    x22 - b89 - 2*b90 - 4*b91 = 0;

e68:    x23 - b92 - 2*b93 - 4*b94 = 0;

e69:    x24 - b95 - 2*b96 - 4*b97 = 0;

e70:    x25 - b98 - 2*b99 - 4*b100 = 0;

e71:    x106 - b111 - 2*b112 - 4*b113 - 8*b114 = 0;

e72:    x107 - b115 - 2*b116 - 4*b117 - 8*b118 = 0;

e73:    x108 - b119 - 2*b120 - 4*b121 - 8*b122 = 0;

e74:    x109 - b123 - 2*b124 - 4*b125 - 8*b126 = 0;

e75:    x110 - b127 - 2*b128 - 4*b129 - 8*b130 = 0;
