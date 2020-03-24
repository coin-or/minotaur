#  NLP written by GAMS Convert at 01/12/18 13:43:26
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        3        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          8        8        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        7        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0, <= 1;
var x4 >= 0, <= 1;
var x5 >= -1, <= 1;
var x6 >= -1, <= 1;
var x7 >= 0, <= 3;

minimize obj:    x1 + x2;

subject to

e1: 51.5712*x3*x5 + 20.7324*x3 - 25.7856*x5 + 14.9251*x3*x7 - 22.2988*x7 - 
    10.1409*x6*x7 + 42.3401*x6 - x1 + x2 - 6.6425*x4 = -72.82;

e2:    x3 = 1;
