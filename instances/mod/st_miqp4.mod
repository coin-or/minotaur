#  MINLP written by GAMS Convert at 01/12/18 13:43:30
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        1        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        4        0        3        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         16       13        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 1;
var i2 integer >= 0, <= 1;
var i3 integer >= 0, <= 1;
var x4 >= 0, <= 1E15;
var x5 >= 0, <= 1E15;
var x6 >= 0, <= 1E15;

minimize obj: 5*x4*x4 + 2*x4 + 5*x5*x5 + 3*x5 + 10*x6*x6 - 500*x6 + 10*i1
     - 4*i2 + 5*i3;

subject to

e1:    x4 + x5 - x6 >= 0;

e2:  - 5*i1 + x4 <= 0;

e3:  - 10*i2 + x5 <= 0;

e4:  - 30*i3 + x6 <= 0;
