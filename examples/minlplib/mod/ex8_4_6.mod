#  NLP written by GAMS Convert at 01/12/18 13:32:09
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        9        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         15       15        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         65        9       56        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 0.171747132, >= 0, <= 1;
var x2 := 0.843266708, >= 0, <= 1;
var x3 := 0.550375356, >= 0, <= 1;
var x4 := 0.301137904, >= 0, <= 1;
var x5 := 0.292212117, >= 0, <= 1;
var x6 := 0.224052867, >= 0, <= 1;
var x7 := 0.349830504, >= 0, <= 1;
var x8 := 0.856270347, >= 0, <= 1;
var x9 := 0.355, >= -10, <= 10;
var x10 := 2.007, >= -10, <= 10;
var x11 := -4.575, >= -10, <= 10;
var x12 := 0.015, >= 0, <= 0.5;
var x13 := 0.11, >= 0, <= 0.5;
var x14 := 0.285, >= 0, <= 0.5;

minimize obj: ((-0.1622 + x1)/x1)^2 + ((-0.6791 + x2)/x2)^2 + ((-0.679 + x3)/x3
    )^2 + ((-0.3875 + x4)/x4)^2 + ((-0.1822 + x5)/x5)^2 + ((-0.1249 + x6)/x6)^2
     + ((-0.0857 + x7)/x7)^2 + ((-0.0616 + x8)/x8)^2;

subject to

e2: exp(-4*x12)*x9 + exp(-4*x13)*x10 + exp(-4*x14)*x11 - x1 = 0;

e3: exp(-8*x12)*x9 + exp(-8*x13)*x10 + exp(-8*x14)*x11 - x2 = 0;

e4: exp(-12*x12)*x9 + exp(-12*x13)*x10 + exp(-12*x14)*x11 - x3 = 0;

e5: exp(-24*x12)*x9 + exp(-24*x13)*x10 + exp(-24*x14)*x11 - x4 = 0;

e6: exp(-48*x12)*x9 + exp(-48*x13)*x10 + exp(-48*x14)*x11 - x5 = 0;

e7: exp(-72*x12)*x9 + exp(-72*x13)*x10 + exp(-72*x14)*x11 - x6 = 0;

e8: exp(-94*x12)*x9 + exp(-94*x13)*x10 + exp(-94*x14)*x11 - x7 = 0;

e9: exp(-118*x12)*x9 + exp(-118*x13)*x10 + exp(-118*x14)*x11 - x8 = 0;
