#  NLP written by GAMS Convert at 01/12/18 13:31:55
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        1        3        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        7        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         19       11        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 5, >= 0;
var x2 := 1, >= 0;
var x3 := 5, >= 1, <= 5;
var x4 >= 0, <= 6;
var x5 := 5, >= 1, <= 5;
var x6 := 10, >= 0, <= 10;

minimize obj: (-25*(-2 + x1)^2) - (-2 + x2)^2 - (-1 + x3)^2 - (-4 + x4)^2 - (-1
     + x5)^2 - (-4 + x6)^2;

subject to

e2: (-3 + x3)^2 + x4 >= 4;

e3: (-3 + x5)^2 + x6 >= 4;

e4:    x1 - 3*x2 <= 2;

e5:  - x1 + x2 <= 2;

e6:    x1 + x2 <= 6;

e7:    x1 + x2 >= 2;
