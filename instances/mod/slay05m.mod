#  MINLP written by GAMS Convert at 01/12/18 13:42:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         91       11       40       40        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         71       31       40        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        311      301       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 2.5, >= 2.5, <= 27.5;
var x2 := 3.5, >= 3.5, <= 26.5;
var x3 := 1.5, >= 1.5, <= 28.5;
var x4 := 1, >= 1, <= 29;
var x5 := 2, >= 2, <= 28;
var x6 := 3, >= 3, <= 27;
var x7 := 2.5, >= 2.5, <= 27.5;
var x8 := 1.5, >= 1.5, <= 28.5;
var x9 := 1.5, >= 1.5, <= 28.5;
var x10 := 2, >= 2, <= 28;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var b13 binary >= 0, <= 1;
var b14 binary >= 0, <= 1;
var b15 binary >= 0, <= 1;
var b16 binary >= 0, <= 1;
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
var x51 >= 0;
var x52 >= 0;
var x53 >= 0;
var x54 >= 0;
var x55 >= 0;
var x56 >= 0;
var x57 >= 0;
var x58 >= 0;
var x59 >= 0;
var x60 >= 0;
var x61 >= 0;
var x62 >= 0;
var x63 >= 0;
var x64 >= 0;
var x65 >= 0;
var x66 >= 0;
var x67 >= 0;
var x68 >= 0;
var x69 >= 0;
var x70 >= 0;

minimize obj: 150*((-4 + x1)^2 + (-10 + x6)^2) + 390*((-10 + x2)^2 + (-15 + x7)
    ^2) + 240*((-7 + x3)^2 + (-9 + x8)^2) + 70*((-3 + x4)^2 + (-3 + x9)^2) + 
    165*((-20 + x5)^2 + (-17 + x10)^2) + 300*x51 + 240*x52 + 210*x53 + 140*x54
     + 100*x55 + 150*x56 + 220*x57 + 120*x58 + 300*x59 + 100*x60 + 300*x61
     + 240*x62 + 210*x63 + 140*x64 + 100*x65 + 150*x66 + 220*x67 + 120*x68
     + 300*x69 + 100*x70;

subject to

e2:  - x1 + x2 + x51 >= 0;

e3:  - x1 + x3 + x52 >= 0;

e4:  - x1 + x4 + x53 >= 0;

e5:  - x1 + x5 + x54 >= 0;

e6:  - x2 + x3 + x55 >= 0;

e7:  - x2 + x4 + x56 >= 0;

e8:  - x2 + x5 + x57 >= 0;

e9:  - x3 + x4 + x58 >= 0;

e10:  - x3 + x5 + x59 >= 0;

e11:  - x4 + x5 + x60 >= 0;

e12:    x1 - x2 + x51 >= 0;

e13:    x1 - x3 + x52 >= 0;

e14:    x1 - x4 + x53 >= 0;

e15:    x1 - x5 + x54 >= 0;

e16:    x2 - x3 + x55 >= 0;

e17:    x2 - x4 + x56 >= 0;

e18:    x2 - x5 + x57 >= 0;

e19:    x3 - x4 + x58 >= 0;

e20:    x3 - x5 + x59 >= 0;

e21:    x4 - x5 + x60 >= 0;

e22:  - x6 + x7 + x61 >= 0;

e23:  - x6 + x8 + x62 >= 0;

e24:  - x6 + x9 + x63 >= 0;

e25:  - x6 + x10 + x64 >= 0;

e26:  - x7 + x8 + x65 >= 0;

e27:  - x7 + x9 + x66 >= 0;

e28:  - x7 + x10 + x67 >= 0;

e29:  - x8 + x9 + x68 >= 0;

e30:  - x8 + x10 + x69 >= 0;

e31:  - x9 + x10 + x70 >= 0;

e32:    x6 - x7 + x61 >= 0;

e33:    x6 - x8 + x62 >= 0;

e34:    x6 - x9 + x63 >= 0;

e35:    x6 - x10 + x64 >= 0;

e36:    x7 - x8 + x65 >= 0;

e37:    x7 - x9 + x66 >= 0;

e38:    x7 - x10 + x67 >= 0;

e39:    x8 - x9 + x68 >= 0;

e40:    x8 - x10 + x69 >= 0;

e41:    x9 - x10 + x70 >= 0;

e42:    x1 - x2 + 30*b11 <= 24;

e43:    x1 - x3 + 30*b12 <= 26;

e44:    x1 - x4 + 30*b13 <= 26.5;

e45:    x1 - x5 + 30*b14 <= 25.5;

e46:    x2 - x3 + 30*b15 <= 25;

e47:    x2 - x4 + 30*b16 <= 25.5;

e48:    x2 - x5 + 30*b17 <= 24.5;

e49:    x3 - x4 + 30*b18 <= 27.5;

e50:    x3 - x5 + 30*b19 <= 26.5;

e51:    x4 - x5 + 30*b20 <= 27;

e52:  - x1 + x2 + 30*b21 <= 24;

e53:  - x1 + x3 + 30*b22 <= 26;

e54:  - x1 + x4 + 30*b23 <= 26.5;

e55:  - x1 + x5 + 30*b24 <= 25.5;

e56:  - x2 + x3 + 30*b25 <= 25;

e57:  - x2 + x4 + 30*b26 <= 25.5;

e58:  - x2 + x5 + 30*b27 <= 24.5;

e59:  - x3 + x4 + 30*b28 <= 27.5;

e60:  - x3 + x5 + 30*b29 <= 26.5;

e61:  - x4 + x5 + 30*b30 <= 27;

e62:    x6 - x7 + 30*b31 <= 24.5;

e63:    x6 - x8 + 30*b32 <= 25.5;

e64:    x6 - x9 + 30*b33 <= 25.5;

e65:    x6 - x10 + 30*b34 <= 25;

e66:    x7 - x8 + 30*b35 <= 26;

e67:    x7 - x9 + 30*b36 <= 26;

e68:    x7 - x10 + 30*b37 <= 25.5;

e69:    x8 - x9 + 30*b38 <= 27;

e70:    x8 - x10 + 30*b39 <= 26.5;

e71:    x9 - x10 + 30*b40 <= 26.5;

e72:  - x6 + x7 + 30*b41 <= 24.5;

e73:  - x6 + x8 + 30*b42 <= 25.5;

e74:  - x6 + x9 + 30*b43 <= 25.5;

e75:  - x6 + x10 + 30*b44 <= 25;

e76:  - x7 + x8 + 30*b45 <= 26;

e77:  - x7 + x9 + 30*b46 <= 26;

e78:  - x7 + x10 + 30*b47 <= 25.5;

e79:  - x8 + x9 + 30*b48 <= 27;

e80:  - x8 + x10 + 30*b49 <= 26.5;

e81:  - x9 + x10 + 30*b50 <= 26.5;

e82:    b11 + b21 + b31 + b41 = 1;

e83:    b12 + b22 + b32 + b42 = 1;

e84:    b13 + b23 + b33 + b43 = 1;

e85:    b14 + b24 + b34 + b44 = 1;

e86:    b15 + b25 + b35 + b45 = 1;

e87:    b16 + b26 + b36 + b46 = 1;

e88:    b17 + b27 + b37 + b47 = 1;

e89:    b18 + b28 + b38 + b48 = 1;

e90:    b19 + b29 + b39 + b49 = 1;

e91:    b20 + b30 + b40 + b50 = 1;
