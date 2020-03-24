#  MINLP written by GAMS Convert at 01/12/18 13:45:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         55        1        3       51        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         49        1       12       36        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        228      174       54        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var i4 integer := 1, >= 0, <= 5;
var i5 integer >= 0, <= 5;
var i6 integer >= 0, <= 5;
var i7 integer >= 0, <= 5;
var i8 integer >= 0, <= 5;
var i9 integer >= 0, <= 5;
var i10 integer >= 0, <= 5;
var i11 integer >= 0, <= 5;
var i12 integer >= 0, <= 5;
var i13 integer := 1, >= 0, <= 5;
var i14 integer >= 0, <= 5;
var i15 integer >= 0, <= 5;
var i16 integer >= 0, <= 5;
var i17 integer >= 0, <= 5;
var i18 integer >= 0, <= 5;
var i19 integer >= 0, <= 5;
var i20 integer >= 0, <= 5;
var i21 integer >= 0, <= 5;
var i22 integer >= 0, <= 5;
var i23 integer := 1, >= 0, <= 5;
var i24 integer >= 0, <= 5;
var i25 integer >= 0, <= 5;
var i26 integer >= 0, <= 5;
var i27 integer >= 0, <= 5;
var i28 integer >= 0, <= 5;
var i29 integer >= 0, <= 5;
var i30 integer >= 0, <= 5;
var b31 binary >= 0, <= 1;
var b32 binary >= 0, <= 1;
var b33 binary >= 0, <= 1;
var b34 binary >= 0, <= 1;
var b35 binary >= 0, <= 1;
var b36 binary >= 0, <= 1;
var b37 binary >= 0, <= 1;
var b38 binary >= 0, <= 1;
var b39 binary >= 0, <= 1;
var i40 integer := 15, >= 0, <= 100;
var i41 integer := 80, >= 0, <= 100;
var i42 integer >= 0, <= 100;
var i43 integer >= 0, <= 100;
var i44 integer >= 0, <= 100;
var i45 integer >= 0, <= 100;
var i46 integer >= 0, <= 100;
var i47 integer >= 0, <= 100;
var i48 integer >= 0, <= 100;

minimize obj:    35*b2 + 35*b3 + 6.53333333333333*b31 + 6.53333333333333*b32
     + 6.7375*b33 + 6.53333333333333*b34 + 6.53333333333333*b35 + 6.7375*b36
     + 6.53333333333333*b37 + 6.53333333333333*b38 + 6.7375*b39;

subject to

e2: i40*i4 + i43*i7 + i46*i10 + i41*i5 + i44*i8 + i47*i11 + i42*i6 + i45*i9 + 
    i48*i12 >= 9;

e3: i40*i13 + i43*i16 + i46*i19 + i41*i14 + i44*i17 + i47*i20 + i42*i15 + i45*
    i18 + i48*i21 >= 15;

e4: i40*i22 + i43*i25 + i46*i28 + i41*i23 + i44*i26 + i47*i29 + i42*i24 + i45*
    i27 + i48*i30 >= 80;

e5:    12*i4 + 24*i13 + 36*i22 - 48*b31 <= 0;

e6:    12*i5 + 24*i14 + 36*i23 - 48*b32 <= 0;

e7:    12*i6 + 24*i15 + 36*i24 - 62*b33 <= 0;

e8:    12*i7 + 24*i16 + 36*i25 - 48*b34 <= 0;

e9:    12*i8 + 24*i17 + 36*i26 - 48*b35 <= 0;

e10:    12*i9 + 24*i18 + 36*i27 - 62*b36 <= 0;

e11:    12*i10 + 24*i19 + 36*i28 - 48*b37 <= 0;

e12:    12*i11 + 24*i20 + 36*i29 - 48*b38 <= 0;

e13:    12*i12 + 24*i21 + 36*i30 - 62*b39 <= 0;

e14:  - i4 - i13 - i22 + b31 <= 0;

e15:  - i5 - i14 - i23 + b32 <= 0;

e16:  - i6 - i15 - i24 + b33 <= 0;

e17:  - i7 - i16 - i25 + b34 <= 0;

e18:  - i8 - i17 - i26 + b35 <= 0;

e19:  - i9 - i18 - i27 + b36 <= 0;

e20:  - i10 - i19 - i28 + b37 <= 0;

e21:  - i11 - i20 - i29 + b38 <= 0;

e22:  - i12 - i21 - i30 + b39 <= 0;

e23:  - 72*b31 + i40 <= 0;

e24:  - 182*b32 + i41 <= 0;

e25:  - 182*b33 + i42 <= 0;

e26:  - 72*b34 + i43 <= 0;

e27:  - 182*b35 + i44 <= 0;

e28:  - 182*b36 + i45 <= 0;

e29:  - 72*b37 + i46 <= 0;

e30:  - 182*b38 + i47 <= 0;

e31:  - 182*b39 + i48 <= 0;

e32:    i4 + i13 + i22 - 5*b31 <= 0;

e33:    i5 + i14 + i23 - 5*b32 <= 0;

e34:    i6 + i15 + i24 - 5*b33 <= 0;

e35:    i7 + i16 + i25 - 5*b34 <= 0;

e36:    i8 + i17 + i26 - 5*b35 <= 0;

e37:    i9 + i18 + i27 - 5*b36 <= 0;

e38:    i10 + i19 + i28 - 5*b37 <= 0;

e39:    i11 + i20 + i29 - 5*b38 <= 0;

e40:    i12 + i21 + i30 - 5*b39 <= 0;

e41:  - 500*b1 + 7*i40 + 7*i43 + 7*i46 <= 0;

e42:  - 1270*b2 + 7*i41 + 7*i44 + 7*i47 <= 0;

e43:  - 1270*b3 + 7*i42 + 7*i45 + 7*i48 <= 0;

e44:  - b31 + b34 <= 0;

e45:  - b32 + b35 <= 0;

e46:  - b33 + b36 <= 0;

e47:  - b34 + b37 <= 0;

e48:  - b35 + b38 <= 0;

e49:  - b36 + b39 <= 0;

e50:  - i40 + i43 <= 0;

e51:  - i41 + i44 <= 0;

e52:  - i42 + i45 <= 0;

e53:  - i43 + i46 <= 0;

e54:  - i44 + i47 <= 0;

e55:  - i45 + i48 <= 0;
