#  MINLP written by GAMS Convert at 01/12/18 13:14:00
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        3        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        5        4        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         24       21        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 0.302884615384618, >= 0;
var x2 := 0.0865384615384593, >= 0;
var x3 := 0.504807692307693, >= 0;
var x4 := 0.10576923076923, >= 0;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;

minimize obj: x1*(4*x1 + 3*x2 - x3) + x2*(3*x1 + 6*x2 + x3) + x3*(x2 - x1 + 10*
    x3);

subject to

e1:    x1 + x2 + x3 + x4 = 1;

e2:    8*x1 + 9*x2 + 12*x3 + 7*x4 = 10;

e4:    x1 - b6 <= 0;

e5:    x2 - b7 <= 0;

e6:    x3 - b8 <= 0;

e7:    x4 - b9 <= 0;

e8:    b6 + b7 + b8 + b9 <= 3;
