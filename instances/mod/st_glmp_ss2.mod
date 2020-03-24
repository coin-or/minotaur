#  NLP written by GAMS Convert at 01/12/18 13:43:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        4        1        4        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        6        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         22       20        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 7;
var x2 >= 0, <= 6;
var x3;
var x4;
var x5;

minimize obj: x4*x5 + x3;

subject to

e1:    x1 - 2*x2 <= 100;

e2:  - x1 - 2*x2 <= 100;

e3:  - x1 + 2*x2 >= 5;

e4:  - x1 + 2*x2 <= 8;

e5:    x1 + 2*x2 <= 12;

e6:    x1 - x3 = 0;

e7:    2*x1 - 3*x2 - x4 = -13;

e8:    x1 + x2 - x5 = 1;
