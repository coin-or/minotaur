#  MINLP written by GAMS Convert at 01/12/18 13:36:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        4        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        1        0        4        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21        1       20        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;
var i3 integer := 1, >= 0, <= 200;
var i4 integer := 1, >= 0, <= 200;

minimize obj: 7*i1^2 + 6*i2^2 - 20*i1 - 93.2*i2 + 8*i3^2 - 6*i3*i1 + 4*i3*i2 - 
    67.2*i3 + 6*i4^2 + 2*i4*i1 + 2*i4*i3 - 36.6*i4;

subject to

e1: (-9*i1^2) - 10*i1*i2 - 8*i2^2 - 5*i3^2 - 6*i3*i1 - 10*i3*i2 - 7*i4^2 - 10*
    i4*i1 - 6*i4*i2 - 2*i4*i3 >= -1100;

e2: (-6*i1^2) - 8*i1*i2 - 6*i2^2 - 4*i3^2 - 2*i3*i1 - 2*i3*i2 - 8*i4^2 + 2*i4*
    i1 + 10*i4*i2 >= -440;

e3: (-9*i1^2) - 6*i2^2 - 8*i3^2 + 2*i2*i1 + 2*i3*i2 - 6*i4^2 + 4*i4*i1 + 4*i4*
    i2 - 2*i4*i3 >= -310;

e4: (-8*i1^2) - 4*i2^2 - 9*i3^2 - 7*i4^2 - 2*i2*i1 - 2*i3*i1 - 4*i3*i2 + 6*i4*
    i1 + 2*i4*i2 - 2*i4*i3 >= -460;
