#  NLP written by GAMS Convert at 01/12/18 13:43:19
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         11        1        0       10        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         56       46       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 20;
var x2 >= 0, <= 20;
var x3 >= 0, <= 20;
var x4 >= 0, <= 20;
var x5 >= 0, <= 20;
var x6 >= 0, <= 20;
var x7 >= 0, <= 20;
var x8 >= 0, <= 20;
var x9 >= 0, <= 20;
var x10 >= 0, <= 20;

minimize obj: x1*x6 + 2*x1 + 3*x6 + x2*x7 - 4*x2 - x7 + x3*x8 + 8*x3 - 2*x8 + 
    x4*x9 + 4*x4 - 4*x9 + x5*x10 + 9*x5 + 5*x10;

subject to

e1:  - 8*x1 - 6*x3 + 7*x4 - 7*x5 <= 1;

e2:  - 6*x1 + 2*x2 - 3*x3 + 9*x4 - 3*x5 <= 3;

e3:    6*x1 - 7*x3 - 8*x4 + 2*x5 <= 5;

e4:  - x1 + x2 - 8*x3 - 7*x4 - 5*x5 <= 4;

e5:    4*x1 - 7*x2 + 4*x3 + 5*x4 + x5 <= 0;

e6:    5*x7 - 4*x8 + 9*x9 - 7*x10 <= 0;

e7:    7*x6 + 4*x7 + 3*x8 + 7*x9 + 5*x10 <= 7;

e8:    6*x6 + x7 - 8*x8 + 8*x9 <= 3;

e9:  - 3*x6 + 2*x7 + 7*x8 + x10 <= 6;

e10:  - 2*x6 - 3*x7 + 8*x8 + 5*x9 - 2*x10 <= 2;
