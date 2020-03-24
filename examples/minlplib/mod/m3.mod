#  MINLP written by GAMS Convert at 01/12/18 13:34:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         44        1        0       43        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         27       21        6        0        0        0        0        0
#  FX      2        2        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        157      151        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var x8;
var x9;
var x10;
var x11;
var x12;
var x13;
var x14 := 2, >= 2, <= 5.4772;
var x15 := 3, >= 3, <= 6.7081;
var x16 := 3.8, >= 3.8, <= 7.5498;
var x17 := 10, >= 10, <= 10;
var x18 := 1.8258, >= 1.8258, <= 5;
var x19 := 2.2361, >= 2.2361, <= 5;
var x20 := 2.5166, >= 2.5166, <= 5;
var x21 := 5, >= 5, <= 5;
var x22;
var x23;
var x24;
var x25;
var x26;
var x27;

minimize obj:    6*x10 + 6*x11 + 6*x12 + 6*x13;

subject to

e2:    x22 - x23 <= 0;

e3:    0.5*x14 - x17 + x22 <= 0;

e4:    0.5*x14 - x22 <= 0;

e5:    0.5*x18 - x21 + x25 <= 0;

e6:    0.5*x18 - x25 <= 0;

e7:    0.5*x15 - x17 + x23 <= 0;

e8:    0.5*x15 - x23 <= 0;

e9:    0.5*x19 - x21 + x26 <= 0;

e10:    0.5*x19 - x26 <= 0;

e11:    0.5*x16 - x17 + x24 <= 0;

e12:    0.5*x16 - x24 <= 0;

e13:    0.5*x20 - x21 + x27 <= 0;

e14:    0.5*x20 - x27 <= 0;

e15:  - x8 + x22 - x23 <= 0;

e16:  - x8 - x22 + x23 <= 0;

e17:  - x9 + x25 - x26 <= 0;

e18:  - x9 - x25 + x26 <= 0;

e19:  - 10*b1 - 10*b2 + 0.5*x14 + 0.5*x15 - x22 + x23 <= 0;

e20:  - 10*b1 + 10*b2 + 0.5*x14 + 0.5*x15 + x22 - x23 <= 10;

e21:    5*b1 - 5*b2 + 0.5*x18 + 0.5*x19 - x25 + x26 <= 5;

e22:    5*b1 + 5*b2 + 0.5*x18 + 0.5*x19 + x25 - x26 <= 10;

e23:  - x10 + x22 - x24 <= 0;

e24:  - x10 - x22 + x24 <= 0;

e25:  - x11 + x25 - x27 <= 0;

e26:  - x11 - x25 + x27 <= 0;

e27:  - 10*b3 - 10*b4 + 0.5*x14 + 0.5*x16 - x22 + x24 <= 0;

e28:  - 10*b3 + 10*b4 + 0.5*x14 + 0.5*x16 + x22 - x24 <= 10;

e29:    5*b3 - 5*b4 + 0.5*x18 + 0.5*x20 - x25 + x27 <= 5;

e30:    5*b3 + 5*b4 + 0.5*x18 + 0.5*x20 + x25 - x27 <= 10;

e31:  - x12 + x23 - x24 <= 0;

e32:  - x12 - x23 + x24 <= 0;

e33:  - x13 + x26 - x27 <= 0;

e34:  - x13 - x26 + x27 <= 0;

e35:  - 10*b5 - 10*b6 + 0.5*x15 + 0.5*x16 - x23 + x24 <= 0;

e36:  - 10*b5 + 10*b6 + 0.5*x15 + 0.5*x16 + x23 - x24 <= 10;

e37:    5*b5 - 5*b6 + 0.5*x19 + 0.5*x20 - x26 + x27 <= 5;

e38:    5*b5 + 5*b6 + 0.5*x19 + 0.5*x20 + x26 - x27 <= 10;

e39: 10/x14 - x18 <= 0;

e40: 10/x18 - x14 <= 0;

e41: 15/x15 - x19 <= 0;

e42: 15/x19 - x15 <= 0;

e43: 19/x16 - x20 <= 0;

e44: 19/x20 - x16 <= 0;
