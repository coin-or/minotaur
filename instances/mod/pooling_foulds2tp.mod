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
#        164      132       32        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 1;
var x5 >= 0, <= 1;
var x6 >= 0, <= 1;
var x7 >= 0, <= 1;
var x8 >= 0, <= 1;
var x9 >= 0, <= 1;
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
var x26 >= 0, <= 600;
var x27 >= 0, <= 600;
var x28 >= 0, <= 100;
var x29 >= 0, <= 200;
var x30 >= 0, <= 100;
var x31 >= 0, <= 200;
var x32 >= 0, <= 600;
var x33 >= 0, <= 600;
var x34 >= 0, <= 100;
var x35 >= 0, <= 200;
var x36 >= 0, <= 100;
var x37 >= 0, <= 200;

minimize obj:  - 3*x10 - 9*x11 - 6*x13 + 7*x14 + x15 + 10*x16 + 4*x17 - 6*x18
     - 12*x19 - 3*x20 - 9*x21 + 4*x22 - 2*x23 + 7*x24 + x25 + x28 - 5*x29
     + 4*x30 - 2*x31 - 2*x34 - 8*x35 + x36 - 5*x37;

subject to

e2:    x10 + x11 + x12 + x13 <= 600;

e3:    x14 + x15 + x16 + x17 <= 600;

e4:    x28 + x29 + x30 + x31 <= 600;

e5:    x18 + x19 + x20 + x21 <= 600;

e6:    x22 + x23 + x24 + x25 <= 600;

e7:    x34 + x35 + x36 + x37 <= 600;

e8:    x10 + x11 + x12 + x13 + x14 + x15 + x16 + x17 <= 600;

e9:    x18 + x19 + x20 + x21 + x22 + x23 + x24 + x25 <= 600;

e10:    x10 + x14 + x18 + x22 + x28 + x34 <= 100;

e11:    x11 + x15 + x19 + x23 + x29 + x35 <= 200;

e12:    x12 + x16 + x20 + x24 + x30 + x36 <= 100;

e13:    x13 + x17 + x21 + x25 + x31 + x37 <= 200;

e14:    0.5*x10 - 1.5*x14 + x18 - x22 - 0.5*x28 <= 0;

e15:    1.5*x11 - 0.5*x15 + 2*x19 + 0.5*x29 + x35 <= 0;

e16:  - 2*x16 + 0.5*x20 - 1.5*x24 - x30 - 0.5*x36 <= 0;

e17:    x13 - x17 + 1.5*x21 - 0.5*x25 + 0.5*x37 <= 0;

e18:    x2 + x3 + x4 + x5 = 1;

e19:    x6 + x7 + x8 + x9 = 1;

e20: -x2*x26 + x10 = 0;

e21: -x3*x26 + x11 = 0;

e22: -x4*x26 + x12 = 0;

e23: -x5*x26 + x13 = 0;

e24: -x2*x27 + x14 = 0;

e25: -x3*x27 + x15 = 0;

e26: -x4*x27 + x16 = 0;

e27: -x5*x27 + x17 = 0;

e28: -x6*x32 + x18 = 0;

e29: -x7*x32 + x19 = 0;

e30: -x8*x32 + x20 = 0;

e31: -x9*x32 + x21 = 0;

e32: -x6*x33 + x22 = 0;

e33: -x7*x33 + x23 = 0;

e34: -x8*x33 + x24 = 0;

e35: -x9*x33 + x25 = 0;
