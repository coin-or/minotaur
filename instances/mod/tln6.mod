#  MINLP written by GAMS Convert at 01/12/18 13:45:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         37        1        0       36        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         49        1        6       42        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        217      145       72        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var i7 integer := 1, >= 0, <= 16;
var i8 integer := 1, >= 0, <= 16;
var i9 integer := 1, >= 0, <= 16;
var i10 integer := 1, >= 0, <= 16;
var i11 integer := 1, >= 0, <= 16;
var i12 integer := 1, >= 0, <= 16;
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
var i36 integer := 1, >= 0, <= 5;
var i37 integer := 1, >= 0, <= 5;
var i38 integer := 1, >= 0, <= 5;
var i39 integer := 1, >= 0, <= 5;
var i40 integer := 1, >= 0, <= 5;
var i41 integer := 1, >= 0, <= 5;
var i42 integer := 1, >= 0, <= 5;
var i43 integer := 1, >= 0, <= 5;
var i44 integer := 1, >= 0, <= 5;
var i45 integer := 1, >= 0, <= 5;
var i46 integer := 1, >= 0, <= 5;
var i47 integer := 1, >= 0, <= 5;
var i48 integer := 1, >= 0, <= 5;

minimize obj:    0.1*b1 + 0.2*b2 + 0.3*b3 + 0.4*b4 + 0.5*b5 + 0.6*b6 + i7 + i8
     + i9 + i10 + i11 + i12;

subject to

e2:    330*i13 + 360*i19 + 380*i25 + 430*i31 + 490*i37 + 530*i43 <= 2200;

e3:    330*i14 + 360*i20 + 380*i26 + 430*i32 + 490*i38 + 530*i44 <= 2200;

e4:    330*i15 + 360*i21 + 380*i27 + 430*i33 + 490*i39 + 530*i45 <= 2200;

e5:    330*i16 + 360*i22 + 380*i28 + 430*i34 + 490*i40 + 530*i46 <= 2200;

e6:    330*i17 + 360*i23 + 380*i29 + 430*i35 + 490*i41 + 530*i47 <= 2200;

e7:    330*i18 + 360*i24 + 380*i30 + 430*i36 + 490*i42 + 530*i48 <= 2200;

e8:  - 330*i13 - 360*i19 - 380*i25 - 430*i31 - 490*i37 - 530*i43 <= -2100;

e9:  - 330*i14 - 360*i20 - 380*i26 - 430*i32 - 490*i38 - 530*i44 <= -2100;

e10:  - 330*i15 - 360*i21 - 380*i27 - 430*i33 - 490*i39 - 530*i45 <= -2100;

e11:  - 330*i16 - 360*i22 - 380*i28 - 430*i34 - 490*i40 - 530*i46 <= -2100;

e12:  - 330*i17 - 360*i23 - 380*i29 - 430*i35 - 490*i41 - 530*i47 <= -2100;

e13:  - 330*i18 - 360*i24 - 380*i30 - 430*i36 - 490*i42 - 530*i48 <= -2100;

e14:    i13 + i19 + i25 + i31 + i37 + i43 <= 5;

e15:    i14 + i20 + i26 + i32 + i38 + i44 <= 5;

e16:    i15 + i21 + i27 + i33 + i39 + i45 <= 5;

e17:    i16 + i22 + i28 + i34 + i40 + i46 <= 5;

e18:    i17 + i23 + i29 + i35 + i41 + i47 <= 5;

e19:    i18 + i24 + i30 + i36 + i42 + i48 <= 5;

e20:    b1 - i7 <= 0;

e21:    b2 - i8 <= 0;

e22:    b3 - i9 <= 0;

e23:    b4 - i10 <= 0;

e24:    b5 - i11 <= 0;

e25:    b6 - i12 <= 0;

e26:  - 16*b1 + i7 <= 0;

e27:  - 16*b2 + i8 <= 0;

e28:  - 16*b3 + i9 <= 0;

e29:  - 16*b4 + i10 <= 0;

e30:  - 16*b5 + i11 <= 0;

e31:  - 16*b6 + i12 <= 0;

e32: -(i7*i13 + i8*i14 + i9*i15 + i10*i16 + i11*i17 + i12*i18) <= -8;

e33: -(i7*i19 + i8*i20 + i9*i21 + i10*i22 + i11*i23 + i12*i24) <= -16;

e34: -(i7*i25 + i8*i26 + i9*i27 + i10*i28 + i11*i29 + i12*i30) <= -12;

e35: -(i7*i31 + i8*i32 + i9*i33 + i10*i34 + i11*i35 + i12*i36) <= -7;

e36: -(i7*i37 + i8*i38 + i9*i39 + i10*i40 + i11*i41 + i12*i42) <= -14;

e37: -(i7*i43 + i8*i44 + i9*i45 + i10*i46 + i11*i47 + i12*i48) <= -16;
