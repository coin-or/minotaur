#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         31       16        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         61       31       30        0
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
var x16 >= 0, <= 5;
var x17 >= 0, <= 5;
var x18 >= 0, <= 5;
var x19 >= 0, <= 5;
var x20 >= 0, <= 5;
var x21 >= 0, <= 5;
var x22 >= 0, <= 5;
var x23 >= 0, <= 5;
var x24 >= 0, <= 5;
var x25 >= 0, <= 5;
var x26 >= 0, <= 5;
var x27 >= 0, <= 5;
var x28 >= 0, <= 5;
var x29 >= 0, <= 5;
var x30 >= 0, <= 5;

minimize obj:  - 0.95*i1 - 0.92*i2 - 0.055*i3 - 0.74*i4 - 0.27*i5 - 0.42*i6
     - 0.55*i7 - 0.945*i8 - 0.42*i9 - 0.985*i10 - 0.3*i11 - 0.7*i12 - 0.665*i13
     - 0.54*i14 - 0.7*i15 - 0.665*x16 - 0.18*x17 - 0.13*x18 - x19 - 0.17*x20
     - 0.035*x21 - 0.56*x22 - 0.88*x23 - 0.67*x24 - 0.19*x25 - 0.37*x26
     - 0.46*x27 - 0.98*x28 - 0.155*x29 - 0.855*x30;

subject to

e1: sqrt(0.0001 + i1^2 + i2^2 + i3^2 + i4^2 + i5^2 + i6^2 + i7^2 + i8^2 + i9^2
     + i10^2 + i11^2 + i12^2 + i13^2 + i14^2 + i15^2 + x16^2 + x17^2 + x18^2 + 
    x19^2 + x20^2 + x21^2 + x22^2 + x23^2 + x24^2 + x25^2 + x26^2 + x27^2 + x28
    ^2 + x29^2 + x30^2) <= 10;
