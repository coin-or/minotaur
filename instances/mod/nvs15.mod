#  MINLP written by GAMS Convert at 01/12/18 13:36:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        1        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        1        0        3        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        4        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 100, >= 0, <= 200;
var i2 integer := 100, >= 0, <= 200;
var i3 integer := 100, >= 0, <= 200;

minimize obj: 2*i1^2 - 8*i1 + 2*i2^2 - 6*i2 + i3^2 - 4*i3 + 2*i1*i2 + 2*i1*i3
     + 9;

subject to

e1:  - i1 - i2 - 2*i3 >= -3;
