#  NLP written by GAMS Convert at 01/12/18 13:43:22
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        6        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         32       25        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 300;
var x2 >= 0, <= 300;
var x3 >= 0, <= 100;
var x4 >= 0, <= 200;
var x5 >= 0, <= 100;
var x6 >= 0, <= 300;
var x7 >= 0, <= 100;
var x8 >= 0, <= 200;
var x9 >= 0, <= 200;
var x10 := 1, >= 1, <= 3;

minimize obj:    6*x1 + 16*x2 - 9*x5 + 10*x6 - 15*x9;

subject to

e1:    x1 + x2 - x3 - x4 = 0;

e2:    x3 - x5 + x7 = 0;

e3:    x4 + x8 - x9 = 0;

e4:  - x6 + x7 + x8 = 0;

e5: x10*x3 - 2.5*x5 + 2*x7 <= 0;

e6: x10*x4 + 2*x8 - 1.5*x9 <= 0;

e7: -x10*(x3 + x4) + 3*x1 + x2 = 0;
