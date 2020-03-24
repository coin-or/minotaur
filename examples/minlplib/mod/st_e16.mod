#  NLP written by GAMS Convert at 01/12/18 13:43:23
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         10       10        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         13       13        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         35       17       18        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 10;
var x2 >= 0, <= 10;
var x3 >= 0, <= 10;
var x4 >= 0, <= 10;
var x5 >= 0, <= 10;
var x6 >= 0, <= 10;
var x7 >= 0, <= 10;
var x8 >= 0, <= 10;
var x9 := 100, >= 100, <= 290;
var x10 := 100, >= 100, <= 310;
var x11 := 100, >= 100, <= 290;
var x12 := 100, >= 100, <= 330;

minimize obj: 1200*(800/(258.333333333333 + 2.5*(0.666666666666667*((320 - x10)
    *(300 - x9))**0.5 - 0.166666666666667*x9 - 0.166666666666667*x10)))**0.6 + 
    1200*(5000/(106.666666666667 + 0.666666666666667*((340 - x12)*(300 - x11))
    **0.5 - 0.166666666666667*x11 - 0.166666666666667*x12))**0.6;

subject to

e1:    x1 + x2 = 10;

e2:    x1 - x3 + x6 = 0;

e3:    x2 - x4 + x5 = 0;

e4:  - x3 + x5 + x7 = 0;

e5:  - x4 + x6 + x8 = 0;

e6: x12*x6 - x9*x3 + 100*x1 = 0;

e7: x10*x5 - x11*x4 + 100*x2 = 0;

e8: x3*(x10 - x9) = 800;

e9: x4*(x12 - x11) = 1000;
