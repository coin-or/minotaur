#  NLP written by GAMS Convert at 01/12/18 13:34:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          2        2        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          2        1        1        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 3, >= -1, <= 4;

minimize obj: (3 + 18*x1^2 - 10*x1 - 13*x1^3 + 2*x1^4)^2;
