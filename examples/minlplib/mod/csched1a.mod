#  MINLP written by GAMS Convert at 01/12/18 13:30:48
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         23       13        3        7        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         29       14       15        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         78       71        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 := 1, >= 0.01, <= 4;
var x5 := 1, >= 0.01, <= 4;
var x6 := 1, >= 0.01, <= 4;
var x7 >= 0;
var x8 >= 0;
var x9 >= 0;
var x10 >= 0;
var x11 >= 0;
var x12 >= 0;
var x13 := 100, >= 0;
var b14 binary >= 0, <= 1;
var b15 binary >= 0, <= 1;
var b16 binary >= 0, <= 1;
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;
var b22 binary >= 0, <= 1;
var b23 binary >= 0, <= 1;
var b24 binary >= 0, <= 1;
var b25 binary >= 0, <= 1;
var b26 binary >= 0, <= 1;
var b27 binary >= 0, <= 1;
var b28 binary >= 0, <= 1;

minimize obj: -((416000 - 416000*exp(-0.1*x1/x4))*x4 + 37440*x1 - 100*x4 + (
    124615.384615385 - 124615.384615385*exp(-0.13*x2/x5))*x5 + 9000*x2 - 90*x5
     + (278666.666666667 - 278666.666666667*exp(-0.09*x3/x6))*x6 + 15840*x3 - 
    80*x6)/x13;

subject to

e2:  - 1300*x1 + x7 + 350*x13 = 0;

e3:  - 1000*x2 + x8 + 300*x13 = 0;

e4:  - 1100*x3 + x9 + 300*x13 = 0;

e5:    x7 - 300*x13 <= 0;

e6:    x8 - 300*x13 <= 0;

e7:    x9 - 300*x13 <= 0;

e8:    x4 - 0.01*b14 - b15 - 2*b16 - 3*b17 - 4*b18 = 0;

e9:    x5 - 0.01*b19 - b20 - 2*b21 - 3*b22 - 4*b23 = 0;

e10:    x6 - 0.01*b24 - b25 - 2*b26 - 3*b27 - 4*b28 = 0;

e11:  - b14 - b15 - b16 - b17 - b18 = -1;

e12:  - b19 - b20 - b21 - b22 - b23 = -1;

e13:  - b24 - b25 - b26 - b27 - b28 = -1;

e14:  - x1 - 2*x4 + x10 = 0;

e15:  - x2 - 3*x5 + x11 = 0;

e16:  - x3 - 3*x6 + x12 = 0;

e17:    x10 + x11 + x12 - x13 <= 0;

e18:    x1 + 150*b14 <= 150;

e19:    x2 + 150*b19 <= 150;

e20:    x3 + 150*b24 <= 150;

e21:    x4 >= 1;

e22:    x5 >= 1;

e23:    x6 >= 1;
