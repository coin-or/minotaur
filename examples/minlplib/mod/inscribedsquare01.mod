#  NLP written by GAMS Convert at 09/25/19 17:16:40
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        9        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        9        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         27       17       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := -3.14159265358979, >= -3.14159265358979, <= 3.14159265358979;
var x3 := -1.5707963267949, >= -3.14159265358979, <= 3.14159265358979;
var x4 >= -3.14159265358979, <= 3.14159265358979;
var x5 := 1.5707963267949, >= -3.14159265358979, <= 3.14159265358979;
var x6 := 1.22464679914735E-16;
var x7 := 3.84734138744358E-16;
var x8 := 1, >= 0;
var x9 := 1, >= 0;

maximize obj: x8^2 + x9^2;

subject to

e2: sin(x2)*cos(x2) - x6 = 0;

e3: sin(x2)*x2 - x7 = 0;

e4: sin(x3)*cos(x3) - x6 - x8 = 0;

e5: sin(x3)*x3 - x7 - x9 = 0;

e6: sin(x4)*cos(x4) - x6 + x9 = 0;

e7: sin(x4)*x4 - x7 - x8 = 0;

e8: sin(x5)*cos(x5) - x6 - x8 + x9 = 0;

e9: sin(x5)*x5 - x7 - x8 - x9 = 0;
