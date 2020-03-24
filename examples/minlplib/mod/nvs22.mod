#  MINLP written by GAMS Convert at 01/12/18 13:37:00
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         10        5        5        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        5        0        4        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         31        7       24        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 2, >= 1, <= 200;
var i2 integer := 2, >= 1, <= 200;
var i3 integer := 2, >= 1, <= 20;
var i4 integer := 2, >= 1, <= 20;
var x5 := 1;
var x6 := 1;
var x7 := 1;
var x8 := 1;

minimize obj: 1.10471*i3**2*i4 + 0.04811*i1*i2*(14 + i4);

subject to

e1: -4243.28147100424/(i3*i4) + x5 = 0;

e2: -sqrt(0.25*i4^2 + (0.5*i1 + 0.5*i3)^2) + x7 = 0;

e3: -(59405.9405940594 + 2121.64073550212*i4)*x7/(i3*i4*(0.0833333333333333*i4^
    2 + (0.5*i1 + 0.5*i3)^2)) + x6 = 0;

e4: -0.5*i4/x7 + x8 = 0;

e5: -sqrt(x5^2 + 2*x5*x6*x8 + x6^2) >= -13600;

e6: -504000/(i1^2*i2) >= -30000;

e7:    i2 - i3 >= 0;

e8: 0.0204744897959184*sqrt(10000000000000*i2^3*i1*i1*i2^3)*(1 - 
    0.0282346219657891*i1) >= 6000;

e9: -2.1952/(i1^3*i2) >= -0.25;
