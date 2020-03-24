#  MINLP written by GAMS Convert at 01/12/18 13:32:12
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         19       11        2        6        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         23       17        6        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         75       59       16        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 300;
var x2 >= 0, <= 300;
var x3 >= 0, <= 300;
var x4 >= 0, <= 300;
var x5 >= 0, <= 300;
var x6 >= 0, <= 300;
var x7 >= 0, <= 300;
var x8 >= 0, <= 300;
var x9 >= 0, <= 300;
var x10 >= 0, <= 300;
var x11 >= 0, <= 300;
var x12 >= 0, <= 300;
var x13 >= 0, <= 300;
var x14 >= 0, <= 300;
var x15 >= 0, <= 300;
var x16 >= 0, <= 300;
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;
var b22 binary >= 0, <= 1;

minimize obj: 50*(x1 + x2 + x3 + x4 + x9 + x10 + x11 + x12)**2.5 + 70*(x5 + x6
     + x7 + x8 + x13 + x14 + x15 + x16)**2.5 + 10*x1 + 15*x2 + 20*x3 + 10*x4 + 
    5*x5 + 5*x6 + 30*x7 + 10*x8 + 25*x9 + 20*x10 + 15*x11 + 20*x12 + 30*x13 + 
    10*x14 + 10*x15 + 30*x16 + 2000*b21 + 2500*b22;

subject to

e2:    x1 + x3 + x5 + x7 <= 100;

e3:    x2 + x4 + x6 + x8 <= 200;

e4:    x9 + x11 + x13 + x15 <= 150;

e5:    x10 + x12 + x14 + x16 <= 120;

e6:    x1 + x9 - 120*b17 = 0;

e7:    x2 + x10 - 140*b17 = 0;

e8:    x3 + x11 - 130*b18 = 0;

e9:    x4 + x12 - 180*b18 = 0;

e10:    x5 + x13 - 120*b19 = 0;

e11:    x6 + x14 - 140*b19 = 0;

e12:    x7 + x15 - 130*b20 = 0;

e13:    x8 + x16 - 180*b20 = 0;

e14:    260*b17 + 310*b18 - 2500*b21 <= 0;

e15:    260*b19 + 310*b20 - 3200*b22 <= 0;

e16:    260*b17 + 310*b18 - 50*b21 >= 0;

e17:    260*b19 + 310*b20 - 50*b22 >= 0;

e18:    b17 + b19 = 1;

e19:    b18 + b20 = 1;
