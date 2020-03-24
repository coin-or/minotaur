#  MINLP written by GAMS Convert at 01/12/18 13:29:55
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


var i2 integer >= -1, <= 1;
var i3 integer >= -1, <= 1;
var i4 integer >= -1, <= 1;
var i5 integer >= -1, <= 1;
var i6 integer >= -1, <= 1;
var i7 integer >= -1, <= 1;
var i8 integer >= -1, <= 1;
var i9 integer >= -1, <= 1;
var i10 integer >= -1, <= 1;
var i11 integer >= -1, <= 1;
var i12 integer >= -1, <= 1;
var i13 integer >= -1, <= 1;
var i14 integer >= -1, <= 1;
var i15 integer >= -1, <= 1;
var i16 integer >= -1, <= 1;
var i17 integer >= -1, <= 1;
var i18 integer >= -1, <= 1;
var i19 integer >= -1, <= 1;
var i20 integer >= -1, <= 1;
var i21 integer >= -1, <= 1;
var i22 integer >= -1, <= 1;
var i23 integer >= -1, <= 1;
var i24 integer >= -1, <= 1;
var i25 integer >= -1, <= 1;
var i26 integer >= -1, <= 1;
var i27 integer >= -1, <= 1;
var i28 integer >= -1, <= 1;
var i29 integer >= -1, <= 1;
var i30 integer >= -1, <= 1;
var i31 integer >= -1, <= 1;

minimize obj:  - i2 - i3 - i4 - i5 - i6 - i7 - i8 - i9 - i10 - i11 - i12 - i13
     - i14 - i15 - i16 - i17 - i18 - i19 - i20 - i21 - i22 - i23 - i24 - i25
     - i26 - i27 - i28 - i29 - i30 - i31;

subject to

e2: i30^2 - 0.99582461641931*i30 + i29^2 - 0.99582461641931*i29 + i28^2 - 
    0.99582461641931*i28 + i27^2 - 0.99582461641931*i27 + i26^2 - 
    0.99582461641931*i26 + i25^2 - 0.99582461641931*i25 + i24^2 - 
    0.99582461641931*i24 + i23^2 - 0.99582461641931*i23 + i22^2 - 
    0.99582461641931*i22 + i21^2 - 0.99582461641931*i21 + i20^2 - 
    0.99582461641931*i20 + i19^2 - 0.99582461641931*i19 + i18^2 - 
    0.99582461641931*i18 + i17^2 - 0.99582461641931*i17 + i16^2 - 
    0.99582461641931*i16 + i15^2 - 0.99582461641931*i15 + i14^2 - 
    0.99582461641931*i14 + i13^2 - 0.99582461641931*i13 + i12^2 - 
    0.99582461641931*i12 + i11^2 - 0.99582461641931*i11 + i10^2 - 
    0.99582461641931*i10 + i9^2 - 0.99582461641931*i9 + i8^2 - 0.99582461641931
    *i8 + i7^2 - 0.99582461641931*i7 + i6^2 - 0.99582461641931*i6 + i5^2 - 
    0.99582461641931*i5 + i4^2 - 0.99582461641931*i4 + i3^2 - 0.99582461641931*
    i3 + i2^2 - 0.99582461641931*i2 + i31^2 - 0.99582461641931*i31 <= 0;
