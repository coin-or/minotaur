#  NLP written by GAMS Convert at 01/12/18 13:34:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        5        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         43       19       24        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 10;
var x2 := -10;
var x3 := 10;
var x4 := 10;
var x5 := 10;
var x6 := -10;

minimize obj: (x1 + x2)^2 + (x3 - x5)^2 + (x6 - x4)^2 + 2*(x1 + x3 - x4)^2 + (
    x2 - x1 + x3 - x4)^2 + 10*sin(x1 + x5 - x6)^2;

subject to

e2: x1^2 - sin(x2) - x4 + x5 + x6 = 0;

e3: x1*x3 - x2*x4*x1 - sin((-x1) - x3 + x6) - x5 = 0;

e4: x2*x6*cos(x5) - sin(x3*x4) + x2 - x5 = 0;

e5: x1*x2 - x3^2 - x4*x5 - x6^2 = 0;

e6:    2*x1 + 5*x2 + x3 + x4 <= 1;

e7:    3*x1 - 2*x2 + x3 - 4*x4 <= 0;

e8:    x1 + x2 + x3 + x4 + x5 + x6 <= 2;
