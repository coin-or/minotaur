#  MINLP written by GAMS Convert at 02/12/18 09:52:52
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         31        1        0       30        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         61       31       30        0
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
var i22 integer >= -100;
var i23 integer >= -100;
var i24 integer >= -100;
var i25 integer >= -100;
var i26 integer >= -100;
var i27 integer >= -100;
var i28 integer >= -100;
var i29 integer >= -100;
var i30 integer >= -100;
var i31 integer >= -100;

minimize obj:    29*i2 + 28*i3 + 27*i4 + 26*i5 + 25*i6 + 24*i7 + 23*i8 + 22*i9
     + 21*i10 + 20*i11 + 19*i12 + 18*i13 + 17*i14 + 16*i15 + 15*i16 + 14*i17
     + 13*i18 + 12*i19 + 11*i20 + 10*i21 + 9*i22 + 8*i23 + 7*i24 + 6*i25
     + 5*i26 + 4*i27 + 3*i28 + 2*i29 + i30 + 30*i31;

subject to

e2: 100*i30^2 - 98*i30 + 100*i29^2 - 98*i29 + 100*i28^2 - 98*i28 + 100*i27^2 - 
    98*i27 + 100*i26^2 - 98*i26 + 100*i25^2 - 98*i25 + 100*i24^2 - 98*i24 + 100
    *i23^2 - 98*i23 + 100*i22^2 - 98*i22 + 100*i21^2 - 98*i21 + 100*i20^2 - 98*
    i20 + 100*i19^2 - 98*i19 + 100*i18^2 - 98*i18 + 100*i17^2 - 98*i17 + 100*
    i16^2 - 98*i16 + 100*i15^2 - 98*i15 + 100*i14^2 - 98*i14 + 100*i13^2 - 98*
    i13 + 100*i12^2 - 98*i12 + 100*i11^2 - 98*i11 + 100*i10^2 - 98*i10 + 100*i9
    ^2 - 98*i9 + 100*i8^2 - 98*i8 + 100*i7^2 - 98*i7 + 100*i6^2 - 98*i6 + 100*
    i5^2 - 98*i5 + 100*i4^2 - 98*i4 + 100*i3^2 - 98*i3 + 100*i2^2 - 98*i2 + 100
    *i31^2 - 98*i31 - 2*i30*i29 - 2*i30*i29 - 2*i28*i27 - 2*i28*i27 - 2*i26*i25
     - 2*i26*i25 - 2*i24*i23 - 2*i24*i23 - 2*i22*i21 - 2*i22*i21 - 2*i20*i19 - 
    2*i20*i19 - 2*i18*i17 - 2*i18*i17 - 2*i16*i15 - 2*i16*i15 - 2*i14*i13 - 2*
    i14*i13 - 2*i12*i11 - 2*i12*i11 - 2*i10*i9 - 2*i10*i9 - 2*i8*i7 - 2*i8*i7
     - 2*i6*i5 - 2*i6*i5 - 2*i4*i3 - 2*i4*i3 - 2*i2*i31 - 2*i2*i31 <= -1;
