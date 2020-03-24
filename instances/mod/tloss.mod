#  MINLP written by GAMS Convert at 01/12/18 13:45:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         54        1       13       40        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         49        1        6       42        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        303      231       72        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 5;
var i2 integer >= 0, <= 5;
var i3 integer >= 0, <= 5;
var i4 integer >= 0, <= 5;
var i5 integer >= 0, <= 5;
var i6 integer >= 0, <= 5;
var i7 integer >= 0, <= 5;
var i8 integer >= 0, <= 5;
var i9 integer >= 0, <= 5;
var i10 integer >= 0, <= 5;
var i11 integer >= 0, <= 5;
var i12 integer >= 0, <= 5;
var i13 integer >= 0, <= 5;
var i14 integer >= 0, <= 5;
var i15 integer >= 0, <= 5;
var i16 integer >= 0, <= 5;
var i17 integer >= 0, <= 5;
var i18 integer >= 0, <= 5;
var i19 integer >= 0, <= 5;
var i20 integer >= 0, <= 5;
var i21 integer >= 0, <= 5;
var i22 integer >= 0, <= 5;
var i23 integer >= 0, <= 5;
var i24 integer >= 0, <= 5;
var i25 integer >= 0, <= 5;
var i26 integer >= 0, <= 5;
var i27 integer >= 0, <= 5;
var i28 integer >= 0, <= 5;
var i29 integer >= 0, <= 5;
var i30 integer >= 0, <= 5;
var i31 integer >= 0, <= 5;
var i32 integer >= 0, <= 5;
var i33 integer >= 0, <= 5;
var i34 integer >= 0, <= 5;
var i35 integer >= 0, <= 5;
var i36 integer >= 0, <= 5;
var b37 binary >= 0, <= 1;
var b38 binary >= 0, <= 1;
var b39 binary >= 0, <= 1;
var b40 binary >= 0, <= 1;
var b41 binary >= 0, <= 1;
var b42 binary >= 0, <= 1;
var i43 integer >= 0, <= 15;
var i44 integer >= 0, <= 12;
var i45 integer >= 0, <= 8;
var i46 integer >= 0, <= 7;
var i47 integer >= 0, <= 4;
var i48 integer >= 0, <= 2;

minimize obj:    0.1*b37 + 0.2*b38 + 0.3*b39 + 0.4*b40 + 0.5*b41 + 0.6*b42
     + i43 + i44 + i45 + i46 + i47 + i48;

subject to

e2: i43*i1 + i44*i2 + i45*i3 + i46*i4 + i47*i5 + i48*i6 >= 8;

e3: i43*i7 + i44*i8 + i45*i9 + i46*i10 + i47*i11 + i48*i12 >= 16;

e4: i43*i13 + i44*i14 + i45*i15 + i46*i16 + i47*i17 + i48*i18 >= 12;

e5: i43*i19 + i44*i20 + i45*i21 + i46*i22 + i47*i23 + i48*i24 >= 7;

e6: i43*i25 + i44*i26 + i45*i27 + i46*i28 + i47*i29 + i48*i30 >= 14;

e7: i43*i31 + i44*i32 + i45*i33 + i46*i34 + i47*i35 + i48*i36 >= 16;

e8:    330*i1 + 360*i7 + 380*i13 + 430*i19 + 490*i25 + 530*i31 - 2100*b37 >= 0;

e9:    330*i2 + 360*i8 + 380*i14 + 430*i20 + 490*i26 + 530*i32 - 2100*b38 >= 0;

e10:    330*i3 + 360*i9 + 380*i15 + 430*i21 + 490*i27 + 530*i33 - 2100*b39
      >= 0;

e11:    330*i4 + 360*i10 + 380*i16 + 430*i22 + 490*i28 + 530*i34 - 2100*b40
      >= 0;

e12:    330*i5 + 360*i11 + 380*i17 + 430*i23 + 490*i29 + 530*i35 - 2100*b41
      >= 0;

e13:    330*i6 + 360*i12 + 380*i18 + 430*i24 + 490*i30 + 530*i36 - 2100*b42
      >= 0;

e14:    330*i1 + 360*i7 + 380*i13 + 430*i19 + 490*i25 + 530*i31 - 2200*b37
      <= 0;

e15:    330*i2 + 360*i8 + 380*i14 + 430*i20 + 490*i26 + 530*i32 - 2200*b38
      <= 0;

e16:    330*i3 + 360*i9 + 380*i15 + 430*i21 + 490*i27 + 530*i33 - 2200*b39
      <= 0;

e17:    330*i4 + 360*i10 + 380*i16 + 430*i22 + 490*i28 + 530*i34 - 2200*b40
      <= 0;

e18:    330*i5 + 360*i11 + 380*i17 + 430*i23 + 490*i29 + 530*i35 - 2200*b41
      <= 0;

e19:    330*i6 + 360*i12 + 380*i18 + 430*i24 + 490*i30 + 530*i36 - 2200*b42
      <= 0;

e20:  - i1 - i7 - i13 - i19 - i25 - i31 + b37 <= 0;

e21:  - i2 - i8 - i14 - i20 - i26 - i32 + b38 <= 0;

e22:  - i3 - i9 - i15 - i21 - i27 - i33 + b39 <= 0;

e23:  - i4 - i10 - i16 - i22 - i28 - i34 + b40 <= 0;

e24:  - i5 - i11 - i17 - i23 - i29 - i35 + b41 <= 0;

e25:  - i6 - i12 - i18 - i24 - i30 - i36 + b42 <= 0;

e26:    i1 + i7 + i13 + i19 + i25 + i31 - 5*b37 <= 0;

e27:    i2 + i8 + i14 + i20 + i26 + i32 - 5*b38 <= 0;

e28:    i3 + i9 + i15 + i21 + i27 + i33 - 5*b39 <= 0;

e29:    i4 + i10 + i16 + i22 + i28 + i34 - 5*b40 <= 0;

e30:    i5 + i11 + i17 + i23 + i29 + i35 - 5*b41 <= 0;

e31:    i6 + i12 + i18 + i24 + i30 + i36 - 5*b42 <= 0;

e32:    b37 - i43 <= 0;

e33:    b38 - i44 <= 0;

e34:    b39 - i45 <= 0;

e35:    b40 - i46 <= 0;

e36:    b41 - i47 <= 0;

e37:    b42 - i48 <= 0;

e38:  - 15*b37 + i43 <= 0;

e39:  - 12*b38 + i44 <= 0;

e40:  - 8*b39 + i45 <= 0;

e41:  - 7*b40 + i46 <= 0;

e42:  - 4*b41 + i47 <= 0;

e43:  - 2*b42 + i48 <= 0;

e44:    i43 + i44 + i45 + i46 + i47 + i48 >= 16;

e45:  - b37 + b38 <= 0;

e46:  - b38 + b39 <= 0;

e47:  - b39 + b40 <= 0;

e48:  - b40 + b41 <= 0;

e49:  - b41 + b42 <= 0;

e50:  - i43 + i44 <= 0;

e51:  - i44 + i45 <= 0;

e52:  - i45 + i46 <= 0;

e53:  - i46 + i47 <= 0;

e54:  - i47 + i48 <= 0;
