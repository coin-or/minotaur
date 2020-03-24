#  MINLP written by GAMS Convert at 01/12/18 13:41:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        1        0        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          5        3        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 1, <= 5;
var i2 integer := 1, >= 1, <= 5;

minimize obj:    3*i1 + 2*i2;

subject to

e2: -i1*i2 <= -3.5;
