#  NLP written by GAMS Convert at 01/12/18 13:46:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          2        2        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          3        1        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 1, >= -2, <= 5;

minimize obj: sin(11*x1) + cos(13*x1) - sin(17*x1) - cos(19*x1);

subject to

e2: 5*sin(x1) - x1 <= 0;
