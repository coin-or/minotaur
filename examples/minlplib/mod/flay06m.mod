#  MINLP written by GAMS Convert at 01/12/18 13:33:45
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         94       16       12       66        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         87       27       60        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        351      345        6        0
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
var x11 >= 0, <= 29;
var x12 >= 0, <= 29;
var x13 := 1, >= 1, <= 40;
var x14 := 1, >= 1, <= 50;
var x15 := 1, >= 1, <= 60;
var x16 := 1, >= 1, <= 35;
var x17 := 1, >= 1, <= 75;
var x18 := 1, >= 1, <= 20;
var x19 := 1, >= 1, <= 40;
var x20 := 1, >= 1, <= 50;
var x21 := 1, >= 1, <= 60;
var x22 := 1, >= 1, <= 35;
var x23 := 1, >= 1, <= 75;
var x24 := 1, >= 1, <= 20;
var x25 >= 0, <= 30;
var x26 >= 0, <= 30;
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

minimize obj:    2*x25 + 2*x26;

subject to

e2:  - x1 - x13 + x25 >= 0;

e3:  - x2 - x14 + x25 >= 0;

e4:  - x3 - x15 + x25 >= 0;

e5:  - x4 - x16 + x25 >= 0;

e6:  - x5 - x17 + x25 >= 0;

e7:  - x6 - x18 + x25 >= 0;

e8:  - x7 - x19 + x26 >= 0;

e9:  - x8 - x20 + x26 >= 0;

e10:  - x9 - x21 + x26 >= 0;

e11:  - x10 - x22 + x26 >= 0;

e12:  - x11 - x23 + x26 >= 0;

e13:  - x12 - x24 + x26 >= 0;

e14: 40/x19 - x13 <= 0;

e15: 50/x20 - x14 <= 0;

e16: 60/x21 - x15 <= 0;

e17: 35/x22 - x16 <= 0;

e18: 75/x23 - x17 <= 0;

e19: 20/x24 - x18 <= 0;

e20:    x1 - x2 + x13 + 69*b27 <= 69;

e21:    x1 - x3 + x13 + 69*b28 <= 69;

e22:    x1 - x4 + x13 + 69*b29 <= 69;

e23:    x1 - x5 + x13 + 69*b30 <= 69;

e24:    x1 - x6 + x13 + 69*b31 <= 69;

e25:    x2 - x3 + x14 + 79*b32 <= 79;

e26:    x2 - x4 + x14 + 79*b33 <= 79;

e27:    x2 - x5 + x14 + 79*b34 <= 79;

e28:    x2 - x6 + x14 + 79*b35 <= 79;

e29:    x3 - x4 + x15 + 89*b36 <= 89;

e30:    x3 - x5 + x15 + 89*b37 <= 89;

e31:    x3 - x6 + x15 + 89*b38 <= 89;

e32:    x4 - x5 + x16 + 64*b39 <= 64;

e33:    x4 - x6 + x16 + 64*b40 <= 64;

e34:    x5 - x6 + x17 + 104*b41 <= 104;

e35:  - x1 + x2 + x14 + 79*b42 <= 79;

e36:  - x1 + x3 + x15 + 89*b43 <= 89;

e37:  - x1 + x4 + x16 + 64*b44 <= 64;

e38:  - x1 + x5 + x17 + 104*b45 <= 104;

e39:  - x1 + x6 + x18 + 49*b46 <= 49;

e40:  - x2 + x3 + x15 + 89*b47 <= 89;

e41:  - x2 + x4 + x16 + 64*b48 <= 64;

e42:  - x2 + x5 + x17 + 104*b49 <= 104;

e43:  - x2 + x6 + x18 + 49*b50 <= 49;

e44:  - x3 + x4 + x16 + 64*b51 <= 64;

e45:  - x3 + x5 + x17 + 104*b52 <= 104;

e46:  - x3 + x6 + x18 + 49*b53 <= 49;

e47:  - x4 + x5 + x17 + 104*b54 <= 104;

e48:  - x4 + x6 + x18 + 49*b55 <= 49;

e49:  - x5 + x6 + x18 + 49*b56 <= 49;

e50:    x7 - x8 + x19 + 69*b57 <= 69;

e51:    x7 - x9 + x19 + 69*b58 <= 69;

e52:    x7 - x10 + x19 + 69*b59 <= 69;

e53:    x7 - x11 + x19 + 69*b60 <= 69;

e54:    x7 - x12 + x19 + 69*b61 <= 69;

e55:    x8 - x9 + x20 + 79*b62 <= 79;

e56:    x8 - x10 + x20 + 79*b63 <= 79;

e57:    x8 - x11 + x20 + 79*b64 <= 79;

e58:    x8 - x12 + x20 + 79*b65 <= 79;

e59:    x9 - x10 + x21 + 89*b66 <= 89;

e60:    x9 - x11 + x21 + 89*b67 <= 89;

e61:    x9 - x12 + x21 + 89*b68 <= 89;

e62:    x10 - x11 + x22 + 64*b69 <= 64;

e63:    x10 - x12 + x22 + 64*b70 <= 64;

e64:    x11 - x12 + x23 + 104*b71 <= 104;

e65:  - x7 + x8 + x20 + 79*b72 <= 79;

e66:  - x7 + x9 + x21 + 89*b73 <= 89;

e67:  - x7 + x10 + x22 + 64*b74 <= 64;

e68:  - x7 + x11 + x23 + 104*b75 <= 104;

e69:  - x7 + x12 + x24 + 49*b76 <= 49;

e70:  - x8 + x9 + x21 + 89*b77 <= 89;

e71:  - x8 + x10 + x22 + 64*b78 <= 64;

e72:  - x8 + x11 + x23 + 104*b79 <= 104;

e73:  - x8 + x12 + x24 + 49*b80 <= 49;

e74:  - x9 + x10 + x22 + 64*b81 <= 64;

e75:  - x9 + x11 + x23 + 104*b82 <= 104;

e76:  - x9 + x12 + x24 + 49*b83 <= 49;

e77:  - x10 + x11 + x23 + 104*b84 <= 104;

e78:  - x10 + x12 + x24 + 49*b85 <= 49;

e79:  - x11 + x12 + x24 + 49*b86 <= 49;

e80:    b27 + b42 + b57 + b72 = 1;

e81:    b28 + b43 + b58 + b73 = 1;

e82:    b29 + b44 + b59 + b74 = 1;

e83:    b30 + b45 + b60 + b75 = 1;

e84:    b31 + b46 + b61 + b76 = 1;

e85:    b32 + b47 + b62 + b77 = 1;

e86:    b33 + b48 + b63 + b78 = 1;

e87:    b34 + b49 + b64 + b79 = 1;

e88:    b35 + b50 + b65 + b80 = 1;

e89:    b36 + b51 + b66 + b81 = 1;

e90:    b37 + b52 + b67 + b82 = 1;

e91:    b38 + b53 + b68 + b83 = 1;

e92:    b39 + b54 + b69 + b84 = 1;

e93:    b40 + b55 + b70 + b85 = 1;

e94:    b41 + b56 + b71 + b86 = 1;
