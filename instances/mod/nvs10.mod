#  MINLP written by GAMS Convert at 01/12/18 13:36:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        2        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        1        0        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        1        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;

minimize obj: 7*i1^2 + 6*i2^2 - 35*i1 - 80.4*i2;

subject to

e1: (-9*i1^2) - 10*i1*i2 - 8*i2^2 >= -583;

e2: (-6*i1^2) - 8*i1*i2 - 6*i2^2 >= -441;
