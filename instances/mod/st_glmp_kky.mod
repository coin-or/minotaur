#  NLP written by GAMS Convert at 01/12/18 13:43:28
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         14        6        0        8        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          8        8        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         37       33        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 10;
var x2 >= 0, <= 10;
var x3;
var x4;
var x5;
var x6;
var x7;

minimize obj: x4*x5 + x6*x7 + x3;

subject to

e1:  - 5*x1 + 8*x2 <= 24;

e2:  - 5*x1 - 8*x2 <= 100;

e3:  - 6*x1 + 3*x2 <= 100;

e4:  - 4*x1 - 5*x2 <= -10;

e5:    5*x1 - 8*x2 <= 100;

e6:    5*x1 + 8*x2 <= 44;

e7:    6*x1 - 3*x2 <= 15;

e8:    4*x1 + 5*x2 <= 100;

e10:    3*x1 - 4*x2 - x3 = 0;

e11:    x1 + 2*x2 - x4 = 1.5;

e12:    2*x1 - x2 - x5 = -4;

e13:    x1 - 2*x2 - x6 = -8.5;

e14:    2*x1 + x2 - x7 = 1;
