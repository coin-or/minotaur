#  NLP written by GAMS Convert at 01/12/18 13:34:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         24        2        6       16        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         12       12        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         59       33       26        0


var x1 := 0.25, >= 0.25, <= 8;
var x2 := 0.5, >= 0.5, <= 3.5;
var x3 := 0.5, >= 0.5, <= 1.5;
var x4 := 0.5, >= 0.5, <= 3.5;
var x5 := 0.5, >= 0.5, <= 1.5;
var x6 := 0.5, >= 0.5, <= 3.5;
var x7 := 0.5, >= 0.5, <= 1.5;
var x8 := 0.5, >= 0.5, <= 3.5;
var x9 := 0.5, >= 0.5, <= 1.5;
var x10 >= 0, <= 4;
var x11 >= 0, <= 2;
var x12 >= 0, <= 8;

minimize obj: x12;

subject to

e1:  - x1 + x12 = -3.14159265358979;

e2: -x10*x11 + x1 = 0;

e3: (x2 - x4)*(x2 - x4) + (x3 - x5)*(x3 - x5) >= 1;

e4: (x2 - x6)*(x2 - x6) + (x3 - x7)*(x3 - x7) >= 1;

e5: (x2 - x8)*(x2 - x8) + (x3 - x9)*(x3 - x9) >= 1;

e6: (x4 - x6)*(x4 - x6) + (x5 - x7)*(x5 - x7) >= 1;

e7: (x4 - x8)*(x4 - x8) + (x5 - x9)*(x5 - x9) >= 1;

e8: (x6 - x8)*(x6 - x8) + (x7 - x9)*(x7 - x9) >= 1;

e9:    x2 - x10 <= -0.5;

e10:    x3 - x11 <= -0.5;

e11:    x4 - x10 <= -0.5;

e12:    x5 - x11 <= -0.5;

e13:    x6 - x10 <= -0.5;

e14:    x7 - x11 <= -0.5;

e15:    x8 - x10 <= -0.5;

e16:    x9 - x11 <= -0.5;

e17:    x2 <= 2;

e18:    x3 <= 1;

e19:    x2 - x4 <= 0;

e20:    x2 - x6 <= 0;

e21:    x2 - x8 <= 0;

e22:    x4 - x6 <= 0;

e23:    x4 - x8 <= 0;

e24:    x6 - x8 <= 0;
