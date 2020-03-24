#  MINLP written by GAMS Convert at 01/12/18 13:43:26
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        2        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        2        0        1        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        1        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 15, >= 15, <= 25;
var x2 := 3, >= 3, <= 5.5;

minimize obj: 2*x2^2 + 0.008*i1^3 - 3.2*x2*i1 - 2*i1;

subject to

e1: (-11 + x2^2 - 6*x2 + 0.8*i1)*((3.25*x2 - 0.62*i1)^2 + (-6.35 + 0.2*i1 + x2)
    ^2)*((3.55*x2 - 0.66*i1)^2 + (-6.85 + 0.2*i1 + x2)^2)*((3.6*x2 - 0.7*i1)^2
     + (-7.1 + 0.2*i1 + x2)^2)*((3.8*x2 - 0.82*i1)^2 + (-7.9 + 0.2*i1 + x2)^2)
     = 0;

e2: 0.6*i1 - 0.2*x2*i1 + exp((-3) + x2) <= 1;
