#  NLP written by GAMS Convert at 01/12/18 13:34:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          4        1        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 500;
var x3 := -150;
var x4 := -0.2, >= -5, <= 5;

minimize obj: (127 - exp(-5*x4)*x3 - x2)^2 + (151 - exp(-3*x4)*x3 - x2)^2 + (
    379 - exp(-x4)*x3 - x2)^2 + (421 - exp(5*x4)*x3 - x2)^2 + (460 - exp(3*x4)*
    x3 - x2)^2 + (426 - exp(x4)*x3 - x2)^2;
