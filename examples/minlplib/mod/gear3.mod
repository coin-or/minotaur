#  MINLP written by GAMS Convert at 01/12/18 13:34:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        5        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        5        0        4        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13        9        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 12, >= 12, <= 60;
var x3 := 12, >= 12, <= 60;
var x4 := 12, >= 12, <= 60;
var x5 := 12, >= 12, <= 60;
var i6 integer := 12, >= 12, <= 60;
var i7 integer := 12, >= 12, <= 60;
var i8 integer := 12, >= 12, <= 60;
var i9 integer := 12, >= 12, <= 60;

minimize obj: (0.14427932477276 - x2*x3/(x4*x5))^2;

subject to

e2:    x2 - i6 = 0;

e3:    x3 - i7 = 0;

e4:    x4 - i8 = 0;

e5:    x5 - i9 = 0;
