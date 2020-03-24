#  NLP written by GAMS Convert at 01/12/18 13:43:26
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13        1       12        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 0.5, >= 0.5, <= 1;
var x2 := 0.5, >= 0.5, <= 1;
var x3 := 0.5, >= 0.5, <= 1;
var x4 := 0.5, >= 0.5, <= 1;

minimize obj: 200*x1**0.6 + 200*x2**0.6 + 200*x3**0.6 + 300*x4**0.6;

subject to

e1: -(-(1 - x1)^2*x3*(1 - x4)^2 - (1 - (1 - (1 - x1)*(1 - x4))*x2)^2*(1 - x3))
     <= 0.1;

e2: (-(1 - x1)^2*x3*(1 - x4)^2) - (1 - (1 - (1 - x1)*(1 - x4))*x2)^2*(1 - x3)
     <= 0;
