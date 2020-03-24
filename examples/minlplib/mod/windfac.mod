#  MINLP written by GAMS Convert at 01/12/18 13:52:12
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         14       14        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         15       12        0        3        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         38       14       24        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1.3, >= 1, <= 10;
var i2 integer := 15.4, >= 1, <= 100;
var x3 := 1.5;
var x4 := 1;
var i5 integer := 2.8, >= 1, <= 100;
var x6;
var x7;
var x8;
var x9;
var x10;
var x11;
var x13;
var x14;
var x15 := 0.8, >= 0.8;

minimize obj: x13*x13 + x14*x14;

subject to

e1:  - 12*i1 + i2 = 0;

e2: -12.566370616/i2 + x3 = 0;

e3:  - 0.25*i2 + x4 = 0;

e4:  - x4 + i5 = -1;

e5: sin(0.5*x3)*i1*x6 - sin(0.5*i1*x3) = 0;

e6: -sin(1.570796327*i5/x4) + x9 = 0;

e7: -x9*x6 + x15 = 0;

e8: sin(1.5*x3)*i1*x7 - sin(1.5*i1*x3) = 0;

e9: -sin(4.712388981*i5/x4) + x10 = 0;

e10: -x10*x7 + x13 = 0;

e11: sin(2.5*x3)*i1*x8 - sin(2.5*i1*x3) = 0;

e12: -sin(7.853981635*i5/x4) + x11 = 0;

e13: -x11*x8 + x14 = 0;
