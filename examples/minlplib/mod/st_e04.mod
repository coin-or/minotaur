#  NLP written by GAMS Convert at 01/12/18 13:43:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        2        1        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          9        4        5        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 15.1;
var x2 := 14.7, >= 14.7, <= 94.2;
var x3 >= 0, <= 5371;
var x4 >= -459.67, <= 80;

minimize obj: 400*x1**0.9 + 22*(-14.7 + x2)**1.2 + x3 + 1000;

subject to

e1: x3*x1 + 144*x4 >= 11520;

e2: -exp(11.86 - 3950/(460 + x4)) + x2 = 0;
