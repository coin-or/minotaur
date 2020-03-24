#  MINLP written by GAMS Convert at 01/12/18 13:42:28
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         55        7       24       24        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         45       21       24        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        189      181        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 2.5, >= 2.5, <= 27.5;
var x2 := 3.5, >= 3.5, <= 26.5;
var x3 := 1.5, >= 1.5, <= 28.5;
var x4 := 1, >= 1, <= 29;
var x5 := 3, >= 3, <= 27;
var x6 := 2.5, >= 2.5, <= 27.5;
var x7 := 1.5, >= 1.5, <= 28.5;
var x8 := 1.5, >= 1.5, <= 28.5;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
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
var x33 >= 0;
var x34 >= 0;
var x35 >= 0;
var x36 >= 0;
var x37 >= 0;
var x38 >= 0;
var x39 >= 0;
var x40 >= 0;
var x41 >= 0;
var x42 >= 0;
var x43 >= 0;
var x44 >= 0;

minimize obj: 150*((-4 + x1)^2 + (-10 + x5)^2) + 390*((-10 + x2)^2 + (-15 + x6)
    ^2) + 240*((-7 + x3)^2 + (-9 + x7)^2) + 70*((-3 + x4)^2 + (-3 + x8)^2)
     + 300*x33 + 240*x34 + 210*x35 + 100*x36 + 150*x37 + 120*x38 + 300*x39
     + 240*x40 + 210*x41 + 100*x42 + 150*x43 + 120*x44;

subject to

e2:  - x1 + x2 + x33 >= 0;

e3:  - x1 + x3 + x34 >= 0;

e4:  - x1 + x4 + x35 >= 0;

e5:  - x2 + x3 + x36 >= 0;

e6:  - x2 + x4 + x37 >= 0;

e7:  - x3 + x4 + x38 >= 0;

e8:    x1 - x2 + x33 >= 0;

e9:    x1 - x3 + x34 >= 0;

e10:    x1 - x4 + x35 >= 0;

e11:    x2 - x3 + x36 >= 0;

e12:    x2 - x4 + x37 >= 0;

e13:    x3 - x4 + x38 >= 0;

e14:  - x5 + x6 + x39 >= 0;

e15:  - x5 + x7 + x40 >= 0;

e16:  - x5 + x8 + x41 >= 0;

e17:  - x6 + x7 + x42 >= 0;

e18:  - x6 + x8 + x43 >= 0;

e19:  - x7 + x8 + x44 >= 0;

e20:    x5 - x6 + x39 >= 0;

e21:    x5 - x7 + x40 >= 0;

e22:    x5 - x8 + x41 >= 0;

e23:    x6 - x7 + x42 >= 0;

e24:    x6 - x8 + x43 >= 0;

e25:    x7 - x8 + x44 >= 0;

e26:    x1 - x2 + 30*b9 <= 24;

e27:    x1 - x3 + 30*b10 <= 26;

e28:    x1 - x4 + 30*b11 <= 26.5;

e29:    x2 - x3 + 30*b12 <= 25;

e30:    x2 - x4 + 30*b13 <= 25.5;

e31:    x3 - x4 + 30*b14 <= 27.5;

e32:  - x1 + x2 + 30*b15 <= 24;

e33:  - x1 + x3 + 30*b16 <= 26;

e34:  - x1 + x4 + 30*b17 <= 26.5;

e35:  - x2 + x3 + 30*b18 <= 25;

e36:  - x2 + x4 + 30*b19 <= 25.5;

e37:  - x3 + x4 + 30*b20 <= 27.5;

e38:    x5 - x6 + 30*b21 <= 24.5;

e39:    x5 - x7 + 30*b22 <= 25.5;

e40:    x5 - x8 + 30*b23 <= 25.5;

e41:    x6 - x7 + 30*b24 <= 26;

e42:    x6 - x8 + 30*b25 <= 26;

e43:    x7 - x8 + 30*b26 <= 27;

e44:  - x5 + x6 + 30*b27 <= 24.5;

e45:  - x5 + x7 + 30*b28 <= 25.5;

e46:  - x5 + x8 + 30*b29 <= 25.5;

e47:  - x6 + x7 + 30*b30 <= 26;

e48:  - x6 + x8 + 30*b31 <= 26;

e49:  - x7 + x8 + 30*b32 <= 27;

e50:    b9 + b15 + b21 + b27 = 1;

e51:    b10 + b16 + b22 + b28 = 1;

e52:    b11 + b17 + b23 + b29 = 1;

e53:    b12 + b18 + b24 + b30 = 1;

e54:    b13 + b19 + b25 + b31 = 1;

e55:    b14 + b20 + b26 + b32 = 1;
