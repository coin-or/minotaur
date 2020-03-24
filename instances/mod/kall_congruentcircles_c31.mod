#  NLP written by GAMS Convert at 01/12/18 13:34:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         16        2        3       11        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         10       10        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         37       23       14        0


var x1 := 0.25, >= 0.25, <= 32;
var x2 := 0.5, >= 0.5, <= 7.5;
var x3 := 0.5, >= 0.5, <= 3.5;
var x4 := 0.5, >= 0.5, <= 7.5;
var x5 := 0.5, >= 0.5, <= 3.5;
var x6 := 0.5, >= 0.5, <= 7.5;
var x7 := 0.5, >= 0.5, <= 3.5;
var x8 >= 0, <= 8;
var x9 >= 0, <= 4;
var x10 >= 0, <= 32;

minimize obj: x10;

subject to

e1:  - x1 + x10 = -2.35619449019234;

e2: -x8*x9 + x1 = 0;

e3: (x2 - x4)*(x2 - x4) + (x3 - x5)*(x3 - x5) >= 1;

e4: (x2 - x6)*(x2 - x6) + (x3 - x7)*(x3 - x7) >= 1;

e5: (x4 - x6)*(x4 - x6) + (x5 - x7)*(x5 - x7) >= 1;

e6:    x2 - x8 <= -0.5;

e7:    x3 - x9 <= -0.5;

e8:    x4 - x8 <= -0.5;

e9:    x5 - x9 <= -0.5;

e10:    x6 - x8 <= -0.5;

e11:    x7 - x9 <= -0.5;

e12:    x2 <= 4;

e13:    x3 <= 2;

e14:    x2 - x4 <= 0;

e15:    x2 - x6 <= 0;

e16:    x4 - x6 <= 0;
