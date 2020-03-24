#  NLP written by GAMS Convert at 01/12/18 13:35:00
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


var x1 := -0.655668942, >= -3, <= 3;
var x2 := 0.346914252, >= -3, <= 3;

minimize obj: exp(sin(50*x1)) + sin(60*exp(x2)) + sin(70*sin(x1)) + sin(sin(80*
    x2)) - sin(10*x1 + 10*x2) + 0.25*(x1^2 + x2^2);
