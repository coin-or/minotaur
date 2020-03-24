#  MINLP written by GAMS Convert at 01/12/18 13:31:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         45        1        6       38        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         36        1        5       30        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        222      172       50        0
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
var b26 binary >= 0, <= 1;
var b27 binary >= 0, <= 1;
var b28 binary >= 0, <= 1;
var b29 binary >= 0, <= 1;
var b30 binary >= 0, <= 1;
var i31 integer >= 0, <= 15;
var i32 integer >= 0, <= 12;
var i33 integer >= 0, <= 9;
var i34 integer >= 0, <= 6;
var i35 integer >= 0, <= 6;

minimize obj:    0.1*b26 + 0.2*b27 + 0.3*b28 + 0.4*b29 + 0.5*b30 + i31 + i32
     + i33 + i34 + i35;

subject to

e2: i31*i1 + i32*i2 + i33*i3 + i34*i4 + i35*i5 >= 12;

e3: i31*i6 + i32*i7 + i33*i8 + i34*i9 + i35*i10 >= 6;

e4: i31*i11 + i32*i12 + i33*i13 + i34*i14 + i35*i15 >= 15;

e5: i31*i16 + i32*i17 + i33*i18 + i34*i19 + i35*i20 >= 6;

e6: i31*i21 + i32*i22 + i33*i23 + i34*i24 + i35*i25 >= 9;

e7:  - 330*i1 - 360*i6 - 370*i11 - 415*i16 - 435*i21 + 1800*b26 <= 0;

e8:  - 330*i2 - 360*i7 - 370*i12 - 415*i17 - 435*i22 + 1800*b27 <= 0;

e9:  - 330*i3 - 360*i8 - 370*i13 - 415*i18 - 435*i23 + 1800*b28 <= 0;

e10:  - 330*i4 - 360*i9 - 370*i14 - 415*i19 - 435*i24 + 1800*b29 <= 0;

e11:  - 330*i5 - 360*i10 - 370*i15 - 415*i20 - 435*i25 + 1800*b30 <= 0;

e12:    330*i1 + 360*i6 + 370*i11 + 415*i16 + 435*i21 - 2000*b26 <= 0;

e13:    330*i2 + 360*i7 + 370*i12 + 415*i17 + 435*i22 - 2000*b27 <= 0;

e14:    330*i3 + 360*i8 + 370*i13 + 415*i18 + 435*i23 - 2000*b28 <= 0;

e15:    330*i4 + 360*i9 + 370*i14 + 415*i19 + 435*i24 - 2000*b29 <= 0;

e16:    330*i5 + 360*i10 + 370*i15 + 415*i20 + 435*i25 - 2000*b30 <= 0;

e17:  - i1 - i6 - i11 - i16 - i21 + b26 <= 0;

e18:  - i2 - i7 - i12 - i17 - i22 + b27 <= 0;

e19:  - i3 - i8 - i13 - i18 - i23 + b28 <= 0;

e20:  - i4 - i9 - i14 - i19 - i24 + b29 <= 0;

e21:  - i5 - i10 - i15 - i20 - i25 + b30 <= 0;

e22:    i1 + i6 + i11 + i16 + i21 - 5*b26 <= 0;

e23:    i2 + i7 + i12 + i17 + i22 - 5*b27 <= 0;

e24:    i3 + i8 + i13 + i18 + i23 - 5*b28 <= 0;

e25:    i4 + i9 + i14 + i19 + i24 - 5*b29 <= 0;

e26:    i5 + i10 + i15 + i20 + i25 - 5*b30 <= 0;

e27:    b26 - i31 <= 0;

e28:    b27 - i32 <= 0;

e29:    b28 - i33 <= 0;

e30:    b29 - i34 <= 0;

e31:    b30 - i35 <= 0;

e32:  - 15*b26 + i31 <= 0;

e33:  - 12*b27 + i32 <= 0;

e34:  - 9*b28 + i33 <= 0;

e35:  - 6*b29 + i34 <= 0;

e36:  - 6*b30 + i35 <= 0;

e37:    i31 + i32 + i33 + i34 + i35 >= 10;

e38:  - b26 + b27 <= 0;

e39:  - b27 + b28 <= 0;

e40:  - b28 + b29 <= 0;

e41:  - b29 + b30 <= 0;

e42:  - i31 + i32 <= 0;

e43:  - i32 + i33 <= 0;

e44:  - i33 + i34 <= 0;

e45:  - i34 + i35 <= 0;
