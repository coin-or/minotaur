#  NLP written by GAMS Convert at 01/12/18 13:43:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        4        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        6        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        7        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 15834;
var x2 >= 0, <= 36250;
var x3 >= 0, <= 10000;
var x4 := 100, >= 100, <= 300;
var x5 := 100, >= 100, <= 400;

minimize obj:    x1 + x2 + x3;

subject to

e1: 100000*x4 - 120*x1*(300 - x4) = 10000000;

e2: 100000*x5 - 80*x2*(400 - x5) - 100000*x4 = 0;

e3:  - 4000*x3 - 100000*x5 = -50000000;
