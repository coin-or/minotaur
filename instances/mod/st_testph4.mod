#  MINLP written by GAMS Convert at 01/12/18 13:43:36
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         11        1        0       10        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        1        0        3        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         28       25        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 100;
var i2 integer >= 0, <= 100;
var i3 integer >= 0, <= 100;

minimize obj: 3.5*i1*i1 - 35*i1 + 0.5*i2*i2 + 3*i2 + 2*i3*i3 + 4*i3;

subject to

e1:    i1 <= 4;

e2:    i2 <= 4;

e3:    i3 <= 4;

e4:    2*i1 + 3*i2 + 4*i3 <= 35;

e5:    2*i1 + 3*i2 - 4*i3 <= 19;

e6:    2*i1 - 3*i2 + 4*i3 <= 23;

e7:  - 2*i1 + 3*i2 + 4*i3 <= 27;

e8:    2*i1 - 3*i2 - 4*i3 <= 7;

e9:  - 2*i1 - 3*i2 + 4*i3 <= 15;

e10:  - 2*i1 + 3*i2 - 4*i3 <= 11;
