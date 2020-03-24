#  MINLP written by GAMS Convert at 01/12/18 13:42:20
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         32       23        5        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         30       28        2        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         99       87       12        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 10, >= 0, <= 50;
var x2 := 2.5, >= 0, <= 50;
var x3 := 4, >= 0, <= 50;
var x4 := 14, >= 0, <= 50;
var x5 := 8, >= 0, <= 25;
var x6 := 25, >= 0, <= 25;
var x7 >= 0, <= 50;
var x8 >= 0, <= 50;
var x9 >= 0, <= 50;
var x10 >= 0, <= 50;
var x11 >= 0, <= 50;
var x12 >= 0, <= 50;
var x13 >= 0, <= 50;
var x14 >= 0, <= 50;
var x15 >= 0, <= 50;
var x16 >= 0, <= 50;
var x17 >= 0, <= 50;
var x18 >= 0, <= 50;
var x19 >= 0, <= 50;
var x20 >= 0, <= 50;
var x21 >= 0, <= 50;
var x22 >= 0, <= 50;
var x23 >= 0, <= 50;
var x24 >= 0, <= 50;
var x25 := 0.1, >= 0, <= 1;
var x26 := 0.1, >= 0, <= 1;
var x27 := 0.1, >= 0, <= 1;
var b28 binary := 0.5, >= 0, <= 1;
var b29 binary := 0.5, >= 0, <= 1;

minimize obj:  - 35*x1 - 30*x4 + 10*x5 + 8*x6 + x9 + x10 + 4*x11 + 4*x12
     + 50*b28 + 2*b29;

subject to

e1:  - 0.55*x5 - 0.5*x6 + x7 = 0;

e2:  - 0.45*x5 - 0.5*x6 + x8 = 0;

e3: -x25*x7 + x9 = 0;

e4: -x25*x8 + x10 = 0;

e5: -x26*x7 + x11 = 0;

e6: -x26*x8 + x12 = 0;

e7: -x27*x7 + x13 = 0;

e8: -x27*x8 + x14 = 0;

e9:  - x7 + x9 + x11 + x13 + x15 = 0;

e10:  - x8 + x10 + x12 + x14 + x16 = 0;

e11:  - 0.85*x9 + x17 = 0;

e12:  - 0.2*x10 + x18 = 0;

e13:  - 0.15*x9 + x19 = 0;

e14:  - 0.8*x10 + x20 = 0;

e15:  - 0.975*x11 + x21 = 0;

e16:  - 0.05*x12 + x22 = 0;

e17:  - 0.025*x11 + x23 = 0;

e18:  - 0.95*x12 + x24 = 0;

e19:    x1 - x13 - x17 - x21 = 0;

e20:    x2 - x14 - x18 - x22 = 0;

e21:    x3 - x15 - x19 - x23 = 0;

e22:    x4 - x16 - x20 - x24 = 0;

e23:    x9 + x10 - 2.5*b29 >= 0;

e24:    x9 + x10 - 25*b29 <= 0;

e25:    x11 + x12 - 2.5*b28 >= 0;

e26:    x11 + x12 - 25*b28 <= 0;

e27:    x1 - 4*x2 >= 0;

e28:  - 3*x3 + x4 >= 0;

e29:    x1 + x2 <= 15;

e30:    x3 + x4 <= 18;

e31:    b28 + b29 >= 1;
