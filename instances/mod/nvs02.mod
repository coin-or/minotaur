#  MINLP written by GAMS Convert at 01/12/18 13:36:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        4        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        4        0        5        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         20        4       16        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 100, >= 0, <= 200;
var i2 integer := 100, >= 0, <= 200;
var i3 integer := 100, >= 0, <= 200;
var i4 integer := 100, >= 0, <= 200;
var i5 integer := 100, >= 0, <= 200;
var x6 >= 0, <= 92;
var x7 := 90, >= 90, <= 110;
var x8 := 20, >= 20, <= 25;

minimize obj: 9.99999999999999e-5*(5.3578547*i3^2 + 0.8356891*i1*i5 + 37.293239
    *i1) + 5.9207859;

subject to

e1: -(0.0056858*i2*i5 + 0.0006262*i1*i4 - 0.0022053*i3*i5) + x6 = 85.334407;

e2: -(0.0071317*i2*i5 + 0.0029955*i1*i2 + 0.0021813*i3^2) + x7 = 80.51249;

e3: -(0.0047026*i3*i5 + 0.0012547*i1*i3 + 0.0019085*i3*i4) + x8 = 9.300961;
