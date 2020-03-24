#  MINLP written by GAMS Convert at 01/12/18 13:43:30
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        1        1        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        1        0        4        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        9        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 1;
var i2 integer >= 0, <= 1;
var i3 integer >= 0, <= 10000000000;
var i4 integer >= 0, <= 10000000000;

minimize obj: 4*i3*i3 - 3*i3 + 2*i4*i4 - 10*i4 + 4*i1 + 5*i2;

subject to

e1:  - 10*i1 + i3 <= 0;

e2:  - 20*i2 + i4 <= 0;

e3:    i3 + i4 >= 5;
