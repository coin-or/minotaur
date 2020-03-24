#  MINLP written by GAMS Convert at 01/12/18 13:31:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         56       21        5       30        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         93       21       72        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        241      209       32        0
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
var x69 >= 0, <= 30;
var x70 >= 0, <= 30;
var x71 >= 0, <= 30;
var x72 >= 0, <= 30;
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

minimize obj:    0.1*b65 + 0.2*b66 + 0.3*b67 + 0.4*b68 + x69 + x70 + x71 + x72;

subject to

e2: x69*x1 + x70*x2 + x71*x3 + x72*x4 >= 15;

e3: x69*x5 + x70*x6 + x71*x7 + x72*x8 >= 28;

e4: x69*x9 + x70*x10 + x71*x11 + x72*x12 >= 21;

e5: x69*x13 + x70*x14 + x71*x15 + x72*x16 >= 30;

e6:  - 290*x1 - 315*x5 - 350*x9 - 455*x13 + 1750*b65 <= 0;

e7:  - 290*x2 - 315*x6 - 350*x10 - 455*x14 + 1750*b66 <= 0;

e8:  - 290*x3 - 315*x7 - 350*x11 - 455*x15 + 1750*b67 <= 0;

e9:  - 290*x4 - 315*x8 - 350*x12 - 455*x16 + 1750*b68 <= 0;

e10:    290*x1 + 315*x5 + 350*x9 + 455*x13 - 1850*b65 <= 0;

e11:    290*x2 + 315*x6 + 350*x10 + 455*x14 - 1850*b66 <= 0;

e12:    290*x3 + 315*x7 + 350*x11 + 455*x15 - 1850*b67 <= 0;

e13:    290*x4 + 315*x8 + 350*x12 + 455*x16 - 1850*b68 <= 0;

e14:  - x1 - x5 - x9 - x13 + b65 <= 0;

e15:  - x2 - x6 - x10 - x14 + b66 <= 0;

e16:  - x3 - x7 - x11 - x15 + b67 <= 0;

e17:  - x4 - x8 - x12 - x16 + b68 <= 0;

e18:    x1 + x5 + x9 + x13 - 5*b65 <= 0;

e19:    x2 + x6 + x10 + x14 - 5*b66 <= 0;

e20:    x3 + x7 + x11 + x15 - 5*b67 <= 0;

e21:    x4 + x8 + x12 + x16 - 5*b68 <= 0;

e22:    b65 - x69 <= 0;

e23:    b66 - x70 <= 0;

e24:    b67 - x71 <= 0;

e25:    b68 - x72 <= 0;

e26:  - 30*b65 + x69 <= 0;

e27:  - 30*b66 + x70 <= 0;

e28:  - 30*b67 + x71 <= 0;

e29:  - 30*b68 + x72 <= 0;

e30:    x69 + x70 + x71 + x72 >= 19;

e31:  - b65 + b66 <= 0;

e32:  - b66 + b67 <= 0;

e33:  - b67 + b68 <= 0;

e34:  - x69 + x70 <= 0;

e35:  - x70 + x71 <= 0;

e36:  - x71 + x72 <= 0;

e37:    x1 - b17 - 2*b18 - 4*b19 = 0;

e38:    x2 - b20 - 2*b21 - 4*b22 = 0;

e39:    x3 - b23 - 2*b24 - 4*b25 = 0;

e40:    x4 - b26 - 2*b27 - 4*b28 = 0;

e41:    x5 - b29 - 2*b30 - 4*b31 = 0;

e42:    x6 - b32 - 2*b33 - 4*b34 = 0;

e43:    x7 - b35 - 2*b36 - 4*b37 = 0;

e44:    x8 - b38 - 2*b39 - 4*b40 = 0;

e45:    x9 - b41 - 2*b42 - 4*b43 = 0;

e46:    x10 - b44 - 2*b45 - 4*b46 = 0;

e47:    x11 - b47 - 2*b48 - 4*b49 = 0;

e48:    x12 - b50 - 2*b51 - 4*b52 = 0;

e49:    x13 - b53 - 2*b54 - 4*b55 = 0;

e50:    x14 - b56 - 2*b57 - 4*b58 = 0;

e51:    x15 - b59 - 2*b60 - 4*b61 = 0;

e52:    x16 - b62 - 2*b63 - 4*b64 = 0;

e53:    x69 - b73 - 2*b74 - 4*b75 - 8*b76 - 16*b77 = 0;

e54:    x70 - b78 - 2*b79 - 4*b80 - 8*b81 - 16*b82 = 0;

e55:    x71 - b83 - 2*b84 - 4*b85 - 8*b86 - 16*b87 = 0;

e56:    x72 - b88 - 2*b89 - 4*b90 - 8*b91 - 16*b92 = 0;
