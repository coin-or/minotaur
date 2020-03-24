#  NLP written by GAMS Convert at 01/12/18 13:31:51
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         14        6        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= -5, <= 5;
var x2 >= -5, <= 5;
var x3;

minimize obj:    x3;

subject to

e2: 2*x2^2 + 4*x1*x2 - 42*x1 + 4*x1^3 - x3 <= 14;

e3: (-2*x2^2) - 4*x1*x2 + 42*x1 - 4*x1^3 - x3 <= -14;

e4: 2*x1^2 + 4*x1*x2 - 26*x2 + 4*x2^3 - x3 <= 22;

e5: (-2*x1^2) - 4*x1*x2 + 26*x2 - 4*x2^3 - x3 <= -22;
