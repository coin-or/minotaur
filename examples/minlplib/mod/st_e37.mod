#  NLP written by GAMS Convert at 01/12/18 13:43:26
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      2        2        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        3        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 100;
var x2 >= 0, <= 100;
var x4 := 1, >= 1, <= 1;
var x5 := 1, >= 1, <= 1;

minimize obj: (-1.9837 + x4 + x5)^2 + (-0.8393 + exp(-x1)*x4 + exp(-x2)*x5)^2
     + (-0.4305 + exp(-2*x1)*x4 + exp(-2*x2)*x5)^2 + (-0.2441 + exp(-3*x1)*x4
     + exp(-3*x2)*x5)^2 + (-0.1248 + exp(-4*x1)*x4 + exp(-4*x2)*x5)^2 + (-
    0.0981 + exp(-5*x1)*x4 + exp(-5*x2)*x5)^2 + (-0.0549 + exp(-6*x1)*x4 + exp(
    -6*x2)*x5)^2 + (-0.0174 + exp(-7*x1)*x4 + exp(-7*x2)*x5)^2 + (-0.0249 + 
    exp(-8*x1)*x4 + exp(-8*x2)*x5)^2 + (-0.0154 + exp(-9*x1)*x4 + exp(-9*x2)*x5
    )^2 + (-0.0127 + exp(-10*x1)*x4 + exp(-10*x2)*x5)^2;

subject to

e1:    x1 - x2 <= 0;
