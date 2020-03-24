#  NLP written by GAMS Convert at 01/12/18 13:31:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          3        1        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= -5;
var x2 <= 5;

minimize obj: 2*x1^2 - 1.05*x1^4 + 0.166666666666667*x1^6 - x1*x2 + x2^2;
