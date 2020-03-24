#  MINLP written by GAMS Convert at 01/12/18 13:31:55
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         32       18        2       12        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         33       25        8        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        100       95        5        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var x9 := 2, >= 0;
var x10 := 1.5, >= 0, <= 2;
var x11 >= 0;
var x12 >= 0, <= 2;
var x13 := 0.75, >= 0;
var x14 := 0.5, >= 0;
var x15 := 0.5, >= 0;
var x16 := 0.75, >= 0, <= 2;
var x17 >= 0, <= 1;
var x18 := 1.5, >= 0;
var x19 := 1.34, >= 0;
var x20 := 2, >= 0;
var x21 >= 0, <= 1;
var x22 >= 0;
var x23 >= 0;
var x24 >= 0, <= 2;
var x25 := 0.75, >= 0, <= 100;
var x26 >= 0, <= 2;
var x27 := 1.5, >= 0, <= 100;
var x28 >= 0, <= 2;
var x29 >= 0, <= 100;
var x30 := 1.7, >= 0;
var x31 := 1.5, >= 0;
var x32 := 0.5, >= 0, <= 3;

minimize obj:    8*b1 + 6*b2 + 10*b3 + 6*b4 + 7*b5 + 4*b6 + 5*b7 + 5*b8 + x9
     - 10*x10 + x11 - 15*x12 - 40*x16 + 15*x17 + 15*x21 + 80*x24 - 65*x25
     + 25*x26 - 60*x27 + 35*x28 - 80*x29 - 35*x32 + 122;

subject to

e1: exp(x10) - x9 = 1;

e2: exp(0.833333333333333*x12) - x11 = 1;

e3:  - x15 + 1.5*x16 + x17 = 0;

e4:    1.25*x19 - x20 + 1.25*x21 = 0;

e5:    x22 - 2*x23 = 0;

e6: exp(0.666666666666667*x27) - x26 = 1;

e7: exp(x29) - x28 = 1;

e8: exp(x25) - x17 - x24 = 1;

e9:    x20 - x26 - x28 = 0;

e10:  - x16 - x23 + x24 - x32 = 0;

e11:    x18 - x19 - x22 = 0;

e12:    x10 + x12 - x13 - x18 = 0;

e13:    x13 - x14 - x15 = 0;

e14:  - x27 - x29 + x30 = 0;

e15:  - x21 + x30 - x31 = 0;

e16:    x17 - 0.8*x24 <= 0;

e17:    x17 - 0.4*x24 >= 0;

e18:    x19 - 5*x21 <= 0;

e19:    x19 - 2*x21 >= 0;

e20:  - 10*b8 + x9 <= 0;

e21:  - 10*b1 + x11 <= 0;

e22:  - 10*b2 + x16 <= 0;

e23:  - 10*b3 + x19 + x21 <= 0;

e24:  - 10*b4 + x22 <= 0;

e25:  - 10*b5 + x26 <= 0;

e26:  - 10*b6 + x28 <= 0;

e27:  - 10*b7 + x17 + x24 <= 0;

e28:    b1 + b8 = 1;

e29:    b3 + b4 <= 1;

e30:  - b3 + b5 + b6 = 0;

e31:    b2 - b7 <= 0;
