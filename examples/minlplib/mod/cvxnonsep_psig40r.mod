#  MINLP written by GAMS Convert at 01/12/18 13:30:51
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         43        1        0       42        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         83       63        0       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        165      124       41        0
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
var x21 := 1, >= 1, <= 10;
var x22 := 1, >= 1, <= 10;
var x23 := 1, >= 1, <= 10;
var x24 := 1, >= 1, <= 10;
var x25 := 1, >= 1, <= 10;
var x26 := 1, >= 1, <= 10;
var x27 := 1, >= 1, <= 10;
var x28 := 1, >= 1, <= 10;
var x29 := 1, >= 1, <= 10;
var x30 := 1, >= 1, <= 10;
var x31 := 1, >= 1, <= 10;
var x32 := 1, >= 1, <= 10;
var x33 := 1, >= 1, <= 10;
var x34 := 1, >= 1, <= 10;
var x35 := 1, >= 1, <= 10;
var x36 := 1, >= 1, <= 10;
var x37 := 1, >= 1, <= 10;
var x38 := 1, >= 1, <= 10;
var x39 := 1, >= 1, <= 10;
var x40 := 1, >= 1, <= 10;
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
var x82;
var x83 := 1E-9, >= 1E-9;

minimize obj:    i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12
     + i13 + i14 + i15 + i16 + i17 + i18 + i19 + i20 + x21 + x22 + x23 + x24
     + x25 + x26 + x27 + x28 + x29 + x30 + x31 + x32 + x33 + x34 + x35 + x36
     + x37 + x38 + x39 + x40 + 40000*x83;

subject to

e2:    x42 + x43 + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53
     + x54 + x55 + x56 + x57 + x58 + x59 + x60 + x61 + x62 + x63 + x64 + x65
     + x66 + x67 + x68 + x69 + x70 + x71 + x72 + x73 + x74 + x75 + x76 + x77
     + x78 + x79 + x80 + x81 + x82 <= 0;

e3: -0.015*log(i1) - x42 <= 0;

e4: -0.37*log(i2) - x43 <= 0;

e5: -0.25*log(i3) - x44 <= 0;

e6: -0.24*log(i4) - x45 <= 0;

e7: -0.45*log(i5) - x46 <= 0;

e8: -0.305*log(i6) - x47 <= 0;

e9: -0.31*log(i7) - x48 <= 0;

e10: -0.43*log(i8) - x49 <= 0;

e11: -0.405*log(i9) - x50 <= 0;

e12: -0.29*log(i10) - x51 <= 0;

e13: -0.09*log(i11) - x52 <= 0;

e14: -0.12*log(i12) - x53 <= 0;

e15: -0.445*log(i13) - x54 <= 0;

e16: -0.015*log(i14) - x55 <= 0;

e17: -0.245*log(i15) - x56 <= 0;

e18: -0.085*log(i16) - x57 <= 0;

e19: -0.49*log(i17) - x58 <= 0;

e20: -0.355*log(i18) - x59 <= 0;

e21: -0.25*log(i19) - x60 <= 0;

e22: -0.235*log(i20) - x61 <= 0;

e23: -0.03*log(x21) - x62 <= 0;

e24: -0.34*log(x22) - x63 <= 0;

e25: -0.02*log(x23) - x64 <= 0;

e26: -0.035*log(x24) - x65 <= 0;

e27: -0.26*log(x25) - x66 <= 0;

e28: -0.05*log(x26) - x67 <= 0;

e29: -0.41*log(x27) - x68 <= 0;

e30: -0.41*log(x28) - x69 <= 0;

e31: -0.36*log(x29) - x70 <= 0;

e32: -0.075*log(x30) - x71 <= 0;

e33: -0.36*log(x31) - x72 <= 0;

e34: -0.33*log(x32) - x73 <= 0;

e35: -0.26*log(x33) - x74 <= 0;

e36: -0.485*log(x34) - x75 <= 0;

e37: -0.4*log(x35) - x76 <= 0;

e38: -0.225*log(x36) - x77 <= 0;

e39: -0.215*log(x37) - x78 <= 0;

e40: -0.415*log(x38) - x79 <= 0;

e41: -0.04*log(x39) - x80 <= 0;

e42: -0.065*log(x40) - x81 <= 0;

e43: -log(x83) - x82 <= 0;
