#  NLP written by GAMS Convert at 01/12/18 13:43:27
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         21        1        0       20        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         25       25        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        121       97       24        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;
var x7 >= 0;
var x8 >= 0;
var x9 >= 0;
var x10 >= 0;
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

minimize obj: 300*x1 - 7*x1^2 - 4*x2^2 + 270*x2 - 6*x3^2 + 460*x3 - 8*x4^2 + 
    800*x4 - 12*x5^2 + 740*x5 - 9*x6^2 + 600*x6 - 14*x7^2 + 540*x7 - 7*x8^2 + 
    380*x8 - 13*x9^2 + 300*x9 - 12*x10^2 + 490*x10 - 8*x11^2 + 380*x11 - 4*x12^
    2 + 760*x12 - 7*x13^2 + 430*x13 - 9*x14^2 + 250*x14 - 16*x15^2 + 390*x15 - 
    8*x16^2 + 600*x16 - 4*x17^2 + 210*x17 - 10*x18^2 + 830*x18 - 21*x19^2 + 470
    *x19 - 13*x20^2 + 680*x20 - 17*x21^2 + 360*x21 - 9*x22^2 + 290*x22 - 8*x23^
    2 + 400*x23 - 4*x24^2 + 310*x24;

subject to

e1:  - x1 - x5 - x9 - x13 - x17 - x21 <= -29;

e2:    x1 + x5 + x9 + x13 + x17 + x21 <= 29;

e3:  - x2 - x6 - x10 - x14 - x18 - x22 <= -41;

e4:    x2 + x6 + x10 + x14 + x18 + x22 <= 41;

e5:  - x3 - x7 - x11 - x15 - x19 - x23 <= -13;

e6:    x3 + x7 + x11 + x15 + x19 + x23 <= 13;

e7:  - x4 - x8 - x12 - x16 - x20 - x24 <= -21;

e8:    x4 + x8 + x12 + x16 + x20 + x24 <= 21;

e9:  - x1 - x2 - x3 - x4 <= -8;

e10:    x1 + x2 + x3 + x4 <= 8;

e11:  - x5 - x6 - x7 - x8 <= -24;

e12:    x5 + x6 + x7 + x8 <= 24;

e13:  - x9 - x10 - x11 - x12 <= -20;

e14:    x9 + x10 + x11 + x12 <= 20;

e15:  - x13 - x14 - x15 - x16 <= -24;

e16:    x13 + x14 + x15 + x16 <= 24;

e17:  - x17 - x18 - x19 - x20 <= -16;

e18:    x17 + x18 + x19 + x20 <= 16;

e19:  - x21 - x22 - x23 - x24 <= -12;

e20:    x21 + x22 + x23 + x24 <= 12;
