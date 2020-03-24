#  NLP written by GAMS Convert at 01/12/18 13:46:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        3        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        1        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2;
var x3;

minimize obj: x2*x2 + x3*x3;

subject to

e2: x2 - sin(2*x2 + 3*x3) - cos(3*x2 - 5*x3) = 0;

e3: x3 - sin(x2 - 2*x3) + cos(x2 + 3*x3) = 0;
