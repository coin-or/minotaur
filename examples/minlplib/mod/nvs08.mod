#  MINLP written by GAMS Convert at 01/12/18 13:36:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        1        3        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        2        0        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13        6        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;
var x3 := 1, >= 0.001, <= 200;

minimize obj: (-3 + i1)^2 + (-2 + i2)^2 + (4 + x3)^2;

subject to

e1: sqrt(x3) + i1 + 2*i2 >= 10;

e2: 0.240038406144983*i1^2 - i2 + 0.255036980362153*x3 >= -3;

e3: i2^2 - 1/(x3^3*sqrt(x3)) - 4*i1 >= -12;
