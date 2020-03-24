#  NLP written by GAMS Convert at 01/12/18 13:43:22
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        3        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        4        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        3        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 34;
var x2 >= 0, <= 17;
var x3 >= 0, <= 300;

minimize obj: 35*x1**0.6 + 35*x2**0.6;

subject to

e1: 600*x1 - x1*x3 - 50*x3 = -5000;

e2:    600*x2 + 50*x3 = 15000;
