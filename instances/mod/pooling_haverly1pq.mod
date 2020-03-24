#  NLP written by GAMS Convert at 01/12/18 13:38:44
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         14        6        0        8        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         43       35        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 100;
var x5 >= 0, <= 200;
var x6 >= 0, <= 100;
var x7 >= 0, <= 200;
var x8 >= 0, <= 100;
var x9 >= 0, <= 200;
var x10 >= 0, <= 100;
var x11 >= 0, <= 200;

minimize obj:    x4 - 5*x5 - 3*x8 - 9*x9 + 7*x10 + x11;

subject to

e2:    x8 + x9 <= 300;

e3:    x10 + x11 <= 300;

e4:    x4 + x5 <= 300;

e5:    x8 + x9 + x10 + x11 <= 300;

e6:    x4 + x8 + x10 <= 100;

e7:    x5 + x9 + x11 <= 200;

e8:  - 0.5*x4 + 0.5*x8 - 1.5*x10 <= 0;

e9:    0.5*x5 + 1.5*x9 - 0.5*x11 <= 0;

e10:    x2 + x3 = 1;

e11: -x2*x6 + x8 = 0;

e12: -x2*x7 + x9 = 0;

e13: -x3*x6 + x10 = 0;

e14: -x3*x7 + x11 = 0;
