#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         32        1        0       31        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         61       46        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        121       91       30        0
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
var x32;
var x33;
var x34;
var x35;
var x36;
var x37;
var x38;
var x39;
var x40;
var x41;
var x42;
var x43;
var x44;
var x45;
var x46;
var x47;
var x48;
var x49;
var x50;
var x51;
var x52;
var x53;
var x54;
var x55;
var x56;
var x57;
var x58;
var x59;
var x60;
var x61;

minimize obj:  - 0.95*i1 - 0.92*i2 - 0.055*i3 - 0.74*i4 - 0.27*i5 - 0.42*i6
     - 0.55*i7 - 0.945*i8 - 0.42*i9 - 0.985*i10 - 0.3*i11 - 0.7*i12 - 0.665*i13
     - 0.54*i14 - 0.7*i15 - 0.665*x16 - 0.18*x17 - 0.13*x18 - x19 - 0.17*x20
     - 0.035*x21 - 0.56*x22 - 0.88*x23 - 0.67*x24 - 0.19*x25 - 0.37*x26
     - 0.46*x27 - 0.98*x28 - 0.155*x29 - 0.855*x30;

subject to

e1:    x32 + x33 + x34 + x35 + x36 + x37 + x38 + x39 + x40 + x41 + x42 + x43
     + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53 + x54 + x55
     + x56 + x57 + x58 + x59 + x60 + x61 <= 99.9999;

e3: i1^2 - x32 <= 0;

e4: i2^2 - x33 <= 0;

e5: i3^2 - x34 <= 0;

e6: i4^2 - x35 <= 0;

e7: i5^2 - x36 <= 0;

e8: i6^2 - x37 <= 0;

e9: i7^2 - x38 <= 0;

e10: i8^2 - x39 <= 0;

e11: i9^2 - x40 <= 0;

e12: i10^2 - x41 <= 0;

e13: i11^2 - x42 <= 0;

e14: i12^2 - x43 <= 0;

e15: i13^2 - x44 <= 0;

e16: i14^2 - x45 <= 0;

e17: i15^2 - x46 <= 0;

e18: x16^2 - x47 <= 0;

e19: x17^2 - x48 <= 0;

e20: x18^2 - x49 <= 0;

e21: x19^2 - x50 <= 0;

e22: x20^2 - x51 <= 0;

e23: x21^2 - x52 <= 0;

e24: x22^2 - x53 <= 0;

e25: x23^2 - x54 <= 0;

e26: x24^2 - x55 <= 0;

e27: x25^2 - x56 <= 0;

e28: x26^2 - x57 <= 0;

e29: x27^2 - x58 <= 0;

e30: x28^2 - x59 <= 0;

e31: x29^2 - x60 <= 0;

e32: x30^2 - x61 <= 0;
