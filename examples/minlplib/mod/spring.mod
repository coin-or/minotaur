#  MINLP written by GAMS Convert at 01/12/18 13:42:55
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        6        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         18        6       11        1        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         44       30       14        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 0.414, >= 0.414;
var x2 := 0.207, >= 0.207;
var x3 := 0.00178571428571429, >= 0.00178571428571429, <= 0.02;
var i4 integer := 1, >= 1, <= 100;
var x5 := 1.1, >= 1.1;
var x6;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var b13 binary >= 0, <= 1;
var b14 binary >= 0, <= 1;
var b15 binary >= 0, <= 1;
var b16 binary >= 0, <= 1;
var b17 binary >= 0, <= 1;

minimize obj: (1.570796327 + 0.7853981635*i4)*x1*x2^2;

subject to

e2: -x1/x2 + x5 = 0;

e3: -((-1 + 4*x5)/(-4 + 4*x5) + 0.615/x5) + x6 = 0;

e4: 2546.47908913782*x6*x5/x2^2 <= 189000;

e5: -6.95652173913044e-7*x5**3*i4/x2 + x3 = 0;

e6: (2.1 + 1.05*i4)*x2 + 1000*x3 <= 14;

e7:    x1 + x2 <= 3;

e8:    x2 - 0.207*b7 - 0.225*b8 - 0.244*b9 - 0.263*b10 - 0.283*b11 - 0.307*b12
     - 0.331*b13 - 0.362*b14 - 0.394*b15 - 0.4375*b16 - 0.5*b17 = 0;

e9:    b7 + b8 + b9 + b10 + b11 + b12 + b13 + b14 + b15 + b16 + b17 = 1;
