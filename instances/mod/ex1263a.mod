#  MINLP written by GAMS Convert at 01/12/18 13:31:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         36        1        5       30        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         25        1        4       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        153      121       32        0
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
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var i21 integer >= 0, <= 30;
var i22 integer >= 0, <= 30;
var i23 integer >= 0, <= 30;
var i24 integer >= 0, <= 30;

minimize obj:    0.1*b17 + 0.2*b18 + 0.3*b19 + 0.4*b20 + i21 + i22 + i23 + i24;

subject to

e2: i21*i1 + i22*i2 + i23*i3 + i24*i4 >= 15;

e3: i21*i5 + i22*i6 + i23*i7 + i24*i8 >= 28;

e4: i21*i9 + i22*i10 + i23*i11 + i24*i12 >= 21;

e5: i21*i13 + i22*i14 + i23*i15 + i24*i16 >= 30;

e6:  - 290*i1 - 315*i5 - 350*i9 - 455*i13 + 1750*b17 <= 0;

e7:  - 290*i2 - 315*i6 - 350*i10 - 455*i14 + 1750*b18 <= 0;

e8:  - 290*i3 - 315*i7 - 350*i11 - 455*i15 + 1750*b19 <= 0;

e9:  - 290*i4 - 315*i8 - 350*i12 - 455*i16 + 1750*b20 <= 0;

e10:    290*i1 + 315*i5 + 350*i9 + 455*i13 - 1850*b17 <= 0;

e11:    290*i2 + 315*i6 + 350*i10 + 455*i14 - 1850*b18 <= 0;

e12:    290*i3 + 315*i7 + 350*i11 + 455*i15 - 1850*b19 <= 0;

e13:    290*i4 + 315*i8 + 350*i12 + 455*i16 - 1850*b20 <= 0;

e14:  - i1 - i5 - i9 - i13 + b17 <= 0;

e15:  - i2 - i6 - i10 - i14 + b18 <= 0;

e16:  - i3 - i7 - i11 - i15 + b19 <= 0;

e17:  - i4 - i8 - i12 - i16 + b20 <= 0;

e18:    i1 + i5 + i9 + i13 - 5*b17 <= 0;

e19:    i2 + i6 + i10 + i14 - 5*b18 <= 0;

e20:    i3 + i7 + i11 + i15 - 5*b19 <= 0;

e21:    i4 + i8 + i12 + i16 - 5*b20 <= 0;

e22:    b17 - i21 <= 0;

e23:    b18 - i22 <= 0;

e24:    b19 - i23 <= 0;

e25:    b20 - i24 <= 0;

e26:  - 30*b17 + i21 <= 0;

e27:  - 30*b18 + i22 <= 0;

e28:  - 30*b19 + i23 <= 0;

e29:  - 30*b20 + i24 <= 0;

e30:    i21 + i22 + i23 + i24 >= 19;

e31:  - b17 + b18 <= 0;

e32:  - b18 + b19 <= 0;

e33:  - b19 + b20 <= 0;

e34:  - i21 + i22 <= 0;

e35:  - i22 + i23 <= 0;

e36:  - i23 + i24 <= 0;
