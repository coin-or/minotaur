#  MINLP written by GAMS Convert at 01/12/18 13:30:48
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         22        1        0       21        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         41       31        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         81       61       20        0
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
var x11 >= 0, <= 5;
var x12 >= 0, <= 5;
var x13 >= 0, <= 5;
var x14 >= 0, <= 5;
var x15 >= 0, <= 5;
var x16 >= 0, <= 5;
var x17 >= 0, <= 5;
var x18 >= 0, <= 5;
var x19 >= 0, <= 5;
var x20 >= 0, <= 5;
var x22;
var x23;
var x24;
var x25;
var x26;
var x27;
var x28;
var x29;
var x30;
var x31;
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

minimize obj:  - 0.175*i1 - 0.39*i2 - 0.83*i3 - 0.805*i4 - 0.06*i5 - 0.4*i6
     - 0.52*i7 - 0.415*i8 - 0.655*i9 - 0.63*i10 - 0.29*x11 - 0.43*x12
     - 0.015*x13 - 0.985*x14 - 0.165*x15 - 0.105*x16 - 0.37*x17 - 0.2*x18
     - 0.49*x19 - 0.34*x20;

subject to

e1:    x22 + x23 + x24 + x25 + x26 + x27 + x28 + x29 + x30 + x31 + x32 + x33
     + x34 + x35 + x36 + x37 + x38 + x39 + x40 + x41 <= 99.9999;

e3: i1^2 - x22 <= 0;

e4: i2^2 - x23 <= 0;

e5: i3^2 - x24 <= 0;

e6: i4^2 - x25 <= 0;

e7: i5^2 - x26 <= 0;

e8: i6^2 - x27 <= 0;

e9: i7^2 - x28 <= 0;

e10: i8^2 - x29 <= 0;

e11: i9^2 - x30 <= 0;

e12: i10^2 - x31 <= 0;

e13: x11^2 - x32 <= 0;

e14: x12^2 - x33 <= 0;

e15: x13^2 - x34 <= 0;

e16: x14^2 - x35 <= 0;

e17: x15^2 - x36 <= 0;

e18: x16^2 - x37 <= 0;

e19: x17^2 - x38 <= 0;

e20: x18^2 - x39 <= 0;

e21: x19^2 - x40 <= 0;

e22: x20^2 - x41 <= 0;
