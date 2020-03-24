#  MINLP written by GAMS Convert at 01/12/18 13:36:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        5        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        1        0        5        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         31        1       30        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;
var i3 integer := 1, >= 0, <= 200;
var i4 integer := 1, >= 0, <= 200;
var i5 integer := 1, >= 0, <= 200;

minimize obj: 7*i1^2 + 6*i2^2 + 12*i1 - 77.2*i2 + 8*i3^2 - 6*i3*i1 + 4*i3*i2 - 
    19.2*i3 + 6*i4^2 + 2*i4*i1 + 2*i4*i3 - 36.6*i4 + 7*i5^2 - 4*i5*i1 - 2*i5*i2
     - 6*i5*i3 - 69.4*i5;

subject to

e1: (-9*i1^2) - 10*i1*i2 - 8*i2^2 - 5*i3^2 - 6*i3*i1 - 10*i3*i2 - 7*i4^2 - 10*
    i4*i1 - 6*i4*i2 - 2*i4*i3 - 2*i5*i2 - 7*i5^2 >= -1430;

e2: (-6*i1^2) - 8*i1*i2 - 6*i2^2 - 4*i3^2 - 2*i3*i1 - 2*i3*i2 - 8*i4^2 + 2*i4*
    i1 + 10*i4*i2 - 2*i5*i1 - 6*i5*i2 + 6*i5*i4 + 7*i5^2 >= -1150;

e3: (-9*i1^2) - 6*i2^2 - 8*i3^2 + 2*i2*i1 + 2*i3*i2 - 6*i4^2 + 4*i4*i1 + 4*i4*
    i2 - 2*i4*i3 - 6*i5*i1 - 2*i5*i2 + 4*i5*i4 + 6*i5^2 >= -850;

e4: (-8*i1^2) - 4*i2^2 - 9*i3^2 - 7*i4^2 - 2*i2*i1 - 2*i3*i1 - 4*i3*i2 + 6*i4*
    i1 + 2*i4*i2 - 2*i4*i3 - 6*i5*i1 - 4*i5*i2 - 2*i5*i3 + 6*i5*i4 + 6*i5^2
     >= -1125;

e5: 2*i2*i1 - 4*i1^2 - 5*i2^2 - 6*i3*i1 - 8*i3^2 - 2*i4*i1 + 6*i4*i2 - 2*i4*i3
     - 6*i4^2 - 4*i5*i1 + 2*i5*i2 - 6*i5*i3 - 8*i5*i4 - 7*i5^2 >= -1030;
