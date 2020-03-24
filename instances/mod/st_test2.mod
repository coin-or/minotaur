#  MINLP written by GAMS Convert at 01/12/18 13:43:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        1        0        6        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         15       10        5        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 1;
var i2 integer >= 0, <= 1;
var i3 integer >= 0, <= 1;
var i4 integer >= 0, <= 1;
var i5 integer >= 0, <= 1;
var i6 integer >= 0, <= 1E15;

minimize obj: 0.5*i1*i1 + 10.5*i1 + 0.25*i2*i2 - 7.5*i2 + 1.5*i3*i3 - 3.5*i3 + 
    0.5*i4*i4 + 2.5*i4 + 0.5*i5*i5 - 1.5*i5 + 10*i6;

subject to

e1:    6*i1 + 3*i2 + 3*i3 + 2*i4 + i5 <= 6.5;

e2:    10*i1 + 10*i3 + i6 <= 20;
