#  NLP written by GAMS Convert at 01/12/18 13:43:25
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        5        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         10       10        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         30       23        7        0
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
var x9 := 0.01, >= 0.01, <= 0.03;

minimize obj:    6*x1 + 16*x2 - 9*x5 + 10*x6 + 10*x7 - 15*x8;

subject to

e1:    x1 + x2 - x3 - x4 = 0;

e2: -x9*(x3 + x4) + 0.03*x1 + 0.01*x2 = 0;

e3:    x3 - x5 + x6 = 0;

e4:    x4 + x7 - x8 = 0;

e5: x9*x3 - 0.025*x5 + 0.02*x6 <= 0;

e6: x9*x4 + 0.02*x7 - 0.015*x8 <= 0;
