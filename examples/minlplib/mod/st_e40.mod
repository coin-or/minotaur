#  MINLP written by GAMS Convert at 01/12/18 13:43:26
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        5        0        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        2        0        3        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         23       17        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 1, <= 12;
var i2 integer := 1, >= 1, <= 12;
var i3 integer := 1, >= 1, <= 12;
var x4;

minimize obj:    2*i1 + i2 + 1.4142135*i3;

subject to

e1: (-0.15*i1*i2) - 0.14142135*i2*i3 - 0.1319*i1*i3 + x4 = 0;

e2:    1.7317*i2 + 1.03366*i3 - x4 <= 0;

e3:    0.634*i1 + 2.828*i3 - x4 <= 0;

e4:    0.5*i1 - 2*i2 - x4 <= 0;

e5:  - 0.5*i1 + 2*i2 - x4 <= 0;

e6: (-1 + i1)*(-2 + i1)*(-3 + i1)*(-5 + i1)*(-8 + i1)*(-10 + i1)*(-12 + i1)
     = 0;

e7: (-1 + i2)*(-2 + i2)*(-3 + i2)*(-5 + i2)*(-8 + i2)*(-10 + i2)*(-12 + i2)
     = 0;

e8: (-1 + i3)*(-2 + i3)*(-3 + i3)*(-5 + i3)*(-8 + i3)*(-10 + i3)*(-12 + i3)
     = 0;
