#  MINLP written by GAMS Convert at 02/12/18 09:52:52
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         21        1        0       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         41       21       20        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i2 integer >= -100;
var i3 integer >= -100;
var i4 integer >= -100;
var i5 integer >= -100;
var i6 integer >= -100;
var i7 integer >= -100;
var i8 integer >= -100;
var i9 integer >= -100;
var i10 integer >= -100;
var i11 integer >= -100;
var i12 integer >= -100;
var i13 integer >= -100;
var i14 integer >= -100;
var i15 integer >= -100;
var i16 integer >= -100;
var i17 integer >= -100;
var i18 integer >= -100;
var i19 integer >= -100;
var i20 integer >= -100;
var i21 integer >= -100;

minimize obj:    19*i2 + 18*i3 + 17*i4 + 16*i5 + 15*i6 + 14*i7 + 13*i8 + 12*i9
     + 11*i10 + 10*i11 + 9*i12 + 8*i13 + 7*i14 + 6*i15 + 5*i16 + 4*i17 + 3*i18
     + 2*i19 + i20 + 20*i21;

subject to

e2: 100*i20^2 - 98*i20 + 100*i19^2 - 98*i19 + 100*i18^2 - 98*i18 + 100*i17^2 - 
    98*i17 + 100*i16^2 - 98*i16 + 100*i15^2 - 98*i15 + 100*i14^2 - 98*i14 + 100
    *i13^2 - 98*i13 + 100*i12^2 - 98*i12 + 100*i11^2 - 98*i11 + 100*i10^2 - 98*
    i10 + 100*i9^2 - 98*i9 + 100*i8^2 - 98*i8 + 100*i7^2 - 98*i7 + 100*i6^2 - 
    98*i6 + 100*i5^2 - 98*i5 + 100*i4^2 - 98*i4 + 100*i3^2 - 98*i3 + 100*i2^2
     - 98*i2 + 100*i21^2 - 98*i21 - 2*i20*i19 - 2*i20*i19 - 2*i18*i17 - 2*i18*
    i17 - 2*i16*i15 - 2*i16*i15 - 2*i14*i13 - 2*i14*i13 - 2*i12*i11 - 2*i12*i11
     - 2*i10*i9 - 2*i10*i9 - 2*i8*i7 - 2*i8*i7 - 2*i6*i5 - 2*i6*i5 - 2*i4*i3 - 
    2*i4*i3 - 2*i2*i21 - 2*i2*i21 <= -1;
