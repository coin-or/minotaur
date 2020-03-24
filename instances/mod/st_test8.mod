#  MINLP written by GAMS Convert at 01/12/18 13:43:36
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         21        1        0       20        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         25        1        0       24        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        121       97       24        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 100;
var i2 integer >= 0, <= 100;
var i3 integer >= 0, <= 100;
var i4 integer >= 0, <= 100;
var i5 integer >= 0, <= 100;
var i6 integer >= 0, <= 100;
var i7 integer >= 0, <= 100;
var i8 integer >= 0, <= 100;
var i9 integer >= 0, <= 100;
var i10 integer >= 0, <= 100;
var i11 integer >= 0, <= 100;
var i12 integer >= 0, <= 100;
var i13 integer >= 0, <= 100;
var i14 integer >= 0, <= 100;
var i15 integer >= 0, <= 100;
var i16 integer >= 0, <= 100;
var i17 integer >= 0, <= 100;
var i18 integer >= 0, <= 100;
var i19 integer >= 0, <= 100;
var i20 integer >= 0, <= 100;
var i21 integer >= 0, <= 100;
var i22 integer >= 0, <= 100;
var i23 integer >= 0, <= 100;
var i24 integer >= 0, <= 100;

minimize obj: 7*i1*i1 + 300*i1 + 4*i2*i2 + 270*i2 + 6*i3*i3 - 460*i3 + 8*i4*i4
     + 800*i4 + 12*i5*i5 + 740*i5 + 9*i6*i6 - 600*i6 + 14*i7*i7 + 540*i7 + 7*i8
    *i8 + 380*i8 + 13*i9*i9 + 300*i9 + 12*i10*i10 - 490*i10 + 8*i11*i11 + 380*
    i11 + 4*i12*i12 + 760*i12 + 7*i13*i13 - 430*i13 + 9*i14*i14 + 250*i14 + 16*
    i15*i15 + 390*i15 + 8*i16*i16 + 600*i16 + 4*i17*i17 - 210*i17 + 10*i18*i18
     + 830*i18 + 21*i19*i19 + 470*i19 + 13*i20*i20 - 680*i20 + 17*i21*i21 + 360
    *i21 + 9*i22*i22 + 290*i22 + 8*i23*i23 - 400*i23 + 4*i24*i24 + 310*i24;

subject to

e1:  - i1 - i5 - i9 - i13 - i17 - i21 <= -29;

e2:  - i2 - i6 - i10 - i14 - i18 - i22 <= -41;

e3:  - i3 - i7 - i11 - i15 - i19 - i23 <= -13;

e4:  - i4 - i8 - i12 - i16 - i20 - i24 <= -21;

e5:  - i1 - i2 - i3 - i4 <= -8;

e6:  - i5 - i6 - i7 - i8 <= -24;

e7:  - i9 - i10 - i11 - i12 <= -20;

e8:  - i13 - i14 - i15 - i16 <= -24;

e9:  - i17 - i18 - i19 - i20 <= -16;

e10:  - i21 - i22 - i23 - i24 <= -12;

e11:    i1 + i5 + i9 + i13 + i17 + i21 <= 29;

e12:    i2 + i6 + i10 + i14 + i18 + i22 <= 41;

e13:    i3 + i7 + i11 + i15 + i19 + i23 <= 13;

e14:    i4 + i8 + i12 + i16 + i20 + i24 <= 21;

e15:    i1 + i2 + i3 + i4 <= 8;

e16:    i5 + i6 + i7 + i8 <= 24;

e17:    i9 + i10 + i11 + i12 <= 20;

e18:    i13 + i14 + i15 + i16 <= 24;

e19:    i17 + i18 + i19 + i20 <= 16;

e20:    i21 + i22 + i23 + i24 <= 12;
