#  MINLP written by GAMS Convert at 01/12/18 13:37:00
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        2        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        2        0        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          8        1        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;
var x3 := 0.1, >= 0, <= 0.2;

minimize obj: -0.00201*i1^4*i2*x3^2;

subject to

e1: -i1^2*i2 >= -675;

e2: -0.1*i1^2*x3^2 >= -0.419;
