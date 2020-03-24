#  MINLP written by GAMS Convert at 01/12/18 13:30:50
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


var i1 integer := 1, >= 1, <= 10;
var i2 integer := 1, >= 1, <= 10;
var i3 integer := 1, >= 1, <= 10;
var i4 integer := 1, >= 1, <= 10;
var i5 integer := 1, >= 1, <= 10;
var i6 integer := 1, >= 1, <= 10;
var i7 integer := 1, >= 1, <= 10;
var i8 integer := 1, >= 1, <= 10;
var i9 integer := 1, >= 1, <= 10;
var i10 integer := 1, >= 1, <= 10;
var i11 integer := 1, >= 1, <= 10;
var i12 integer := 1, >= 1, <= 10;
var i13 integer := 1, >= 1, <= 10;
var i14 integer := 1, >= 1, <= 10;
var i15 integer := 1, >= 1, <= 10;
var i16 integer := 1, >= 1, <= 10;
var i17 integer := 1, >= 1, <= 10;
var i18 integer := 1, >= 1, <= 10;
var i19 integer := 1, >= 1, <= 10;
var i20 integer := 1, >= 1, <= 10;
var x21 := 1E-5, >= 1E-5, <= 10;
var x22 := 1E-5, >= 1E-5, <= 10;
var x23 := 1E-5, >= 1E-5, <= 10;
var x24 := 1E-5, >= 1E-5, <= 10;
var x25 := 1E-5, >= 1E-5, <= 10;
var x26 := 1E-5, >= 1E-5, <= 10;
var x27 := 1E-5, >= 1E-5, <= 10;
var x28 := 1E-5, >= 1E-5, <= 10;
var x29 := 1E-5, >= 1E-5, <= 10;
var x30 := 1E-5, >= 1E-5, <= 10;
var x31 := 1E-5, >= 1E-5, <= 10;
var x32 := 1E-5, >= 1E-5, <= 10;
var x33 := 1E-5, >= 1E-5, <= 10;
var x34 := 1E-5, >= 1E-5, <= 10;
var x35 := 1E-5, >= 1E-5, <= 10;
var x36 := 1E-5, >= 1E-5, <= 10;
var x37 := 1E-5, >= 1E-5, <= 10;
var x38 := 1E-5, >= 1E-5, <= 10;
var x39 := 1E-5, >= 1E-5, <= 10;
var x40 := 1E-5, >= 1E-5, <= 10;
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
var x81;

minimize obj:    1.1*i1 + 1.86*i2 + 0.86*i3 + 1.23*i4 + 0.72*i5 + 0.47*i6
     + 0.98*i7 + 0.39*i8 + 0.25*i9 + 0.41*i10 + 0.29*i11 + 0.38*i12 + 1.49*i13
     + 1.27*i14 + 0.56*i15 + 1.08*i16 + 1.39*i17 + i18 + 1.37*i19 + 0.89*i20
     + 0.25*x21 + 0.98*x22 + 1.71*x23 + 1.75*x24 + 0.54*x25 + 0.42*x26
     + 1.13*x27 + 1.28*x28 + 0.83*x29 + 0.41*x30 + 1.9*x31 + 0.16*x32
     + 0.21*x33 + 0.28*x34 + 0.33*x35 + 1.24*x36 + 1.15*x37 + 0.1*x38
     + 1.86*x39 + 1.46*x40;

subject to

e2:    x42 + x43 + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53
     + x54 + x55 + x56 + x57 + x58 + x59 + x60 + x61 + x62 + x63 + x64 + x65
     + x66 + x67 + x68 + x69 + x70 + x71 + x72 + x73 + x74 + x75 + x76 + x77
     + x78 + x79 + x80 + x81 <= -1.6094379124341;

e3: -0.035*log(i1) - x42 <= 0;

e4: -0.003*log(i2) - x43 <= 0;

e5: -0.04*log(i3) - x44 <= 0;

e6: -0.044*log(i4) - x45 <= 0;

e7: -0.046*log(i5) - x46 <= 0;

e8: -0.04*log(i6) - x47 <= 0;

e9: -0.037*log(i7) - x48 <= 0;

e10: -0.024*log(i8) - x49 <= 0;

e11: -0.008*log(i9) - x50 <= 0;

e12: -0.019*log(i10) - x51 <= 0;

e13: -0.006*log(i11) - x52 <= 0;

e14: -0.001*log(i12) - x53 <= 0;

e15: -0.044*log(i13) - x54 <= 0;

e16: -0.014*log(i14) - x55 <= 0;

e17: -0.014*log(i15) - x56 <= 0;

e18: -0.016*log(i16) - x57 <= 0;

e19: -0.022*log(i17) - x58 <= 0;

e20: -0.03*log(i18) - x59 <= 0;

e21: -0.001*log(i19) - x60 <= 0;

e22: -0.039*log(i20) - x61 <= 0;

e23: -0.026*log(x21) - x62 <= 0;

e24: -0.04*log(x22) - x63 <= 0;

e25: -0.016*log(x23) - x64 <= 0;

e26: -log(x24**0.021) - x65 <= 0;

e27: -log(x25**0.003) - x66 <= 0;

e28: -0.008*log(x26) - x67 <= 0;

e29: -0.031*log(x27) - x68 <= 0;

e30: -0.015*log(x28) - x69 <= 0;

e31: -0.042*log(x29) - x70 <= 0;

e32: -0.006*log(x30) - x71 <= 0;

e33: -0.046*log(x31) - x72 <= 0;

e34: -0.025*log(x32) - x73 <= 0;

e35: -0.033*log(x33) - x74 <= 0;

e36: -0.047*log(x34) - x75 <= 0;

e37: -0.013*log(x35) - x76 <= 0;

e38: -0.019*log(x36) - x77 <= 0;

e39: -0.022*log(x37) - x78 <= 0;

e40: -0.036*log(x38) - x79 <= 0;

e41: -0.038*log(x39) - x80 <= 0;

e42: -0.005*log(x40) - x81 <= 0;
