#  NLP written by GAMS Convert at 01/12/18 13:43:25
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        1        4        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         27       13       14        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 0.31;
var x2 >= 0, <= 0.046;
var x3 >= 0, <= 0.068;
var x4 >= 0, <= 0.042;
var x5 >= 0, <= 0.028;
var x6 >= 0, <= 0.0134;

minimize obj:    4.3*x1 + 31.8*x2 + 63.3*x3 + 15.8*x4 + 68.5*x5 + 4.7*x6;

subject to

e1: 17.1*x1 - 169*x1*x3 + 204.2*x3 - 3580*x3*x5 + 623.4*x5 - 3810*x4*x6 + 212.3
    *x4 + 1495.5*x6 - 18500*x4*x6 + 38.2*x2 >= 4.97;

e2: 17.9*x1 - 139*x1*x3 + 113.9*x3 - 2450*x4*x5 + 169.7*x4 + 337.8*x5 - 16600*
    x4*x6 + 1385.2*x6 - 17200*x5*x6 + 36.8*x2 >= -1.88;

e3: 26000*x4*x5 - 70*x4 - 819*x5 - 273*x2 >= -69.08;

e4: 159.9*x1 - 14000*x1*x6 + 2198*x6 - 311*x2 + 587*x4 + 391*x5 >= -118.02;
