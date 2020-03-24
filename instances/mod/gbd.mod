#  MINLP written by GAMS Convert at 01/12/18 13:34:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        2        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        2        3        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         17       16        1        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 0.2, >= 0.2, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;

minimize obj: 5*x2^2 + b3 + b4 + b5;

subject to

e2:    3*x2 - b3 - b4 <= 0;

e3:  - x2 + 0.1*b4 + 0.25*b5 <= 0;

e4:    b3 + b4 + b5 >= 2;

e5:    b3 + b4 + 2*b5 >= 2;
