#  MINLP written by GAMS Convert at 01/12/18 13:34:22
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        1        0        4        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          5        1        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 12, >= 12, <= 60;
var i2 integer := 12, >= 12, <= 60;
var i3 integer := 12, >= 12, <= 60;
var i4 integer := 12, >= 12, <= 60;

minimize obj: (0.14427932477276 - i1*i2/(i3*i4))^2;
