#  MINLP written by GAMS Convert at 01/12/18 13:33:54
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         16        7        6        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         16       13        3        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         39       33        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var x4 := 100;
var x5 := 100;
var x6 := 100;
var x7 := 50, >= 50, <= 700;
var x8 := 50, >= 50, <= 700;
var x9 := 50, >= 50, <= 700;
var x10 >= 0;
var x11 >= 0;
var x12 >= 0;
var x13 >= 0, <= 4000;
var x14 >= 0, <= 4000;
var x15 := 2000, >= 2000, <= 4000;

minimize obj: 0.0025*x7^2 + 6*x7 + 0.0025*x8^2 + 6*x8 + 0.0025*x9^2 + 6*x9
     + 900;

subject to

e2:  - 100*b1 + x4 >= 0;

e3:  - 100*b2 + x5 >= 0;

e4:  - 100*b3 + x6 >= 0;

e5:  - 500*b1 + x4 <= 0;

e6:  - 500*b2 + x5 <= 0;

e7:  - 500*b3 + x6 <= 0;

e8:    x10 + x13 = 3500;

e9:    x11 - x13 + x14 = 500;

e10:    x12 - x14 + x15 = 500;

e11:    x4 + x7 >= 400;

e12:    x5 + x8 >= 900;

e13:    x6 + x9 >= 700;

e14: -(0.005*x4^2 + x4) - 50*b1 + x10 = 0;

e15: -(0.005*x5^2 + x5) - 50*b2 + x11 = 0;

e16: -(0.005*x6^2 + x6) - 50*b3 + x12 = 0;
