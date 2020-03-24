#  MINLP written by GAMS Convert at 01/12/18 13:45:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         43        1        0       42        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         64        1        7       56        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        288      190       98        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var i8 integer := 1, >= 0, <= 15;
var i9 integer := 1, >= 0, <= 15;
var i10 integer := 1, >= 0, <= 15;
var i11 integer := 1, >= 0, <= 15;
var i12 integer := 1, >= 0, <= 15;
var i13 integer := 1, >= 0, <= 15;
var i14 integer := 1, >= 0, <= 15;
var i15 integer := 1, >= 0, <= 6;
var i16 integer := 1, >= 0, <= 6;
var i17 integer := 1, >= 0, <= 6;
var i18 integer := 1, >= 0, <= 6;
var i19 integer := 1, >= 0, <= 6;
var i20 integer := 1, >= 0, <= 6;
var i21 integer := 1, >= 0, <= 6;
var i22 integer := 1, >= 0, <= 6;
var i23 integer := 1, >= 0, <= 6;
var i24 integer := 1, >= 0, <= 6;
var i25 integer := 1, >= 0, <= 6;
var i26 integer := 1, >= 0, <= 6;
var i27 integer := 1, >= 0, <= 6;
var i28 integer := 1, >= 0, <= 6;
var i29 integer := 1, >= 0, <= 6;
var i30 integer := 1, >= 0, <= 6;
var i31 integer := 1, >= 0, <= 6;
var i32 integer := 1, >= 0, <= 6;
var i33 integer := 1, >= 0, <= 6;
var i34 integer := 1, >= 0, <= 6;
var i35 integer := 1, >= 0, <= 6;
var i36 integer := 1, >= 0, <= 6;
var i37 integer := 1, >= 0, <= 6;
var i38 integer := 1, >= 0, <= 6;
var i39 integer := 1, >= 0, <= 6;
var i40 integer := 1, >= 0, <= 6;
var i41 integer := 1, >= 0, <= 6;
var i42 integer := 1, >= 0, <= 6;
var i43 integer := 1, >= 0, <= 6;
var i44 integer := 1, >= 0, <= 6;
var i45 integer := 1, >= 0, <= 6;
var i46 integer := 1, >= 0, <= 6;
var i47 integer := 1, >= 0, <= 6;
var i48 integer := 1, >= 0, <= 6;
var i49 integer := 1, >= 0, <= 6;
var i50 integer := 1, >= 0, <= 6;
var i51 integer := 1, >= 0, <= 6;
var i52 integer := 1, >= 0, <= 6;
var i53 integer := 1, >= 0, <= 6;
var i54 integer := 1, >= 0, <= 6;
var i55 integer := 1, >= 0, <= 6;
var i56 integer := 1, >= 0, <= 6;
var i57 integer := 1, >= 0, <= 6;
var i58 integer := 1, >= 0, <= 6;
var i59 integer := 1, >= 0, <= 6;
var i60 integer := 1, >= 0, <= 6;
var i61 integer := 1, >= 0, <= 6;
var i62 integer := 1, >= 0, <= 6;
var i63 integer := 1, >= 0, <= 6;

minimize obj:    0.1*b1 + 0.2*b2 + 0.3*b3 + 0.4*b4 + 0.5*b5 + 0.6*b6 + 0.7*b7
     + i8 + i9 + i10 + i11 + i12 + i13 + i14;

subject to

e2:    550*i15 + 630*i22 + 685*i29 + 720*i36 + 760*i43 + 810*i50 + 850*i57
     <= 3400;

e3:    550*i16 + 630*i23 + 685*i30 + 720*i37 + 760*i44 + 810*i51 + 850*i58
     <= 3400;

e4:    550*i17 + 630*i24 + 685*i31 + 720*i38 + 760*i45 + 810*i52 + 850*i59
     <= 3400;

e5:    550*i18 + 630*i25 + 685*i32 + 720*i39 + 760*i46 + 810*i53 + 850*i60
     <= 3400;

e6:    550*i19 + 630*i26 + 685*i33 + 720*i40 + 760*i47 + 810*i54 + 850*i61
     <= 3400;

e7:    550*i20 + 630*i27 + 685*i34 + 720*i41 + 760*i48 + 810*i55 + 850*i62
     <= 3400;

e8:    550*i21 + 630*i28 + 685*i35 + 720*i42 + 760*i49 + 810*i56 + 850*i63
     <= 3400;

e9:  - 550*i15 - 630*i22 - 685*i29 - 720*i36 - 760*i43 - 810*i50 - 850*i57
     <= -3200;

e10:  - 550*i16 - 630*i23 - 685*i30 - 720*i37 - 760*i44 - 810*i51 - 850*i58
      <= -3200;

e11:  - 550*i17 - 630*i24 - 685*i31 - 720*i38 - 760*i45 - 810*i52 - 850*i59
      <= -3200;

e12:  - 550*i18 - 630*i25 - 685*i32 - 720*i39 - 760*i46 - 810*i53 - 850*i60
      <= -3200;

e13:  - 550*i19 - 630*i26 - 685*i33 - 720*i40 - 760*i47 - 810*i54 - 850*i61
      <= -3200;

e14:  - 550*i20 - 630*i27 - 685*i34 - 720*i41 - 760*i48 - 810*i55 - 850*i62
      <= -3200;

e15:  - 550*i21 - 630*i28 - 685*i35 - 720*i42 - 760*i49 - 810*i56 - 850*i63
      <= -3200;

e16:    i15 + i22 + i29 + i36 + i43 + i50 + i57 <= 6;

e17:    i16 + i23 + i30 + i37 + i44 + i51 + i58 <= 6;

e18:    i17 + i24 + i31 + i38 + i45 + i52 + i59 <= 6;

e19:    i18 + i25 + i32 + i39 + i46 + i53 + i60 <= 6;

e20:    i19 + i26 + i33 + i40 + i47 + i54 + i61 <= 6;

e21:    i20 + i27 + i34 + i41 + i48 + i55 + i62 <= 6;

e22:    i21 + i28 + i35 + i42 + i49 + i56 + i63 <= 6;

e23:    b1 - i8 <= 0;

e24:    b2 - i9 <= 0;

e25:    b3 - i10 <= 0;

e26:    b4 - i11 <= 0;

e27:    b5 - i12 <= 0;

e28:    b6 - i13 <= 0;

e29:    b7 - i14 <= 0;

e30:  - 15*b1 + i8 <= 0;

e31:  - 15*b2 + i9 <= 0;

e32:  - 15*b3 + i10 <= 0;

e33:  - 15*b4 + i11 <= 0;

e34:  - 15*b5 + i12 <= 0;

e35:  - 15*b6 + i13 <= 0;

e36:  - 15*b7 + i14 <= 0;

e37: -(i8*i15 + i9*i16 + i10*i17 + i11*i18 + i12*i19 + i13*i20 + i14*i21)
      <= -8;

e38: -(i8*i22 + i9*i23 + i10*i24 + i11*i25 + i12*i26 + i13*i27 + i14*i28)
      <= -11;

e39: -(i8*i29 + i9*i30 + i10*i31 + i11*i32 + i12*i33 + i13*i34 + i14*i35)
      <= -15;

e40: -(i8*i36 + i9*i37 + i10*i38 + i11*i39 + i12*i40 + i13*i41 + i14*i42)
      <= -5;

e41: -(i8*i43 + i9*i44 + i10*i45 + i11*i46 + i12*i47 + i13*i48 + i14*i49)
      <= -8;

e42: -(i8*i50 + i9*i51 + i10*i52 + i11*i53 + i12*i54 + i13*i55 + i14*i56)
      <= -12;

e43: -(i8*i57 + i9*i58 + i10*i59 + i11*i60 + i12*i61 + i13*i62 + i14*i63)
      <= -6;
