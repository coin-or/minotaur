#  MINLP written by GAMS Convert at 01/12/18 13:33:44
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         52       10        4       38        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         47       43        4        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        139      137        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 29;
var x2 >= 0, <= 29;
var x3 >= 0, <= 29;
var x4 >= 0, <= 29;
var x5 := 1, >= 1, <= 40;
var x6 := 1, >= 1, <= 50;
var x7 := 1, >= 1, <= 40;
var x8 := 1, >= 1, <= 50;
var x9 >= 0, <= 30;
var x10 >= 0, <= 30;
var x11 >= 0;
var x12 >= 0;
var x13 >= 0;
var x14 >= 0;
var x15 >= 0;
var x16 >= 0;
var x17 >= 0;
var x18 >= 0;
var x19 >= 0;
var x20 >= 0;
var x21 >= 0;
var x22 >= 0;
var x23 >= 0;
var x24 >= 0;
var x25 >= 0;
var x26 >= 0;
var x27 >= 0;
var x28 >= 0;
var x29 >= 0;
var x30 >= 0;
var x31 >= 0;
var x32 >= 0;
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
var b43 binary >= 0, <= 1;
var b44 binary >= 0, <= 1;
var b45 binary >= 0, <= 1;
var b46 binary >= 0, <= 1;

minimize obj:    2*x9 + 2*x10;

subject to

e2:  - x1 - x5 + x9 >= 0;

e3:  - x2 - x6 + x9 >= 0;

e4:  - x3 - x7 + x10 >= 0;

e5:  - x4 - x8 + x10 >= 0;

e6: 40/x7 - x5 <= 0;

e7: 50/x8 - x6 <= 0;

e8:    x1 - x11 - x12 - x13 - x14 = 0;

e9:    x2 - x15 - x16 - x17 - x18 = 0;

e10:    x3 - x19 - x20 - x21 - x22 = 0;

e11:    x4 - x23 - x24 - x25 - x26 = 0;

e12:    x5 - x27 - x28 - x29 - x30 = 0;

e13:    x6 - x31 - x32 - x33 - x34 = 0;

e14:    x7 - x35 - x36 - x37 - x38 = 0;

e15:    x8 - x39 - x40 - x41 - x42 = 0;

e16:    x11 - 29*b43 <= 0;

e17:    x12 - 29*b44 <= 0;

e18:    x13 - 29*b45 <= 0;

e19:    x14 - 29*b46 <= 0;

e20:    x15 - 29*b43 <= 0;

e21:    x16 - 29*b44 <= 0;

e22:    x17 - 29*b45 <= 0;

e23:    x18 - 29*b46 <= 0;

e24:    x19 - 29*b43 <= 0;

e25:    x20 - 29*b44 <= 0;

e26:    x21 - 29*b45 <= 0;

e27:    x22 - 29*b46 <= 0;

e28:    x23 - 29*b43 <= 0;

e29:    x24 - 29*b44 <= 0;

e30:    x25 - 29*b45 <= 0;

e31:    x26 - 29*b46 <= 0;

e32:    x27 - 40*b43 <= 0;

e33:    x28 - 40*b44 <= 0;

e34:    x29 - 40*b45 <= 0;

e35:    x30 - 40*b46 <= 0;

e36:    x31 - 40*b43 <= 0;

e37:    x32 - 40*b44 <= 0;

e38:    x33 - 40*b45 <= 0;

e39:    x34 - 40*b46 <= 0;

e40:    x35 - 40*b43 <= 0;

e41:    x36 - 40*b44 <= 0;

e42:    x37 - 40*b45 <= 0;

e43:    x38 - 40*b46 <= 0;

e44:    x39 - 40*b43 <= 0;

e45:    x40 - 40*b44 <= 0;

e46:    x41 - 40*b45 <= 0;

e47:    x42 - 40*b46 <= 0;

e48:    x11 - x15 + x27 <= 0;

e49:  - x12 + x16 + x32 <= 0;

e50:    x21 - x25 + x37 <= 0;

e51:  - x22 + x26 + x42 <= 0;

e52:    b43 + b44 + b45 + b46 = 1;
