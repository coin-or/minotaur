#  MINLP written by GAMS Convert at 01/12/18 13:34:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        5        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         29        5       24        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         33       29        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 12, >= 12, <= 60;
var x3 := 12, >= 12, <= 60;
var x4 := 12, >= 12, <= 60;
var x5 := 12, >= 12, <= 60;
var b6 binary >= 0, <= 1;
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
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;
var b22 binary >= 0, <= 1;
var b23 binary >= 0, <= 1;
var b24 binary >= 0, <= 1;
var b25 binary >= 0, <= 1;
var b26 binary >= 0, <= 1;
var b27 binary >= 0, <= 1;
var b28 binary >= 0, <= 1;
var b29 binary >= 0, <= 1;

minimize obj: (0.14427932477276 - x2*x3/(x4*x5))^2;

subject to

e2:    x2 - b6 - 2*b7 - 4*b8 - 8*b9 - 16*b10 - 32*b11 = 0;

e3:    x3 - b12 - 2*b13 - 4*b14 - 8*b15 - 16*b16 - 32*b17 = 0;

e4:    x4 - b18 - 2*b19 - 4*b20 - 8*b21 - 16*b22 - 32*b23 = 0;

e5:    x5 - b24 - 2*b25 - 4*b26 - 8*b27 - 16*b28 - 32*b29 = 0;
