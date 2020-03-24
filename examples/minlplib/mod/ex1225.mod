#  MINLP written by GAMS Convert at 01/12/18 13:31:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         11        3        0        8        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        3        6        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         27       25        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 1, >= 1, <= 5;
var x2 := 1, >= 1, <= 5;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;

minimize obj:    7*x1 + 10*x2;

subject to

e2: x1**1.2*x2**1.7 - 7*x1 - 9*x2 <= -24;

e3:  - x1 - 2*x2 <= -5;

e4:  - 3*x1 + x2 <= 1;

e5:    4*x1 - 3*x2 <= 11;

e6:    x1 - b3 - 2*b4 - 4*b5 = 1;

e7:    x2 - b6 - 2*b7 - 4*b8 = 1;

e8:    b3 + b5 <= 1;

e9:    b6 + b8 <= 1;

e10:    b4 + b5 <= 1;

e11:    b7 + b8 <= 1;
