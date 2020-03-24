#  MINLP written by GAMS Convert at 01/12/18 13:43:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        1        0        5        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        7        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 1;
var i2 integer >= 0, <= 1;
var i3 integer >= 0, <= 1;
var i4 integer >= 0, <= 1;
var i5 integer >= 0, <= 1;

minimize obj: 50*i1*i1 + 42*i1 + 50*i2*i2 - 44*i2 + 50*i4*i4 - 47*i4 + 50*i5*i5
     - 47.5*i5 + 45*i3;

subject to

e1:    20*i1 + 12*i2 + 11*i3 + 7*i4 + 4*i5 <= 40;
