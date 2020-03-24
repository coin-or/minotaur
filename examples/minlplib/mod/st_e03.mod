#  NLP written by GAMS Convert at 01/12/18 13:43:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        7        1        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         28       17       11        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 1, >= 1, <= 2000;
var x2 := 1, >= 1, <= 16000;
var x3 >= 0, <= 120;
var x4 := 1, >= 1, <= 5000;
var x5 >= 0, <= 2000;
var x6 := 85, >= 85, <= 93;
var x7 := 90, >= 90, <= 95;
var x8 := 3, >= 3, <= 12;
var x9 := 1.2, >= 1.2, <= 4;
var x10 := 145, >= 145, <= 162;

minimize obj: -0.063*x4*x7 + 5.04*x1 + 0.035*x2 + 10*x3 + 3.36*x5;

subject to

e1:    x1 - 1.22*x4 + x5 = 0;

e2:    x9 + 0.222*x10 = 35.82;

e3:    3*x7 - x10 = 133;

e4: 0.038*x8^2 - 1.098*x8 - 0.325*x6 + x7 = 57.425;

e5: x4*x9*x6 + 1000*x3*x6 - 98000*x3 = 0;

e6: -x1*x8 + x2 + x5 = 0;

e7: 0.13167*x8*x1 + 1.12*x1 - 0.00667*x8^2*x1 - x4 >= 0;
