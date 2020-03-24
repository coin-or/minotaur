#  MINLP written by GAMS Convert at 01/12/18 13:34:23
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        6        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         12        9        3        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         28       26        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0, <= 5;
var x7 >= 0;
var x8 >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;

minimize obj:    1.8*x1 + 7*x5 + x6 + 1.2*x7 - 11*x8 + 3.5*b9 + b10 + 1.5*b11;

subject to

e2: -log(1 + x2) + x6 = 0;

e3: -1.2*log(1 + x3) + x7 = 0;

e4:  - 0.9*x4 + x8 = 0;

e5:  - x4 + x5 + x6 + x7 = 0;

e6:    x1 - x2 - x3 = 0;

e7:    x4 - 5*b9 <= 0;

e8:    x2 - 5*b10 <= 0;

e9:    x3 - 5*b11 <= 0;
