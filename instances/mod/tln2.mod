#  MINLP written by GAMS Convert at 01/12/18 13:45:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         13        1        0       12        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        1        2        6        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         33       25        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var i3 integer := 1, >= 0, <= 15;
var i4 integer := 1, >= 0, <= 15;
var i5 integer := 1, >= 0, <= 5;
var i6 integer := 1, >= 0, <= 5;
var i7 integer := 1, >= 0, <= 5;
var i8 integer := 1, >= 0, <= 5;

minimize obj:    0.1*b1 + 0.2*b2 + i3 + i4;

subject to

e2:    460*i5 + 570*i7 <= 1900;

e3:    460*i6 + 570*i8 <= 1900;

e4:  - 460*i5 - 570*i7 <= -1700;

e5:  - 460*i6 - 570*i8 <= -1700;

e6:    i5 + i7 <= 5;

e7:    i6 + i8 <= 5;

e8:    b1 - i3 <= 0;

e9:    b2 - i4 <= 0;

e10:  - 15*b1 + i3 <= 0;

e11:  - 15*b2 + i4 <= 0;

e12: -(i3*i5 + i4*i6) <= -8;

e13: -(i3*i7 + i4*i8) <= -7;
