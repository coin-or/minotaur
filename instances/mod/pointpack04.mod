#  NLP written by GAMS Convert at 01/12/18 13:38:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         10        0        0       10        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        9        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         38       14       24        0


var x1 := 0.5, >= 0.5, <= 1;
var x2 := 0.5, >= 0.5, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 1;
var x5 >= 0, <= 1;
var x6 >= 0, <= 1;
var x7 >= 0, <= 1;
var x8 >= 0, <= 1;
var x9;

maximize obj: x9;

subject to

e1: 2*x1*x2 - x1*x1 - x2*x2 - x5*x5 + 2*x5*x6 - x6*x6 + x9 <= 0;

e2: 2*x1*x3 - x1*x1 - x3*x3 - x5*x5 + 2*x5*x7 - x7*x7 + x9 <= 0;

e3: 2*x1*x4 - x1*x1 - x4*x4 - x5*x5 + 2*x5*x8 - x8*x8 + x9 <= 0;

e4: 2*x2*x3 - x2*x2 - x3*x3 - x6*x6 + 2*x6*x7 - x7*x7 + x9 <= 0;

e5: 2*x2*x4 - x2*x2 - x4*x4 - x6*x6 + 2*x6*x8 - x8*x8 + x9 <= 0;

e6: 2*x3*x4 - x3*x3 - x4*x4 - x7*x7 + 2*x7*x8 - x8*x8 + x9 <= 0;

e7:  - x5 + x6 <= 0;

e8:  - x1 + x2 <= 0;

e9:  - x2 + x3 <= 0;

e10:  - x3 + x4 <= 0;
