#  MINLP written by GAMS Convert at 01/12/18 13:33:45
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         66       11       10       45        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         63       23       40        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        243      238        5        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 29;
var x2 >= 0, <= 29;
var x3 >= 0, <= 29;
var x4 >= 0, <= 29;
var x5 >= 0, <= 29;
var x6 >= 0, <= 29;
var x7 >= 0, <= 29;
var x8 >= 0, <= 29;
var x9 >= 0, <= 29;
var x10 >= 0, <= 29;
var x11 := 1, >= 1, <= 40;
var x12 := 1, >= 1, <= 50;
var x13 := 1, >= 1, <= 60;
var x14 := 1, >= 1, <= 35;
var x15 := 1, >= 1, <= 75;
var x16 := 1, >= 1, <= 40;
var x17 := 1, >= 1, <= 50;
var x18 := 1, >= 1, <= 60;
var x19 := 1, >= 1, <= 35;
var x20 := 1, >= 1, <= 75;
var x21 >= 0, <= 30;
var x22 >= 0, <= 30;
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

minimize obj:    2*x21 + 2*x22;

subject to

e2:  - x1 - x11 + x21 >= 0;

e3:  - x2 - x12 + x21 >= 0;

e4:  - x3 - x13 + x21 >= 0;

e5:  - x4 - x14 + x21 >= 0;

e6:  - x5 - x15 + x21 >= 0;

e7:  - x6 - x16 + x22 >= 0;

e8:  - x7 - x17 + x22 >= 0;

e9:  - x8 - x18 + x22 >= 0;

e10:  - x9 - x19 + x22 >= 0;

e11:  - x10 - x20 + x22 >= 0;

e12: 40/x16 - x11 <= 0;

e13: 50/x17 - x12 <= 0;

e14: 60/x18 - x13 <= 0;

e15: 35/x19 - x14 <= 0;

e16: 75/x20 - x15 <= 0;

e17:    x1 - x2 + x11 + 69*b23 <= 69;

e18:    x1 - x3 + x11 + 69*b24 <= 69;

e19:    x1 - x4 + x11 + 69*b25 <= 69;

e20:    x1 - x5 + x11 + 69*b26 <= 69;

e21:    x2 - x3 + x12 + 79*b27 <= 79;

e22:    x2 - x4 + x12 + 79*b28 <= 79;

e23:    x2 - x5 + x12 + 79*b29 <= 79;

e24:    x3 - x4 + x13 + 89*b30 <= 89;

e25:    x3 - x5 + x13 + 89*b31 <= 89;

e26:    x4 - x5 + x14 + 64*b32 <= 64;

e27:  - x1 + x2 + x12 + 79*b33 <= 79;

e28:  - x1 + x3 + x13 + 89*b34 <= 89;

e29:  - x1 + x4 + x14 + 64*b35 <= 64;

e30:  - x1 + x5 + x15 + 104*b36 <= 104;

e31:  - x2 + x3 + x13 + 89*b37 <= 89;

e32:  - x2 + x4 + x14 + 64*b38 <= 64;

e33:  - x2 + x5 + x15 + 104*b39 <= 104;

e34:  - x3 + x4 + x14 + 64*b40 <= 64;

e35:  - x3 + x5 + x15 + 104*b41 <= 104;

e36:  - x4 + x5 + x15 + 104*b42 <= 104;

e37:    x6 - x7 + x16 + 69*b43 <= 69;

e38:    x6 - x8 + x16 + 69*b44 <= 69;

e39:    x6 - x9 + x16 + 69*b45 <= 69;

e40:    x6 - x10 + x16 + 69*b46 <= 69;

e41:    x7 - x8 + x17 + 79*b47 <= 79;

e42:    x7 - x9 + x17 + 79*b48 <= 79;

e43:    x7 - x10 + x17 + 79*b49 <= 79;

e44:    x8 - x9 + x18 + 89*b50 <= 89;

e45:    x8 - x10 + x18 + 89*b51 <= 89;

e46:    x9 - x10 + x19 + 64*b52 <= 64;

e47:  - x6 + x7 + x17 + 79*b53 <= 79;

e48:  - x6 + x8 + x18 + 89*b54 <= 89;

e49:  - x6 + x9 + x19 + 64*b55 <= 64;

e50:  - x6 + x10 + x20 + 104*b56 <= 104;

e51:  - x7 + x8 + x18 + 89*b57 <= 89;

e52:  - x7 + x9 + x19 + 64*b58 <= 64;

e53:  - x7 + x10 + x20 + 104*b59 <= 104;

e54:  - x8 + x9 + x19 + 64*b60 <= 64;

e55:  - x8 + x10 + x20 + 104*b61 <= 104;

e56:  - x9 + x10 + x20 + 104*b62 <= 104;

e57:    b23 + b33 + b43 + b53 = 1;

e58:    b24 + b34 + b44 + b54 = 1;

e59:    b25 + b35 + b45 + b55 = 1;

e60:    b26 + b36 + b46 + b56 = 1;

e61:    b27 + b37 + b47 + b57 = 1;

e62:    b28 + b38 + b48 + b58 = 1;

e63:    b29 + b39 + b49 + b59 = 1;

e64:    b30 + b40 + b50 + b60 = 1;

e65:    b31 + b41 + b51 + b61 = 1;

e66:    b32 + b42 + b52 + b62 = 1;
