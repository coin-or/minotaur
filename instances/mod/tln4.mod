#  MINLP written by GAMS Convert at 01/12/18 13:45:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         25        1        0       24        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         25        1        4       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        105       73       32        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var i5 integer := 1, >= 0, <= 12;
var i6 integer := 1, >= 0, <= 12;
var i7 integer := 1, >= 0, <= 12;
var i8 integer := 1, >= 0, <= 12;
var i9 integer := 1, >= 0, <= 5;
var i10 integer := 1, >= 0, <= 5;
var i11 integer := 1, >= 0, <= 5;
var i12 integer := 1, >= 0, <= 5;
var i13 integer := 1, >= 0, <= 5;
var i14 integer := 1, >= 0, <= 5;
var i15 integer := 1, >= 0, <= 5;
var i16 integer := 1, >= 0, <= 5;
var i17 integer := 1, >= 0, <= 5;
var i18 integer := 1, >= 0, <= 5;
var i19 integer := 1, >= 0, <= 5;
var i20 integer := 1, >= 0, <= 5;
var i21 integer := 1, >= 0, <= 5;
var i22 integer := 1, >= 0, <= 5;
var i23 integer := 1, >= 0, <= 5;
var i24 integer := 1, >= 0, <= 5;

minimize obj:    0.1*b1 + 0.2*b2 + 0.3*b3 + 0.4*b4 + i5 + i6 + i7 + i8;

subject to

e2:    330*i9 + 360*i13 + 385*i17 + 415*i21 <= 1900;

e3:    330*i10 + 360*i14 + 385*i18 + 415*i22 <= 1900;

e4:    330*i11 + 360*i15 + 385*i19 + 415*i23 <= 1900;

e5:    330*i12 + 360*i16 + 385*i20 + 415*i24 <= 1900;

e6:  - 330*i9 - 360*i13 - 385*i17 - 415*i21 <= -1700;

e7:  - 330*i10 - 360*i14 - 385*i18 - 415*i22 <= -1700;

e8:  - 330*i11 - 360*i15 - 385*i19 - 415*i23 <= -1700;

e9:  - 330*i12 - 360*i16 - 385*i20 - 415*i24 <= -1700;

e10:    i9 + i13 + i17 + i21 <= 5;

e11:    i10 + i14 + i18 + i22 <= 5;

e12:    i11 + i15 + i19 + i23 <= 5;

e13:    i12 + i16 + i20 + i24 <= 5;

e14:    b1 - i5 <= 0;

e15:    b2 - i6 <= 0;

e16:    b3 - i7 <= 0;

e17:    b4 - i8 <= 0;

e18:  - 12*b1 + i5 <= 0;

e19:  - 12*b2 + i6 <= 0;

e20:  - 12*b3 + i7 <= 0;

e21:  - 12*b4 + i8 <= 0;

e22: -(i5*i9 + i6*i10 + i7*i11 + i8*i12) <= -8;

e23: -(i5*i13 + i6*i14 + i7*i15 + i8*i16) <= -7;

e24: -(i5*i17 + i6*i18 + i7*i19 + i8*i20) <= -12;

e25: -(i5*i21 + i6*i22 + i7*i23 + i8*i24) <= -11;
