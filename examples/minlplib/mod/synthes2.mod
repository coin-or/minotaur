#  MINLP written by GAMS Convert at 01/12/18 13:43:54
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         15        2        0       13        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         12        7        5        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         49       41        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 2;
var x2 >= 0, <= 2;
var x3 >= 0, <= 2;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0, <= 3;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;

minimize obj: exp(x1) - 10*x1 + exp(0.833333*x2) - 15*x2 - 60*log(1 + x4 + x5)
     + 15*x4 + 5*x5 - 15*x3 - 20*x6 + 5*b7 + 8*b8 + 6*b9 + 10*b10 + 6*b11
     + 140;

subject to

e1: -log(1 + x4 + x5) <= 0;

e2: exp(x1) - 10*b7 <= 1;

e3: exp(0.833333*x2) - 10*b8 <= 1;

e4:    1.25*x3 - 10*b9 <= 0;

e5:    x4 + x5 - 10*b10 <= 0;

e6:  - 2*x3 + 2*x6 - 10*b11 <= 0;

e7:  - x1 - x2 - 2*x3 + x4 + 2*x6 <= 0;

e8:  - x1 - x2 - 0.75*x3 + x4 + 2*x6 <= 0;

e9:    x3 - x6 <= 0;

e10:    2*x3 - x4 - 2*x6 <= 0;

e11:  - 0.5*x4 + x5 <= 0;

e12:  - 0.2*x4 - x5 <= 0;

e13:    b7 + b8 = 1;

e14:    b10 + b11 <= 1;
