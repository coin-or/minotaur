#  MINLP written by GAMS Convert at 01/12/18 13:31:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        2        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        3        3        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         15       13        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 1, >= 1, <= 10;
var x2 := 1, >= 1, <= 6;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;

minimize obj:  - 5*x1 + 3*x2;

subject to

e1: 8*x1 - 2*x1**0.5*x2^2 + 11*x2 + 2*x2^2 - 2*x2**0.5 <= 39;

e2:    x1 - x2 <= 3;

e3:    3*x1 + 2*x2 <= 24;

e4:    x2 - b3 - 2*b4 - 4*b5 = 1;

e5:    b4 + b5 <= 1;
