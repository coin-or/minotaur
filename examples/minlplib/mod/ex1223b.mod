#  MINLP written by GAMS Convert at 01/12/18 13:31:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         10        1        0        9        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          8        4        4        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         32       15       17        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 10;
var x2 >= 0, <= 10;
var x3 >= 0, <= 10;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;

minimize obj: (-1 + b4)^2 + (-2 + b5)^2 + (-1 + b6)^2 - log(1 + b7) + (-1 + x1)
    ^2 + (-2 + x2)^2 + (-3 + x3)^2;

subject to

e1:    x1 + x2 + x3 + b4 + b5 + b6 <= 5;

e2: b6^2 + x1^2 + x2^2 + x3^2 <= 5.5;

e3:    x1 + b4 <= 1.2;

e4:    x2 + b5 <= 1.8;

e5:    x3 + b6 <= 2.5;

e6:    x1 + b7 <= 1.2;

e7: b5^2 + x2^2 <= 1.64;

e8: b6^2 + x3^2 <= 4.25;

e9: b5^2 + x3^2 <= 4.64;
