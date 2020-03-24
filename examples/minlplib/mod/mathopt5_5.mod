#  NLP written by GAMS Convert at 01/12/18 13:35:00
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


var x1 := 5, >= -10, <= 10;

minimize obj: sin(1 + 2*x1) + 2*sin(2 + 3*x1) + 3*sin(3 + 4*x1) + 4*sin(4 + 5*
    x1) + 5*sin(5 + 6*x1);
