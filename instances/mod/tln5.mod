#  MINLP written by GAMS Convert at 01/12/18 13:45:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         31        1        0       30        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         36        1        5       30        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        156      106       50        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var i6 integer := 1, >= 0, <= 15;
var i7 integer := 1, >= 0, <= 15;
var i8 integer := 1, >= 0, <= 15;
var i9 integer := 1, >= 0, <= 15;
var i10 integer := 1, >= 0, <= 15;
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
var i25 integer := 1, >= 0, <= 5;
var i26 integer := 1, >= 0, <= 5;
var i27 integer := 1, >= 0, <= 5;
var i28 integer := 1, >= 0, <= 5;
var i29 integer := 1, >= 0, <= 5;
var i30 integer := 1, >= 0, <= 5;
var i31 integer := 1, >= 0, <= 5;
var i32 integer := 1, >= 0, <= 5;
var i33 integer := 1, >= 0, <= 5;
var i34 integer := 1, >= 0, <= 5;
var i35 integer := 1, >= 0, <= 5;

minimize obj:    0.1*b1 + 0.2*b2 + 0.3*b3 + 0.4*b4 + 0.5*b5 + i6 + i7 + i8 + i9
     + i10;

subject to

e2:    330*i11 + 360*i16 + 370*i21 + 415*i26 + 435*i31 <= 2000;

e3:    330*i12 + 360*i17 + 370*i22 + 415*i27 + 435*i32 <= 2000;

e4:    330*i13 + 360*i18 + 370*i23 + 415*i28 + 435*i33 <= 2000;

e5:    330*i14 + 360*i19 + 370*i24 + 415*i29 + 435*i34 <= 2000;

e6:    330*i15 + 360*i20 + 370*i25 + 415*i30 + 435*i35 <= 2000;

e7:  - 330*i11 - 360*i16 - 370*i21 - 415*i26 - 435*i31 <= -1800;

e8:  - 330*i12 - 360*i17 - 370*i22 - 415*i27 - 435*i32 <= -1800;

e9:  - 330*i13 - 360*i18 - 370*i23 - 415*i28 - 435*i33 <= -1800;

e10:  - 330*i14 - 360*i19 - 370*i24 - 415*i29 - 435*i34 <= -1800;

e11:  - 330*i15 - 360*i20 - 370*i25 - 415*i30 - 435*i35 <= -1800;

e12:    i11 + i16 + i21 + i26 + i31 <= 5;

e13:    i12 + i17 + i22 + i27 + i32 <= 5;

e14:    i13 + i18 + i23 + i28 + i33 <= 5;

e15:    i14 + i19 + i24 + i29 + i34 <= 5;

e16:    i15 + i20 + i25 + i30 + i35 <= 5;

e17:    b1 - i6 <= 0;

e18:    b2 - i7 <= 0;

e19:    b3 - i8 <= 0;

e20:    b4 - i9 <= 0;

e21:    b5 - i10 <= 0;

e22:  - 15*b1 + i6 <= 0;

e23:  - 15*b2 + i7 <= 0;

e24:  - 15*b3 + i8 <= 0;

e25:  - 15*b4 + i9 <= 0;

e26:  - 15*b5 + i10 <= 0;

e27: -(i6*i11 + i7*i12 + i8*i13 + i9*i14 + i10*i15) <= -12;

e28: -(i6*i16 + i7*i17 + i8*i18 + i9*i19 + i10*i20) <= -6;

e29: -(i6*i21 + i7*i22 + i8*i23 + i9*i24 + i10*i25) <= -15;

e30: -(i6*i26 + i7*i27 + i8*i28 + i9*i29 + i10*i30) <= -6;

e31: -(i6*i31 + i7*i32 + i8*i33 + i9*i34 + i10*i35) <= -9;
