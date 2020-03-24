#  MINLP written by GAMS Convert at 01/12/18 13:33:44
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         12        2        4        6        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         15       11        4        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         39       37        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 29;
var x2 >= 0, <= 29;
var x3 >= 0, <= 29;
var x4 >= 0, <= 29;
var x5 := 1, >= 1, <= 40;
var x6 := 1, >= 1, <= 50;
var x7 := 1, >= 1, <= 40;
var x8 := 1, >= 1, <= 50;
var x9 >= 0, <= 30;
var x10 >= 0, <= 30;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var b13 binary >= 0, <= 1;
var b14 binary >= 0, <= 1;

minimize obj:    2*x9 + 2*x10;

subject to

e2:  - x1 - x5 + x9 >= 0;

e3:  - x2 - x6 + x9 >= 0;

e4:  - x3 - x7 + x10 >= 0;

e5:  - x4 - x8 + x10 >= 0;

e6: 40/x7 - x5 <= 0;

e7: 50/x8 - x6 <= 0;

e8:    x1 - x2 + x5 + 69*b11 <= 69;

e9:  - x1 + x2 + x6 + 79*b12 <= 79;

e10:    x3 - x4 + x7 + 69*b13 <= 69;

e11:  - x3 + x4 + x8 + 79*b14 <= 79;

e12:    b11 + b12 + b13 + b14 = 1;
