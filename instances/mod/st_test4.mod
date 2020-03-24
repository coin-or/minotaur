#  MINLP written by GAMS Convert at 01/12/18 13:43:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        1        0        6        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         37       35        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 1E15;
var i2 integer >= 0, <= 1E15;
var i3 integer >= 0, <= 1E15;
var i4 integer >= 0, <= 1;
var i5 integer >= 0, <= 1;
var i6 integer >= 0, <= 2;

minimize obj: 0.5*i1*i1 + 6.5*i1 + 7*i6*i6 - i6 - i2 - 2*i3 + 3*i4 - 2*i5;

subject to

e1:    i1 + 2*i2 + 8*i3 + i4 + 3*i5 + 5*i6 <= 16;

e2:  - 8*i1 - 4*i2 - 2*i3 + 2*i4 + 4*i5 - i6 <= -1;

e3:    2*i1 + 0.5*i2 + 0.2*i3 - 3*i4 - i5 - 4*i6 <= 24;

e4:    0.2*i1 + 2*i2 + 0.1*i3 - 4*i4 + 2*i5 + 2*i6 <= 12;

e5:  - 0.1*i1 - 0.5*i2 + 2*i3 + 5*i4 - 5*i5 + 3*i6 <= 3;
