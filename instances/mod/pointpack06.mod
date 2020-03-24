#  NLP written by GAMS Convert at 01/12/18 13:38:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         21        0        0       21        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         13       13        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         87       27       60        0


var x1 := 0.5, >= 0.5, <= 1;
var x2 := 0.5, >= 0.5, <= 1;
var x3 := 0.5, >= 0.5, <= 1;
var x4 >= 0, <= 1;
var x5 >= 0, <= 1;
var x6 >= 0, <= 1;
var x7 >= 0, <= 1;
var x8 >= 0, <= 1;
var x9 >= 0, <= 1;
var x10 >= 0, <= 1;
var x11 >= 0, <= 1;
var x12 >= 0, <= 1;
var x13;

maximize obj: x13;

subject to

e1: 2*x1*x2 - x1*x1 - x2*x2 - x7*x7 + 2*x7*x8 - x8*x8 + x13 <= 0;

e2: 2*x1*x3 - x1*x1 - x3*x3 - x7*x7 + 2*x7*x9 - x9*x9 + x13 <= 0;

e3: 2*x1*x4 - x1*x1 - x4*x4 - x7*x7 + 2*x7*x10 - x10*x10 + x13 <= 0;

e4: 2*x1*x5 - x1*x1 - x5*x5 - x7*x7 + 2*x7*x11 - x11*x11 + x13 <= 0;

e5: 2*x1*x6 - x1*x1 - x6*x6 - x7*x7 + 2*x7*x12 - x12*x12 + x13 <= 0;

e6: 2*x2*x3 - x2*x2 - x3*x3 - x8*x8 + 2*x8*x9 - x9*x9 + x13 <= 0;

e7: 2*x2*x4 - x2*x2 - x4*x4 - x8*x8 + 2*x8*x10 - x10*x10 + x13 <= 0;

e8: 2*x2*x5 - x2*x2 - x5*x5 - x8*x8 + 2*x8*x11 - x11*x11 + x13 <= 0;

e9: 2*x2*x6 - x2*x2 - x6*x6 - x8*x8 + 2*x8*x12 - x12*x12 + x13 <= 0;

e10: 2*x3*x4 - x3*x3 - x4*x4 - x9*x9 + 2*x9*x10 - x10*x10 + x13 <= 0;

e11: 2*x3*x5 - x3*x3 - x5*x5 - x9*x9 + 2*x9*x11 - x11*x11 + x13 <= 0;

e12: 2*x3*x6 - x3*x3 - x6*x6 - x9*x9 + 2*x9*x12 - x12*x12 + x13 <= 0;

e13: 2*x4*x5 - x4*x4 - x5*x5 - x10*x10 + 2*x10*x11 - x11*x11 + x13 <= 0;

e14: 2*x4*x6 - x4*x4 - x6*x6 - x10*x10 + 2*x10*x12 - x12*x12 + x13 <= 0;

e15: 2*x5*x6 - x5*x5 - x6*x6 - x11*x11 + 2*x11*x12 - x12*x12 + x13 <= 0;

e16:  - x7 + x8 <= 0;

e17:  - x1 + x2 <= 0;

e18:  - x2 + x3 <= 0;

e19:  - x3 + x4 <= 0;

e20:  - x4 + x5 <= 0;

e21:  - x5 + x6 <= 0;
