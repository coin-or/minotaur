#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         42        1        0       41        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         81       61        0       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        161      121       40        0
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
var x31 >= 0, <= 5;
var x32 >= 0, <= 5;
var x33 >= 0, <= 5;
var x34 >= 0, <= 5;
var x35 >= 0, <= 5;
var x36 >= 0, <= 5;
var x37 >= 0, <= 5;
var x38 >= 0, <= 5;
var x39 >= 0, <= 5;
var x40 >= 0, <= 5;
var x42 >= 0;
var x43 >= 0;
var x44 >= 0;
var x45 >= 0;
var x46 >= 0;
var x47 >= 0;
var x48 >= 0;
var x49 >= 0;
var x50 >= 0;
var x51 >= 0;
var x52 >= 0;
var x53 >= 0;
var x54 >= 0;
var x55 >= 0;
var x56 >= 0;
var x57 >= 0;
var x58 >= 0;
var x59 >= 0;
var x60 >= 0;
var x61 >= 0;
var x62 >= 0;
var x63 >= 0;
var x64 >= 0;
var x65 >= 0;
var x66 >= 0;
var x67 >= 0;
var x68 >= 0;
var x69 >= 0;
var x70 >= 0;
var x71 >= 0;
var x72 >= 0;
var x73 >= 0;
var x74 >= 0;
var x75 >= 0;
var x76 >= 0;
var x77 >= 0;
var x78 >= 0;
var x79 >= 0;
var x80 >= 0;
var x81 >= 0;

minimize obj:  - 0.64*i1 - 0.38*i2 - 0.19*i3 - 0.43*i4 - 0.48*i5 - 0.12*i6
     - 0.59*i7 - 0.23*i8 - 0.38*i9 - 0.85*i10 - 0.25*i11 - 0.29*i12 - 0.62*i13
     - 0.82*i14 - 0.27*i15 - 0.98*i16 - 0.73*i17 - 0.34*i18 - 0.58*i19
     - 0.11*i20 - 0.91*x21 - 0.88*x22 - 0.82*x23 - 0.26*x24 - 0.02*x25
     - 0.43*x26 - 0.31*x27 - 0.59*x28 - 0.16*x29 - 0.18*x30 - 0.42*x31
     - 0.09*x32 - 0.6*x33 - 0.47*x34 - 0.7*x35 - 0.7*x36 - 0.64*x37 - 0.03*x38
     - 0.07*x39 - 0.32*x40;

subject to

e1:    x42 + x43 + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53
     + x54 + x55 + x56 + x57 + x58 + x59 + x60 + x61 + x62 + x63 + x64 + x65
     + x66 + x67 + x68 + x69 + x70 + x71 + x72 + x73 + x74 + x75 + x76 + x77
     + x78 + x79 + x80 + x81 <= 99.9999;

e3: i1^2 - x42 <= 0;

e4: i2^2 - x43 <= 0;

e5: i3^2 - x44 <= 0;

e6: i4^2 - x45 <= 0;

e7: i5^2 - x46 <= 0;

e8: i6^2 - x47 <= 0;

e9: i7^2 - x48 <= 0;

e10: i8^2 - x49 <= 0;

e11: i9^2 - x50 <= 0;

e12: i10^2 - x51 <= 0;

e13: i11^2 - x52 <= 0;

e14: i12^2 - x53 <= 0;

e15: i13^2 - x54 <= 0;

e16: i14^2 - x55 <= 0;

e17: i15^2 - x56 <= 0;

e18: i16^2 - x57 <= 0;

e19: i17^2 - x58 <= 0;

e20: i18^2 - x59 <= 0;

e21: i19^2 - x60 <= 0;

e22: i20^2 - x61 <= 0;

e23: x21^2 - x62 <= 0;

e24: x22^2 - x63 <= 0;

e25: x23^2 - x64 <= 0;

e26: x24^2 - x65 <= 0;

e27: x25^2 - x66 <= 0;

e28: x26^2 - x67 <= 0;

e29: x27^2 - x68 <= 0;

e30: x28^2 - x69 <= 0;

e31: x29^2 - x70 <= 0;

e32: x30^2 - x71 <= 0;

e33: x31^2 - x72 <= 0;

e34: x32^2 - x73 <= 0;

e35: x33^2 - x74 <= 0;

e36: x34^2 - x75 <= 0;

e37: x35^2 - x76 <= 0;

e38: x36^2 - x77 <= 0;

e39: x37^2 - x78 <= 0;

e40: x38^2 - x79 <= 0;

e41: x39^2 - x80 <= 0;

e42: x40^2 - x81 <= 0;
