#  MINLP written by GAMS Convert at 01/12/18 13:43:26
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        1        1        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        3        0        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        5        6        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 18, >= 18, <= 100;
var i2 integer := 10, >= 10, <= 100;
var x3 := 40, >= 40, <= 80;
var x4 := 20, >= 20, <= 60;

minimize obj: 0.0389*i1*x3*x4 + 0.1111312*x3^2*i2 + 0.012348046875*i1^2*x4 + 
    0.0775*i1^2*x3;

subject to

e1:  - 0.0625*i1 + 0.0193*x3 <= 0;

e2:  - 0.0625*i2 + 0.00954*x3 <= 0;

e3: 3.1415927*(x3^2*x4 + 1.33333333333333*x3^3) >= 1296000;
