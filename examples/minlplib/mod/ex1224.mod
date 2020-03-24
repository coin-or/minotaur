#  MINLP written by GAMS Convert at 01/12/18 13:31:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        3        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         12        4        8        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         31       25        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 0.997;
var x2 >= 0, <= 0.9985;
var x3 >= 0, <= 0.9988;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;

minimize obj: -x1*x2*x3;

subject to

e2: -log(1 - x1) - 2.30258509299405*b4 - 1.6094379124341*b5
     - 1.89711998488588*b6 = 0;

e3: -log(1 - x2) - 2.99573227355399*b7 - 1.6094379124341*b8
     - 1.89711998488588*b9 = 0;

e4: -log(1 - x3) - 3.91202300542815*b10 - 2.81341071676004*b11 <= 0;

e5:  - b4 - b5 - b6 <= -1;

e6:  - b7 - b8 - b9 <= -1;

e7:  - b10 - b11 <= -1;

e8:    3*b4 + b5 + 2*b6 + 3*b7 + 2*b8 + b9 + 3*b10 + 2*b11 <= 10;
