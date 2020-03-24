#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         33        1        0       32        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         63       48        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        125       94       31        0
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
var x16 := 1, >= 1, <= 10;
var x17 := 1, >= 1, <= 10;
var x18 := 1, >= 1, <= 10;
var x19 := 1, >= 1, <= 10;
var x20 := 1, >= 1, <= 10;
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
var x32 <= 0;
var x33 <= 0;
var x34 <= 0;
var x35 <= 0;
var x36 <= 0;
var x37 <= 0;
var x38 <= 0;
var x39 <= 0;
var x40 <= 0;
var x41 <= 0;
var x42 <= 0;
var x43 <= 0;
var x44 <= 0;
var x45 <= 0;
var x46 <= 0;
var x47 <= 0;
var x48 <= 0;
var x49 <= 0;
var x50 <= 0;
var x51 <= 0;
var x52 <= 0;
var x53 <= 0;
var x54 <= 0;
var x55 <= 0;
var x56 <= 0;
var x57 <= 0;
var x58 <= 0;
var x59 <= 0;
var x60 <= 0;
var x61 <= 0;
var x62;
var x63 := 1E-10, >= 1E-10;

minimize obj:    i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12
     + i13 + i14 + i15 + x16 + x17 + x18 + x19 + x20 + x21 + x22 + x23 + x24
     + x25 + x26 + x27 + x28 + x29 + x30 + 30000*x63;

subject to

e2:    x32 + x33 + x34 + x35 + x36 + x37 + x38 + x39 + x40 + x41 + x42 + x43
     + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53 + x54 + x55
     + x56 + x57 + x58 + x59 + x60 + x61 + x62 <= 0;

e3: -0.48*log(i1) - x32 <= 0;

e4: -0.275*log(i2) - x33 <= 0;

e5: -0.26*log(i3) - x34 <= 0;

e6: -0.215*log(i4) - x35 <= 0;

e7: -0.245*log(i5) - x36 <= 0;

e8: -0.31*log(i6) - x37 <= 0;

e9: -0.34*log(i7) - x38 <= 0;

e10: -0.2*log(i8) - x39 <= 0;

e11: -0.185*log(i9) - x40 <= 0;

e12: -0.495*log(i10) - x41 <= 0;

e13: -0.02*log(i11) - x42 <= 0;

e14: -0.445*log(i12) - x43 <= 0;

e15: -0.455*log(i13) - x44 <= 0;

e16: -0.4*log(i14) - x45 <= 0;

e17: -0.05*log(i15) - x46 <= 0;

e18: -0.13*log(x16) - x47 <= 0;

e19: -0.17*log(x17) - x48 <= 0;

e20: -0.34*log(x18) - x49 <= 0;

e21: -0.07*log(x19) - x50 <= 0;

e22: -0.36*log(x20) - x51 <= 0;

e23: -0.05*log(x21) - x52 <= 0;

e24: -0.325*log(x22) - x53 <= 0;

e25: -0.245*log(x23) - x54 <= 0;

e26: -0.39*log(x24) - x55 <= 0;

e27: -0.36*log(x25) - x56 <= 0;

e28: -0.45*log(x26) - x57 <= 0;

e29: -0.445*log(x27) - x58 <= 0;

e30: -0.165*log(x28) - x59 <= 0;

e31: -0.35*log(x29) - x60 <= 0;

e32: -0.1*log(x30) - x61 <= 0;

e33: -log(x63) - x62 <= 0;
