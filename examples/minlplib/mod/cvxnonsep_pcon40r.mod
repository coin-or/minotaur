#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         41        1        0       40        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         80       60        0       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        197      119       78        0
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
var x62;
var x63;
var x64;
var x65;
var x66;
var x67;
var x68;
var x69;
var x70;
var x71;
var x72;
var x73;
var x74;
var x75;
var x76;
var x77;
var x78;
var x79;
var x80;

minimize obj:  - 0.18*i1 - 0.72*i2 - 0.47*i3 - 0.15*i4 - 0.34*i5 - 0.61*i6
     - 0.19*i7 - 0.74*i8 - 0.24*i9 - 0.92*i10 - 0.27*i11 - 0.77*i12 - 0.19*i13
     - 0.29*i14 - 0.09*i15 - 0.58*i16 - 0.68*i17 - 0.55*i18 - 0.43*i19
     - 0.64*i20 - 0.65*x21 - 0.68*x22 - 0.64*x23 - 0.95*x24 - 0.21*x25
     - 0.71*x26 - 0.24*x27 - 0.12*x28 - 0.61*x29 - 0.45*x30 - 0.46*x31
     - 0.66*x32 - 0.77*x33 - 0.35*x34 - 0.66*x35 - 0.42*x36 - 0.84*x37
     - 0.83*x38 - 0.26*x39 - 0.61*x40;

subject to

e2:    x42 + x43 + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53
     + x54 + x55 + x56 + x57 + x58 + x59 + x60 + x61 + x62 + x63 + x64 + x65
     + x66 + x67 + x68 + x69 + x70 + x71 + x72 + x73 + x74 + x75 + x76 + x77
     + x78 + x79 + x80 <= 480;

e3: 2**(i1 + i2) - x42 <= 0;

e4: 2**(i2 + i3) - x43 <= 0;

e5: 2**(i3 + i4) - x44 <= 0;

e6: 2**(i4 + i5) - x45 <= 0;

e7: 2**(i5 + i6) - x46 <= 0;

e8: 2**(i6 + i7) - x47 <= 0;

e9: 2**(i7 + i8) - x48 <= 0;

e10: 2**(i8 + i9) - x49 <= 0;

e11: 2**(i9 + i10) - x50 <= 0;

e12: 2**(i10 + i11) - x51 <= 0;

e13: 2**(i11 + i12) - x52 <= 0;

e14: 2**(i12 + i13) - x53 <= 0;

e15: 2**(i13 + i14) - x54 <= 0;

e16: 2**(i14 + i15) - x55 <= 0;

e17: 2**(i15 + i16) - x56 <= 0;

e18: 2**(i16 + i17) - x57 <= 0;

e19: 2**(i17 + i18) - x58 <= 0;

e20: 2**(i18 + i19) - x59 <= 0;

e21: 2**(i19 + i20) - x60 <= 0;

e22: 2**(i20 + x21) - x61 <= 0;

e23: 2**(x21 + x22) - x62 <= 0;

e24: 2**(x22 + x23) - x63 <= 0;

e25: 2**(x23 + x24) - x64 <= 0;

e26: 2**(x24 + x25) - x65 <= 0;

e27: 2**(x25 + x26) - x66 <= 0;

e28: 2**(x26 + x27) - x67 <= 0;

e29: 2**(x27 + x28) - x68 <= 0;

e30: 2**(x28 + x29) - x69 <= 0;

e31: 2**(x29 + x30) - x70 <= 0;

e32: 2**(x30 + x31) - x71 <= 0;

e33: 2**(x31 + x32) - x72 <= 0;

e34: 2**(x32 + x33) - x73 <= 0;

e35: 2**(x33 + x34) - x74 <= 0;

e36: 2**(x34 + x35) - x75 <= 0;

e37: 2**(x35 + x36) - x76 <= 0;

e38: 2**(x36 + x37) - x77 <= 0;

e39: 2**(x37 + x38) - x78 <= 0;

e40: 2**(x38 + x39) - x79 <= 0;

e41: 2**(x39 + x40) - x80 <= 0;
