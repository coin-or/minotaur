#  NLP written by GAMS Convert at 01/12/18 13:43:20
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13        7        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 99;
var x2 >= 0, <= 99;
var x3 >= 0, <= 99;
var x4 >= 0, <= 99;
var x5 >= 0, <= 99;
var x6 >= 0, <= 99;

minimize obj: 10.5*x1 - 1.5*x1^2 - x2^2 - 3.95*x2 - x3^2 + 3*x3 - 2*x4^2 + 5*x4
     - x5^2 + 1.5*x5 - 2.5*x6^2 - 1.5*x6;

subject to

e1:    x1 + x2 + x3 + x4 + x5 + x6 <= 10000000000;
