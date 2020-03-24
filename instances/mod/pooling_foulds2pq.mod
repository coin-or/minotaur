#  NLP written by GAMS Convert at 01/12/18 13:38:41
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         35       19        0       16        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         37       37        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        160      128       32        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 1;
var x5 >= 0, <= 1;
var x6 >= 0, <= 100;
var x7 >= 0, <= 200;
var x8 >= 0, <= 100;
var x9 >= 0, <= 200;
var x10 >= 0, <= 100;
var x11 >= 0, <= 200;
var x12 >= 0, <= 100;
var x13 >= 0, <= 200;
var x14 >= 0, <= 100;
var x15 >= 0, <= 200;
var x16 >= 0, <= 100;
var x17 >= 0, <= 200;
var x18 >= 0, <= 100;
var x19 >= 0, <= 200;
var x20 >= 0, <= 100;
var x21 >= 0, <= 200;
var x22 >= 0, <= 100;
var x23 >= 0, <= 200;
var x24 >= 0, <= 100;
var x25 >= 0, <= 200;
var x26 >= 0, <= 100;
var x27 >= 0, <= 200;
var x28 >= 0, <= 100;
var x29 >= 0, <= 200;
var x30 >= 0, <= 100;
var x31 >= 0, <= 200;
var x32 >= 0, <= 100;
var x33 >= 0, <= 200;
var x34 >= 0, <= 100;
var x35 >= 0, <= 200;
var x36 >= 0, <= 100;
var x37 >= 0, <= 200;

minimize obj:    x6 - 5*x7 + 4*x8 - 2*x9 - 2*x10 - 8*x11 + x12 - 5*x13 - 3*x22
     - 9*x23 - 6*x25 + 7*x26 + x27 + 10*x28 + 4*x29 - 6*x30 - 12*x31 - 3*x32
     - 9*x33 + 4*x34 - 2*x35 + 7*x36 + x37;

subject to

e2:    x22 + x23 + x24 + x25 <= 600;

e3:    x26 + x27 + x28 + x29 <= 600;

e4:    x6 + x7 + x8 + x9 <= 600;

e5:    x30 + x31 + x32 + x33 <= 600;

e6:    x34 + x35 + x36 + x37 <= 600;

e7:    x10 + x11 + x12 + x13 <= 600;

e8:    x22 + x23 + x24 + x25 + x26 + x27 + x28 + x29 <= 600;

e9:    x30 + x31 + x32 + x33 + x34 + x35 + x36 + x37 <= 600;

e10:    x6 + x10 + x22 + x26 + x30 + x34 <= 100;

e11:    x7 + x11 + x23 + x27 + x31 + x35 <= 200;

e12:    x8 + x12 + x24 + x28 + x32 + x36 <= 100;

e13:    x9 + x13 + x25 + x29 + x33 + x37 <= 200;

e14:  - 0.5*x6 + 0.5*x22 - 1.5*x26 + x30 - x34 <= 0;

e15:    0.5*x7 + x11 + 1.5*x23 - 0.5*x27 + 2*x31 <= 0;

e16:  - x8 - 0.5*x12 - 2*x28 + 0.5*x32 - 1.5*x36 <= 0;

e17:    0.5*x13 + x25 - x29 + 1.5*x33 - 0.5*x37 <= 0;

e18:    x2 + x3 = 1;

e19:    x4 + x5 = 1;

e20: -x2*x14 + x22 = 0;

e21: -x2*x15 + x23 = 0;

e22: -x2*x16 + x24 = 0;

e23: -x2*x17 + x25 = 0;

e24: -x3*x14 + x26 = 0;

e25: -x3*x15 + x27 = 0;

e26: -x3*x16 + x28 = 0;

e27: -x3*x17 + x29 = 0;

e28: -x4*x18 + x30 = 0;

e29: -x4*x19 + x31 = 0;

e30: -x4*x20 + x32 = 0;

e31: -x4*x21 + x33 = 0;

e32: -x5*x18 + x34 = 0;

e33: -x5*x19 + x35 = 0;

e34: -x5*x20 + x36 = 0;

e35: -x5*x21 + x37 = 0;
